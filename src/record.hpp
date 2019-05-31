#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct Record {

    struct State {
        uint8_t is_set[16] = {};
        uint8_t reg[16] = {};
    };

    std::vector<State> states;
    std::string        song_name;
    int                song_count;
    int                song_nr;

    bool load(const char* filename, int song_nr = -1);

};
