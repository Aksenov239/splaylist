#include <algorithm>
#include <iostream>
#include <atomic>
#include <memory>
#include <sstream>
#include <mutex>
#include <shared_mutex>

#include "DCLCRWLock.h"
//#include "CLHRWLock.h"

using namespace std;

template <typename K, typename V>
struct CBTreeNode {
    K key;
    V value;
    CBTreeNode* left;
    CBTreeNode* right;
    CBTreeNode* parent;
    long long w;
    CBTreeNode(K key, V value): key(key), value(value){
        left = right = parent = nullptr;
        w = 0;
    }
    inline long long c() {
        return w - (left == nullptr ? 0 : left->w) - (right == nullptr ? 0 : right->w);
    }
};

const int PADDING_SIZE = 128;
struct MyLength {

    long long value;
    volatile char pad[PADDING_SIZE];
    MyLength() {
        value = 0;
    }
};

template<typename skey_t, typename sval_t, typename Comp = std::less<skey_t>, int rotation_eps=1>
class CBTree {

    public:
    CBTree(skey_t max_key, sval_t no_value): max_key(max_key), no_value(no_value) {
        root = new NodeType(max_key, no_value);
        eps = 1.0 / (1 << rotation_eps);
        one_plus_eps = 1 + eps;

        //pthread_rwlock_init(&lock, NULL);
        //clh_rwlock_init(&lock);
    }
    sval_t insertIfAbsent(const int tid, skey_t key, sval_t val) {
        NodeTypePtr parent_node;
        //pthread_rwlock_wrlock(&lock);
        std::unique_lock<std::shared_timed_mutex> lock(mutex_);
        NodeTypePtr node = search(tid, key, &parent_node);
        if (node != nullptr)
        {
            //pthread_rwlock_unlock(&lock);
            return node->value;
        }
        else {
            node = new NodeType(key, val);
            if (less(key, parent_node))
                parent_node->left = node;
            else
                parent_node->right = node;
            node->w++;
            //pthread_rwlock_unlock(&lock);
            return this->no_value;
        }
    }
    sval_t find(const int tid, skey_t x) {
        NodeTypePtr node;
        if (warmup) {
            if (tid == 1)
            {
                lock.exclusiveLock();
                node = search(tid, x, nullptr);
                lock.exclusiveUnlock();
            }
            else
            {
                lock.sharedLock();
                node = search_no_restructure(tid, x);
                lock.sharedUnlock();
            }
        } else {
            node = search_no_restructure(tid, x);
        }
        if (node != nullptr)
            return node->value;
        return this->no_value;
    }
    long long getLength(const int tid) {
        return length[tid].value;
    }
    void warmupEnd() {
        warmup = false;
    }
    void stats() {
        stats(root);
        cout << " -------- " << endl;
    }
    private:
    MyLength length[100];
    std::shared_timed_mutex mutex_;
    DCLCRWLock lock;
    bool warmup = true;
    typedef CBTreeNode<skey_t, sval_t> NodeType;
    typedef NodeType* NodeTypePtr;
    const skey_t max_key;
    const sval_t no_value;
    NodeTypePtr root;
    double eps;
    double one_plus_eps;
    static bool greater(const skey_t& key, const NodeTypePtr node) {
        return node && Comp()(node->key, key);
    }

    static bool less(const skey_t& key, const NodeTypePtr node) {
        return (node == nullptr) || Comp()(key, node->key);
    }
    static bool equal(const skey_t& key, const NodeTypePtr node) {
        return !Comp()(key, node->key) && !Comp()(node->key, key);
    }
    inline NodeTypePtr get_child(NodeTypePtr node, const skey_t &key, bool &move_left) {
        if (less(key, node))
        {
            move_left = true;
            return node->left;
        }
        else if (greater(key, node))
        {
            move_left = false;
            return node->right;
        }
        else
            return nullptr;
    }
    NodeTypePtr search(const int tid, skey_t key, NodeTypePtr* parent_storage) {
        NodeTypePtr z = root;
        bool first_move_left = false;
        NodeTypePtr y = get_child(z, key, first_move_left);
        NodeTypePtr z_par = nullptr;
        while (z != nullptr) {
            length[tid].value++;
            if (y == nullptr)
            {
                z->w++;
                if (equal(key, z))
                    return z;
                if (parent_storage != nullptr)
                    (*parent_storage) = z;
                return y;
            }
            bool second_move_left = false;
            NodeTypePtr x = get_child(y, key, second_move_left);
            if (x == nullptr) {
                z->w++;
                y->w++;
                if (equal(key, y))
                    return y;
                if (parent_storage != nullptr)
                    (*parent_storage) = y;
                return x;
            }
            if (first_move_left == second_move_left) {
                // Single Rotation
                if (!check_single_rotation(y, z, first_move_left))
                {
                    z->w++;
                    y->w++;
                    z_par = y;
                    z = x;
                    y = get_child(z, key, first_move_left);
                    continue;
                }
                if (first_move_left)
                    rotate_right(y, z, z_par);
                else
                    rotate_left(y, z, z_par);
                z = y;
                y = x;
                first_move_left = second_move_left;
            }
            else {
                if (check_double_rotation(x, y, z, first_move_left))
                {
                    if (first_move_left)
                    {
                        rotate_left(x, y, z);
                        rotate_right(x, z, z_par);
                    }
                    else
                    {
                        rotate_right(x, y, z);
                        rotate_left(x, z, z_par);
                    }
                }
                else
                {
                    z->w++;
                    y->w++;
                    z_par = y;
                }
                z = x;
                y = get_child(z, key, first_move_left);
            }

        }
        assert(false);

    }
    NodeTypePtr search_no_restructure(const int tid, skey_t key) {
        NodeTypePtr z = root;
        bool first_move_left;
        NodeTypePtr y = get_child(z, key, first_move_left);
        while (z != nullptr) {
            length[tid].value++;
            if (y == nullptr)
            {
                if (equal(key, z))
                    return z;
                return y;
            }
            bool second_move_left = false;
            NodeTypePtr x = get_child(y, key, second_move_left);
            if (x == nullptr) {
                if (equal(key, y))
                    return y;
                return x;
            }
            z = x;
            y = get_child(z, key, first_move_left);
        }
        assert(false);
    }
    inline bool check_single_rotation(NodeTypePtr y, NodeTypePtr z, bool first_move_left) {
        if (z == root)
            return false;
        if (first_move_left)
            return (z->w + (y->right == nullptr ? 0: y->right->w)) < y->w * one_plus_eps;
        else
            return (z->w + (y->left == nullptr ? 0: y->left->w)) < y->w * one_plus_eps;
    }
    inline bool check_double_rotation(NodeTypePtr x, NodeTypePtr y, NodeTypePtr z, bool first_move_left) {
        if (z == root)
            return false;
        if (first_move_left)
            return ((z->w - y->w + (x->right == nullptr ? 0: x->right->w)) *
                (y->w - x->w + (x->left == nullptr ? 0: x->left->w))) < (y->w * x->w) * eps;
        else
            return ((z->w - y->w + (x->left == nullptr ? 0: x->left->w)) *
                (y->w - x->w + (x->right == nullptr ? 0: x->right->w))) < (y->w * x->w) * eps;
    }
    inline void rotate_right(NodeTypePtr x, NodeTypePtr y, NodeTypePtr z) {
        int y_new_w = y->w - x->w + (x->right == nullptr ? 0 : x->right->w);
        x->w = y->w;
        y->w = y_new_w;
        y->left = x->right;
        x->right = y;
        if (z->left == y)
            z->left = x;
        else
            z->right = x;
    }

    inline void rotate_left(NodeTypePtr x, NodeTypePtr y, NodeTypePtr z) {
        int y_new_w = y->w - x->w + (x->left == nullptr ? 0 : x->left->w);
        x->w = y->w;
        y->w = y_new_w;
        y->right = x->left;
        x->left = y;
        if (z->left == y)
            z->left = x;
        else
            z->right = x;
    }
    void stats(NodeTypePtr root) {
        if (!root)
            return;
        cout << root->key << " w(" << root->w << ") c(" << root->c() << ") ";
        if (root->left)
        {
            cout << "SR(" << check_single_rotation(root->left, root, true) << ") ";
            if (root->left->right)
                cout << "DR(" << check_double_rotation(root->left->right, root->left, root, true) << ") ";
        }
        if (root->right)
            cout << "SL(" << check_single_rotation(root->right, root, false) << ") ";
        cout << (root->left ? root->left->key : -1) << " " << (root->right ? root->right->key : -1) << endl;
        stats(root->left);
        stats(root->right);
    }

};
