FLAGS =
LDFLAGS =
GPP = g++

### if you do not want PAPI to be used, invoke make with "has_libpapi=0"
has_libpapi=$(shell ldconfig -p | grep libpapi | wc -l)
ifneq ($(has_libpapi), 0)
    FLAGS += -DUSE_PAPI
    LDFLAGS += -lpapi
endif

### if you do not want libnuma to be used, invoke make with "has_libnuma=0"
has_libnuma=$(shell ldconfig -p | grep libnuma | wc -l)
ifneq ($(has_libnuma), 0)
    FLAGS += -DUSE_LIBNUMA
    LDFLAGS += -lnuma
endif

use_asan=0
ifneq ($(use_asan), 0)
    LDFLAGS += -fsanitize=address -static-libasan
endif

use_asserts=0
ifeq ($(use_asserts), 0)
    FLAGS += -DNDEBUG
endif

FLAGS += -DMAX_THREADS_POW2=256
FLAGS += -DCPU_FREQ_GHZ=2.1 #$(shell ./experiments/get_cpu_ghz.sh)
FLAGS += -DMEMORY_STATS=if\(1\) -DMEMORY_STATS2=if\(0\)
FLAGS += -std=c++14 -mcx16 -O2 -gdwarf
FLAGS += -fno-omit-frame-pointer
FLAGS += -fopenmp
#FLAGS += -DNO_CLEANUP_AFTER_WORKLOAD ### avoid executing data structure destructors, to save teardown time at the end of each trial (useful with massive trees)
#FLAGS += -DRAPID_RECLAMATION
FLAGS += -DPREFILL_INSERTION_ONLY
#FLAGS += -DMEASURE_REBUILDING_TIME
#FLAGS += -DMEASURE_TIMELINE_STATS
#FLAGS += -DUSE_TREE_STATS
#FLAGS += -DOVERRIDE_PRINT_STATS_ON_ERROR
#FLAGS += -Wno-format
FLAGS += $(xargs)

LDFLAGS += -L../lib
LDFLAGS += -I./ -I../ `find ../common -type d | sed s/^/-I/`
LDFLAGS += -lpthread
LDFLAGS += -ldl
LDFLAGS += -mrtm

bin_dir=bin
dir_guard:
	@mkdir -p $(bin_dir)

MINIMAL_BINS=$(patsubst ../ds/%/minimal_example.cpp,%.minimal,$(wildcard ../ds/*/minimal_example.cpp))
minimal_%: dir_guard
	$(GPP) $(FLAGS) ../ds/$(@:minimal_%=%)/minimal_example.cpp -o $(bin_dir)/$@ $(LDFLAGS)

DATA_STRUCTURES=$(patsubst ../ds/%/adapter.h,%,$(wildcard ../ds/*/adapter.h))
#DATA_STRUCTURES=flexlist_locks cbtree-concurrent-fair
#RECLAIMERS=debra none
RECLAIMERS=none
POOLS=none
ALLOCATORS=new

#DATA_STRUCTURES=$(patsubst ../ds/%/adapter.h,%,$(wildcard ../ds/brown_ext_ist*/adapter.h))
#DATA_STRUCTURES+=$(patsubst ../ds/%/adapter.h,%,$(wildcard ../ds/bronson*/adapter.h))
#DATA_STRUCTURES+=$(patsubst ../ds/%/adapter.h,%,$(wildcard ../ds/natarajan*/adapter.h))

#$(info $(DATA_STRUCTURES))

define make-custom-target =
write_$(1).alloc_$(2).reclaim_$(3).pool_$(4).out: dir_guard
	$(GPP) benchmark_write.cpp -o $(bin_dir)/write_$(1).alloc_$(2).reclaim_$(3).pool_$(4).out -I../ds/$1 -DDS_TYPENAME=$(1) -DALLOC_TYPE=$(2) -DRECLAIM_TYPE=$(3) -DPOOL_TYPE=$(4) $(FLAGS) $(LDFLAGS)
bench_$(1).alloc_$(2).reclaim_$(3).pool_$(4).out: dir_guard
	$(GPP) benchmark.cpp -o $(bin_dir)/bench_$(1).alloc_$(2).reclaim_$(3).pool_$(4).out -I../ds/$1 -DDS_TYPENAME=$(1) -DALLOC_TYPE=$(2) -DRECLAIM_TYPE=$(3) -DPOOL_TYPE=$(4) $(FLAGS) $(LDFLAGS)
all:: write_$(1).alloc_$(2).reclaim_$(3).pool_$(4).out bench_$(1).alloc_$(2).reclaim_$(3).pool_$(4).out
endef

$(foreach ds,$(DATA_STRUCTURES),$(foreach alloc,$(ALLOCATORS),$(foreach reclaim,$(RECLAIMERS),$(foreach pool,$(POOLS),$(eval $(call make-custom-target,$(ds),$(alloc),$(reclaim),$(pool)))))))

clean:
	rm $(bin_dir)/*.out
