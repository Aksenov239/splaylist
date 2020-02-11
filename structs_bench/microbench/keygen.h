/* 
 * File:   keygen.h
 * Author: t35brown
 *
 * Created on August 5, 2019, 7:24 PM
 */

#ifndef KEYGEN_H
#define KEYGEN_H

#include <cassert>
#include "plaf.h"

template <typename K>
class KeyGeneratorUniform {
private:
    PAD;
    RandomFNV1A * rng;
    int maxKey;
    PAD;
public:
    KeyGeneratorUniform(RandomFNV1A * _rng, int _maxKey) : rng(_rng), maxKey(_maxKey) {}
    K next() {
        auto result = 1+rng->next(maxKey);
        assert((result >= 1) && (result <= maxKey));
        return result;
    }
};

class KeyGeneratorZipfData {
public:
    PAD;
    int maxKey;
    double theta;
    double c = 0; // Normalization constant
    double * sum_probs; // Pre-calculated sum of probabilities
    PAD;
    
    KeyGeneratorZipfData(int _maxKey, double _alpha) {
//        std::cout<<"start KeyGeneratorZipfData"<<std::endl;
        maxKey = _maxKey;
        
        // Compute normalization constant c for implied key range: [1, maxKey]
        int i;
        for (i = 1; i <= _maxKey; i++)
            c = c + (1.0 / pow((double) i, theta));
        c = 1.0 / c;

        sum_probs = new double[_maxKey+1];
        sum_probs[0] = 0;
        for (i = 1; i <= _maxKey; i++) {
            sum_probs[i] = sum_probs[i - 1] + c / pow((double) i, theta);
        }
//        std::cout<<"end KeyGeneratorZipfData"<<std::endl;
    }
    ~KeyGeneratorZipfData() {
        delete[] sum_probs;
    }
};

template <typename K>
class KeyGeneratorZipf {
private:
    PAD;
    KeyGeneratorZipfData * data;
    RandomFNV1A * rng;
    PAD;
public:
    KeyGeneratorZipf(KeyGeneratorZipfData * _data, RandomFNV1A * _rng)
          : data(_data), rng(_rng) {}
    K next() {
        double z; // Uniform random number (0 < z < 1)
        int zipf_value; // Computed exponential value to be returned
        int low, high, mid; // Binary-search bounds

        // Pull a uniform random number (0 < z < 1)
        do {
            z = (rng->next() / (double) std::numeric_limits<uint64_t>::max());
//            printf("    z=%lf\n", z);
        } while ((z == 0) || (z == 1));

        // Map z to the value
        low = 1, high = data->maxKey, mid;
        do {
            mid = floor((low + high) / 2);
            if (data->sum_probs[mid] >= z && data->sum_probs[mid - 1] < z) {
                zipf_value = mid;
                break;
            } else if (data->sum_probs[mid] >= z) {
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        } while (low <= high);

        // Assert that zipf_value is between 1 and N
        assert((zipf_value >= 1) && (zipf_value <= maxKey));

        return (zipf_value);
    }
};

#endif /* KEYGEN_H */

