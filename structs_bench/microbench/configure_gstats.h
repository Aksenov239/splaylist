/*
 * File:   gstats_output_defs.h
 * Author: t35brown
 *
 * Created on August 5, 2019, 5:25 PM
 */

#ifndef GSTATS_OUTPUT_DEFS_H
#define GSTATS_OUTPUT_DEFS_H

#define __AND ,
#define GSTATS_HANDLE_STATS(gstats_handle_stat) \
    gstats_handle_stat(LONG_LONG, node_allocated_addresses, 100, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, descriptor_allocated_addresses, 100, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, extra_type1_allocated_addresses, 100, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, extra_type2_allocated_addresses, 100, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, extra_type3_allocated_addresses, 100, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, extra_type4_allocated_addresses, 100, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, num_inserts, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_deletes, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_searches, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_rq, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_operations, 1, { \
            gstats_output_item(PRINT_RAW, SUM, BY_THREAD) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, SUM, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, limbo_reclamation_event_size, 10000, { \
            gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, pool_cpu_get, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, pool_cpu_add, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, move_block_reclaimer_to_cpu, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, move_block_cpu_to_node, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, move_block_node_to_global, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, move_block_global_to_alloc, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, move_block_alloc_to_cpu, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, move_block_global_to_cpu, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, move_block_node_to_cpu, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_bail_from_addkv_at_depth, 10, { \
            gstats_output_item(PRINT_RAW, SUM, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, num_bail_from_build_at_depth, 10, { \
            gstats_output_item(PRINT_RAW, SUM, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, num_help_subtree, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_try_rebuild_at_depth, 100, { \
            gstats_output_item(PRINT_RAW, SUM, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, num_complete_rebuild_at_depth, 100, { \
            gstats_output_item(PRINT_RAW, SUM, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, num_help_rebuild, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
/*    gstats_handle_stat(LONG_LONG, num_multi_counter_node_created, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_multi_counter_node_deallocated, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_multi_counter_node_retired, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_multi_counter_array_created, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_multi_counter_array_reclaimed, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    })*/ \
    /*gstats_handle_stat(LONG_LONG, num_isearch, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    })*/ \
    /*gstats_handle_stat(DOUBLE, visited_in_isearch, 100000, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    })*/ \
/*    gstats_handle_stat(LONG_LONG, num_getguard, 1, { \
            gstats_output_item(PRINT_RAW, SUM, BY_THREAD) \
      __AND gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_tryadvance, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_root, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_parentlarge, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_nodes_visited, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_hist_readepoch, 1, { \
            gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_root_update_time, 1, { \
            gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_root_read_time, 1, { \
            gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, timersplit_guard, 1, {}) */ \
    gstats_handle_stat(LONG_LONG, timersplit_epoch, 1, {}) \
    gstats_handle_stat(LONG_LONG, num_prop_epoch_latency, 10000, { \
            gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, bag_rotation_reclaim_size, 100, { \
            gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      /*__AND gstats_output_item(PRINT_RAW, NONE, FULL_DATA)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, timersplit_token_received, 1, {}) \
    gstats_handle_stat(LONG_LONG, timer_bag_rotation_start, 1, {}) \
    gstats_handle_stat(LONG_LONG, token_received_time_split_ms, 10000, { \
            gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      /*__AND gstats_output_item(PRINT_RAW, NONE, FULL_DATA)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, token_received_time_last_ms, 1, { \
           gstats_output_item(PRINT_RAW, FIRST, BY_THREAD) \
    }) \
    gstats_handle_stat(LONG_LONG, bag_rotation_start_time_us, 100, { \
            /*gstats_output_item(PRINT_RAW, NONE, FULL_DATA)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, bag_rotation_end_time_us, 100, { \
            /*gstats_output_item(PRINT_RAW, NONE, FULL_DATA)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, bag_rotation_duration_split_ms, 10000, { \
            gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
    }) \
    gstats_handle_stat(LONG_LONG, bag_curr_size, 1, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_THREAD) \
    }) \
    gstats_handle_stat(LONG_LONG, bag_last_size, 1, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_THREAD) \
    }) \
    gstats_handle_stat(LONG_LONG, token_counts, 1, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_THREAD) \
    }) \
/*  gstats_handle_stat(LONG_LONG, num_prop_guard_split, 100000, { \
            gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_guard_insdel_attempts, 100000, { \
            gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_guard_rebalance_attempts, 100000, { \
            gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_epoch_allocations, 500000, { \
            gstats_output_item(PRINT_RAW, SUM, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, num_prop_thread_exit_time, 1, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_THREAD) \
    }) */ \
    gstats_handle_stat(LONG_LONG, thread_announced_epoch, 1, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, thread_reclamation_start, 10, { \
           /* gstats_output_item(PRINT_RAW, NONE, FULL_DATA)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, thread_reclamation_end, 10, { \
           /* gstats_output_item(PRINT_RAW, NONE, FULL_DATA)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, visited_in_bags, 1, { \
            /*gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, SUM, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, skipped_in_bags, 1, { \
            /*gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, SUM, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, latency_rqs, 1, { \
            /*gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, SUM, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, latency_updates, 1, { \
            /*gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, SUM, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, elapsed_rebuild_depth, 10, { \
            gstats_output_item(PRINT_RAW, SUM, BY_INDEX) \
      __AND gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, duration_all_ops, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, duration_markAndCount, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, duration_wastedWorkBuilding, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, duration_buildAndReplace, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, duration_rotateAndFree, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, duration_traverseAndRetire, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, latency_searches, 1, { \
            /*gstats_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
      __AND gstats_output_item(PRINT_RAW, SUM, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, AVERAGE, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, STDEV, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL)*/ \
    }) \
    gstats_handle_stat(LONG_LONG, rebuild_is_subsumed, 1, { \
            gstats_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, rebuild_is_subsumed_at_depth, 100, { \
            gstats_output_item(PRINT_RAW, SUM, BY_INDEX) \
    }) \
    gstats_handle_stat(LONG_LONG, size_checksum, 1, {}) \
    gstats_handle_stat(LONG_LONG, key_checksum, 1, {}) \
    gstats_handle_stat(LONG_LONG, prefill_size, 1, {}) \
    gstats_handle_stat(LONG_LONG, time_thread_terminate, 1, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_THREAD) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, time_thread_start, 1, { \
            gstats_output_item(PRINT_RAW, FIRST, BY_THREAD) \
      __AND gstats_output_item(PRINT_RAW, MIN, TOTAL) \
      __AND gstats_output_item(PRINT_RAW, MAX, TOTAL) \
    }) \
    gstats_handle_stat(LONG_LONG, timer_duration, 1, {}) \
    gstats_handle_stat(LONG_LONG, timer_latency, 1, {}) \
    /*gstats_handle_stat(LONG_LONG, timer_rebuild, 1, {})*/

#define USE_GSTATS
#include "gstats_global.h"
GSTATS_DECLARE_STATS_OBJECT(MAX_THREADS_POW2);
GSTATS_DECLARE_ALL_STAT_IDS;

#define TIMING_START(s) \
    std::cout<<"timing_start "<<s<<"..."<<std::endl; \
    GSTATS_TIMER_RESET(tid, timer_duration);
#define TIMING_STOP \
    std::cout<<"timing_elapsed "<<(GSTATS_TIMER_SPLIT(tid, timer_duration)/1000000000.)<<"s"<<std::endl;

#endif /* GSTATS_OUTPUT_DEFS_H */
