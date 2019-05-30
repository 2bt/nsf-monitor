#pragma once
#include <cstdint>
#include <vector>

struct Record {

    struct State {
        uint8_t is_set[16] = {};
        uint8_t reg[16] = {};
    };

    std::vector<State> states;

    bool load(const char* filename, int song_nr);

};