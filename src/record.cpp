#include <functional>
#include <fstream>
#include "record.hpp"
#include "cpu.hpp"

struct Header {
    char     magic[5];
    uint8_t  version;
    uint8_t  song_count;
    uint8_t  start_song;
    uint16_t load_addr;
    uint16_t init_addr;
    uint16_t play_addr;
    char     song_name[32];
    char     artist_name[32];
    char     copyright[32];
    uint16_t speed_ntsc;
    uint8_t  bank[8];
    uint16_t speed_pal;
    uint8_t  speed_flags;
    uint8_t  chip_flags;
    uint8_t  reserved;
    uint8_t  size[3];
} __attribute__((packed));


class MyCPU : public CPU {
public:
    void jsr(uint16_t npc, uint8_t na, std::function<void(uint16_t, uint8_t)> f = nullptr) {
        callback = f;        
        CPU::jsr(npc, na);
    }
    std::array<uint8_t, 0x8000> rom = {};
    std::array<uint8_t, 0x8000> ram = {};

protected:
    void setmem(uint16_t addr, uint8_t value) override {
        if (addr >= 0x8000) {
            int base = ram[0x5ff0 + (addr >> 12)] << 12;
            rom[base + (addr & 0x0fff)] = value;
            return;
        }
        ram[addr] = value;
        //if (addr >= 0x5ff8 && addr < 0x6000) printf("BANK %x %x\n", addr, value);
        if (callback) callback(addr, value);
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


bool Record::load(const char* filename, int nr) {
    states.clear();

    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    if (!ifs.is_open()) {
        printf("error: could not open file\n");
        return false;
    }
    auto pulse_pos = ifs.tellg();
    std::vector<uint8_t> data(pulse_pos);
    ifs.seekg(0, std::ios::beg);
    ifs.read((char*) data.data(), pulse_pos);

    Header* h = (Header*) data.data();
    printf("version:     %d\n", h->version);
    printf("song count:  %d\n", h->song_count);
    printf("start song:  %d\n", h->start_song);
    printf("load addr:   %x\n", h->load_addr);
    printf("init addr:   %x\n", h->init_addr);
    printf("play addr:   %x\n", h->play_addr);
    printf("song name:   %s\n", h->song_name);
    printf("artist name: %s\n", h->artist_name);
    printf("copyright:   %s\n", h->copyright);
    printf("speed ntsc:  %d\n", h->speed_ntsc);
    printf("speed pal:   %d\n", h->speed_pal);
    printf("speed flags: %x\n", h->speed_flags);
    printf("bank:        %x %x %x %x %x %x %x %x\n",
            h->bank[0], h->bank[1], h->bank[2], h->bank[3], h->bank[4], h->bank[5], h->bank[6], h->bank[7]);
    printf("chip flags:  %x\n", h->chip_flags);

    song_nr    = nr >= 0 ? nr : h->start_song;
    song_count = h->song_count;
    song_name  = h->song_name;


    MyCPU cpu;

    // init banks
    int b = h->bank[0] | h->bank[1] | h->bank[2] | h->bank[3] | h->bank[4] | h->bank[5] | h->bank[6] | h->bank[7];
    for (int i = 0; i < 8; ++i) cpu.ram[0x5ff8 + i] = b ? h->bank[i] & 0x7 : i;
    size_t j = h->load_addr & (b ? 0x0fff : 0x7fff);
    for (size_t i = sizeof(Header); i < data.size() && j < cpu.rom.size(); ++i, ++j) {
        cpu.rom[j] = data[i];
    }

    cpu.jsr(h->init_addr, song_nr);

    for (int m = 0; m < 60 * 60 * 10; ++m) {
        State s;
        cpu.jsr(h->play_addr, 0, [&s](uint16_t addr, uint8_t value) {
            if (addr >= 0x4000 && addr < 0x4010)
            s.is_set[addr - 0x4000] = true;
        });
        printf("%02x\n", cpu.ram[0x4015]);

        for (int i = 0; i < 16; ++i) s.reg[i] = cpu.ram[0x4000 + i];
        states.emplace_back(s);
    }

    return true;
}
