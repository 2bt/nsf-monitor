// this code is based on kb's 6502 code in tinysid, i think.
#pragma once
#include <cinttypes>
#include <array>

class CPU {
public:
    void jsr(uint16_t npc, uint8_t na);

    std::array<uint8_t, 65536> memory;

protected:
    virtual uint8_t getmem(uint16_t addr) {
        return memory[addr];
    }

    virtual void setmem(uint16_t addr, uint8_t value) {
        memory[addr] = value;
    }

private:
    uint8_t getaddr(int mode);
    void    setaddr(int mode, uint8_t val);
    void    putaddr(int mode, uint8_t val);
    void    setflags(int flag, int cond);
    void    push(uint8_t val);
    uint8_t pop();
    void    branch(int flag);
    void    parse(uint8_t opc);

    int      cycles;
    uint8_t  a, x, y, s, p;
    uint16_t pc;
};
