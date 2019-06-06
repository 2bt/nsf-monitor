#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <functional>
#include "cpu.hpp"

class MyCPU : public CPU {
public:
    void jsr(uint16_t npc, uint8_t na, std::function<void(uint16_t, uint8_t)> f = nullptr) {
        callback = f;
        CPU::jsr(npc, na);
    }
    std::vector<uint8_t>        rom = std::vector<uint8_t>(0x8000);
    std::array<uint8_t, 0x8000> ram = {};

    void setmem(uint16_t addr, uint8_t value) override {
        if (addr >= 0x8000) {
            int base = ram[0x5ff0 + (addr >> 12)] << 12;
            rom[base + (addr & 0x0fff)] = value;
            return;
        }
        if (callback) callback(addr, value);
        ram[addr] = value;
    }
    uint8_t getmem(uint16_t addr) override {
        if (addr >= 0x8000) {
            int base = ram[0x5ff0 + (addr >> 12)] << 12;
            return rom[base + (addr & 0x0fff)];
        }
        return ram[addr];
    }
private:
    std::function<void(uint16_t, uint8_t)> callback;
};



struct Record {

    struct State {
        std::array<uint8_t, 22> is_set = {};
        std::array<uint8_t, 22> reg    = {};
        std::array<uint8_t, 8>  bank   = {};
    };

    MyCPU              cpu;
    std::vector<State> states;
    std::string        song_name;
    int                song_count;
    int                song_nr;

    bool load(const char* filename, int song_nr = 0);

};
