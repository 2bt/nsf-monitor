#include <fstream>
#include <cstring>
#include "record.hpp"

struct Header {
    uint8_t  magic[5];
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


bool Record::load(const char* filename, int nr) {
    states.clear();

    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    if (!ifs.is_open()) {
        printf("error: could not open file\n");
        return false;
    }
    auto pos = ifs.tellg();
    std::vector<uint8_t> data(pos);
    ifs.seekg(0, std::ios::beg);
    ifs.read((char*) data.data(), pos);

    Header* h = (Header*) data.data();
    if (memcmp(h->magic, "NESM\x1a", 5) != 0) {
        printf("error: wrong file format\n");
        return false;
    }
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

    song_nr    = nr > 0 ? nr : h->start_song;
    song_count = h->song_count;
    song_name  = h->song_name;


    // init banks
    int b = h->bank[0] | h->bank[1] | h->bank[2] | h->bank[3] | h->bank[4] | h->bank[5] | h->bank[6] | h->bank[7];
    for (int i = 0; i < 8; ++i) cpu.ram[0x5ff8 + i] = b ? h->bank[i] & 0x7 : i;
    size_t j = h->load_addr & (b ? 0x0fff : 0x7fff);
    for (size_t i = sizeof(Header); i < data.size(); ++i, ++j) {
        if (j >= cpu.rom.size()) cpu.rom.resize(cpu.rom.size() + 0x1000);
        cpu.rom[j] = data[i];
    }

    // set status register
    cpu.rom[0x4015] = 0x0f;

    // init song
    cpu.jsr(h->init_addr, song_nr - 1);

    // play song
    for (int m = 0; m < 60 * 60 * 10; ++m) {
        State s;
        cpu.jsr(h->play_addr, 0, [&s](uint16_t addr, uint8_t value) {
            if (addr >= 0x4000 && addr < 0x4000 + s.is_set.size())
            s.is_set[addr - 0x4000] = true;
        });
        for (size_t i = 0; i < s.reg.size(); ++i) s.reg[i] = cpu.ram[0x4000 + i];
        for (size_t i = 0; i < s.bank.size(); ++i) s.bank[i] = cpu.ram[0x5ff0 + i];
        states.emplace_back(s);
    }

    return true;
}
