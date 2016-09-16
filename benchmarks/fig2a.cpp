#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>

#include "hashpack.h"
#include "simple-hashmap.h"
#include "util.h"

using namespace std;

template <typename HashFamily, typename Key>
size_t probe_length(size_t log_num_keys, const vector<Key>& to_insert) {
    HashSet<Key, HashFamily> ht(1 + log_num_keys);
    size_t result = 0;
    for (const auto k : to_insert) {
        result += ht.Insert(k).second;
    }
    return result;
}

int main() {
  vector<size_t> results;
  static const size_t LOG_NUM_KEYS = 20;
  vector<uint64_t> to_insert(1 << LOG_NUM_KEYS);
  iota(to_insert.begin(), to_insert.end(), get64rand());
  {
      std::random_device rd;
      std::mt19937 g(rd());
      shuffle(to_insert.begin(), to_insert.end(), g);
  }
  for (int i = 0; i < 100; ++i) {
      results.push_back(
          probe_length<MultiplyShift64Pack>(LOG_NUM_KEYS, to_insert));
  }
  cout << "done" << endl;
  sort(results.begin(), results.end(), greater<uint64_t>());
  for (const auto v : results) {
    cout << v << endl;
  }
}
