#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>
#include <sys/resource.h>

#include "hashpack.h"
#include "hashmap.h"

using namespace std;
using namespace rigtorp;

inline uint64_t RDTSC_START() {
    unsigned cyc_high, cyc_low;
    __asm volatile("cpuid\n\t"
                   "rdtsc\n\t"
                   "mov %%edx, %0\n\t"
                   "mov %%eax, %1\n\t"
                   : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx",
                   "%rdx");
    return ((uint64_t)cyc_high << 32) | cyc_low;
}

inline uint64_t RDTSC_FINAL() {
    unsigned cyc_high, cyc_low;
    __asm volatile("rdtscp\n\t"
                   "mov %%edx, %0\n\t"
                   "mov %%eax, %1\n\t"
                   "cpuid\n\t"
                   : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx",
                   "%rdx");
    return ((uint64_t)cyc_high << 32) | cyc_low;
}



template <typename ht>
void populate(ht & h, vector<uint64_t> & keys) {
    for(size_t i = 0; i < keys.size(); ++i) {
        h.emplace(keys[i], i);
    }
}

template <typename ht>
size_t  query(ht & h, vector<uint64_t> & keys) {
    size_t sum = 0;
    for(size_t i = 0; i < keys.size(); ++i) {
        auto it = h.find(keys[i]);
        if(it != h.end()) sum+= it->second;
    }
    return sum;
}

template <typename ht>
double query_avg_probed_keys(ht & h, vector<uint64_t> & keys) {
    size_t sum = 0;
    for(size_t i = 0; i < keys.size(); ++i) {
        sum += h.probed_keys(keys[i]);
    }
    return sum * 1.0 / keys.size();
}

const uint64_t EMPTY = 0xFFFFFFFFFFFFFFFF;


template <typename Pack>
void basic(std::vector<uint64_t> & keys,  const float loadfactor, const int repeat) {
    double querycycles = 0;
    double probespercycle = 0;
    size_t howmany = keys.size();
    std::cout << "testing "  << setw(20) << string(Pack::NAME) << " ";
    std::cout.flush();
    for(int r = 0 ; r < repeat; ++r) {

        HashMap<uint64_t, uint32_t, Pack> hm(16, EMPTY, loadfactor);
        uint64_t  cycles_start, cycles_end;
        cycles_start = RDTSC_START();
        populate(hm,keys);
        cycles_end = RDTSC_FINAL();
        cycles_start = RDTSC_START();
        query(hm,keys);
        cycles_end = RDTSC_FINAL();
        querycycles += (cycles_end - cycles_start) * 1.0 / howmany;

        double pk = query_avg_probed_keys(hm,keys);
        probespercycle += pk;
    }
    std::cout << "     " << setw(10) << querycycles / repeat << " cycles per query on average " ;
    std::cout << "     " << setw(10) << probespercycle / repeat << " probes per query on average " ;
    std::cout << std::endl;

}

void demorandom(const uint64_t howmany) {
    srand(0);
    const float loadfactor = 0.9;
    const float repeat = 1000;
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(get64rand());// formally, we should make sure that the key differs from EMPTY
    }
    std::cout << "populating a hash table with " << howmany << " random 64-bit keys and then retrieving them. " << std::endl;
    std::cout << "load factor = " << loadfactor << std::endl;
    std::cout << "We repeat with " << repeat << " different hash functions, using the same keys." << std::endl;


    basic<Zobrist64Pack>(keys, loadfactor, repeat);

    basic<MultiplyShift64Pack>(keys, loadfactor, repeat);
    basic<ClLinear64Pack>(keys, loadfactor, repeat);
    basic<ThorupZhangCWCubic64Pack>(keys, loadfactor, repeat);

    std::cout << std::endl;

}

void demofixed(const uint64_t howmany) {
    srand(0);
    const float loadfactor = 0.9;
    const float repeat = 1000;
    std::vector<uint64_t>  keys;
    for(uint64_t i = 1; i <= howmany; ++i) {
        keys.push_back(i + UINT64_C(0xFFFFFFFFF));
    }
    std::cout << "populating a hash table with " << howmany << " sequential 64-bit keys and then retrieving them. " << std::endl;
    std::cout << "load factor = " << loadfactor << std::endl;
    std::cout << "We repeat with " << repeat << " different hash functions, using the same keys." << std::endl;


    basic<Zobrist64Pack>(keys, loadfactor, repeat);

    basic<MultiplyShift64Pack>(keys, loadfactor, repeat);
    basic<ClLinear64Pack>(keys, loadfactor, repeat);

    basic<ThorupZhangCWCubic64Pack>(keys, loadfactor, repeat);

    std::cout << std::endl;

}


int main() {
    demorandom(2000);
    demorandom(64000);

    demofixed(2000);
    demofixed(64000);

}
