#include "./include/ART.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

int main() {
    constexpr uint64_t MAX_KEYS = 1000000; // 1M keys
    constexpr int LOOKUP_ROUNDS = 20;
    Key *keys_array = new Key[MAX_KEYS];
    std::vector<uint64_t> order(MAX_KEYS);
    for (uint64_t i = 0; i < MAX_KEYS; i++) {
        keys_array[i].setInt(i + 1);
        order[i] = i;
    }
    std::mt19937_64 rng(42);
    std::shuffle(order.begin(), order.end(), rng);
    ART *art_index = new ART(keys_array, MAX_KEYS);

    // random insert
    auto ins_start = std::chrono::steady_clock::now();
    for (uint64_t i = 0; i < MAX_KEYS; i++) {
        art_index->insert(keys_array[order[i]], order[i]);
    }
    auto ins_end = std::chrono::steady_clock::now();
    double ins_us =
        std::chrono::duration<double, std::micro>(ins_end - ins_start).count();
    printf("Insert time=%lluus\n", (unsigned long long)ins_us);

    // random lookup
    std::shuffle(order.begin(), order.end(), rng);
    Key search_key;
    auto lk_start = std::chrono::steady_clock::now();
    for (int r = 0; r < LOOKUP_ROUNDS; r++) {
        for (uint64_t i = 0; i < MAX_KEYS; i++) {
            search_key.setInt(order[i] + 1);
            art_index->lookup(search_key);
        }
    }
    auto lk_end = std::chrono::steady_clock::now();
    double lk_us =
        std::chrono::duration<double, std::micro>(lk_end - lk_start).count();
    printf("Lookup time=%lluus\n", (unsigned long long)lk_us);

    delete art_index;
    delete[] keys_array;
    return 0;
}
