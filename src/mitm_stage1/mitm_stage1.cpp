#include <stdio.h>
#include <stdlib.h>

#include "crc32.h"
#include "mitm_stage1.h"

// info: the info about the archive to attack
// table: vector<vector<stage1a>> table(0x01000000)
void mitm_stage1a(archive_info& info, vector<vector<stage1a>>& table,
                  correct_guess* c) {
    // STAGE 1
    //
    // Guess s0, chunk2, chunk3 and carry bits.
    uint8_t xf0 = info.file[0].x[0];
    uint8_t xf1 = info.file[1].x[0];
    uint32_t extra(0);

    for (uint16_t s0 = 0; s0 < 0x100; ++s0) {
        fprintf(stderr, "%02x ", s0);
        if ((s0 & 0xf) == 0xf) {
            fprintf(stderr, "\n");
        }
        for (uint16_t chunk2 = 0; chunk2 < 0x100; ++chunk2) {
            for (uint16_t chunk3 = 0; chunk3 < 0x100; ++chunk3) {
                for (uint8_t carries = 0; carries < 0x10; ++carries) {
                    if (nullptr != c && s0 == c->sx[0][0] &&
                        chunk2 == c->chunk2 && chunk3 == c->chunk3 &&
                        carries == (c->carries >> 12)) {
                        fprintf(stderr, "On correct guess.\n");
                    }
                    uint8_t carryxf0 = carries & 1;
                    uint8_t carryyf0 = (carries >> 1) & 1;
                    uint8_t carryxf1 = (carries >> 2) & 1;
                    uint8_t carryyf1 = (carries >> 3) & 1;
                    uint32_t upper = 0x01000000;  // exclusive
                    uint32_t lower = 0x00000000;  // inclusive

                    uint32_t k0crc = chunk2;
                    uint32_t extra = 0;
                    uint8_t msbxf0 =
                        first_half_step(xf0, false, chunk3, carryxf0, k0crc,
                                        extra, upper, lower);
                    uint8_t yf0 = xf0 ^ s0;
                    k0crc = chunk2;
                    extra = 0;
                    uint8_t msbyf0 =
                        first_half_step(yf0, false, chunk3, carryyf0, k0crc,
                                        extra, upper, lower);
                    if (upper < lower) {
                        if (nullptr != c && s0 == c->sx[0][0] &&
                            chunk2 == c->chunk2 && chunk3 == c->chunk3 &&
                            carries == (c->carries >> 12)) {
                            fprintf(stderr,
                                    "Failed to get correct guess: s0 = %02x, "
                                    "chunk2 = %02x, "
                                    "chunk3 = "
                                    "%02x, carries = %x\n",
                                    s0, chunk2, chunk3, carries);
                        }
                        continue;
                    }
                    k0crc = chunk2;
                    extra = 0;
                    uint8_t msbxf1 =
                        first_half_step(xf1, false, chunk3, carryxf1, k0crc,
                                        extra, upper, lower);
                    if (upper < lower) {
                        if (nullptr != c && s0 == c->sx[0][0] &&
                            chunk2 == c->chunk2 && chunk3 == c->chunk3 &&
                            carries == (c->carries >> 12)) {
                            fprintf(stderr,
                                    "Failed to get correct guess: s0 = %02x, "
                                    "chunk2 = %02x, "
                                    "chunk3 = "
                                    "%02x, carries = %x\n",
                                    s0, chunk2, chunk3, carries);
                        }
                        continue;
                    }
                    uint8_t yf1 = xf1 ^ s0;
                    k0crc = chunk2;
                    extra = 0;
                    uint8_t msbyf1 =
                        first_half_step(yf1, false, chunk3, carryyf1, k0crc,
                                        extra, upper, lower);
                    if (upper < lower) {
                        if (nullptr != c && s0 == c->sx[0][0] &&
                            chunk2 == c->chunk2 && chunk3 == c->chunk3 &&
                            carries == (c->carries >> 12)) {
                            fprintf(stderr,
                                    "Failed to get correct guess: s0 = %02x, "
                                    "chunk2 = %02x, "
                                    "chunk3 = "
                                    "%02x, carries = %x\n",
                                    s0, chunk2, chunk3, carries);
                        }
                        continue;
                    }
                    uint32_t mk = toMapKey(msbxf0, msbyf0, msbxf1, msbyf1);
                    if (nullptr != c && s0 == c->sx[0][0] &&
                        chunk2 == c->chunk2 && chunk3 == c->chunk3 &&
                        carries == (c->carries >> 12)) {
                        fprintf(stderr,
                                "MSBs: %02x, %02x, %02x, %02x, Mapkey: %08x, "
                                "carries: %x, "
                                "c.carries: %04x\n",
                                msbxf0, msbyf0, msbxf1, msbyf1, mk, carries,
                                c->carries);
                    }
                    stage1a candidate = {uint8_t(s0), uint8_t(chunk2),
                                         uint8_t(chunk3), carries, msbxf0};
                    table[mk].push_back(candidate);
                }
            }
        }
    }
}

// info: the info about the archive to attack
// table: the output of mitm_stage1a
// candidates: an empty vector
// preimages: generated by build_preimages
void mitm_stage1b(archive_info& info, vector<vector<stage1a>>& table,
                  vector<stage1_candidate>& candidates,
                  vector<vector<uint16_t>>& preimages, correct_guess* c) {
    // Second half of MITM for stage 1
    bool found_correct = false;
    for (uint16_t s1xf0 = 0; s1xf0 < 0x100; ++s1xf0) {
        for (uint8_t prefix = 0; prefix < 0x40; ++prefix) {
            uint16_t pxf0(preimages[s1xf0][prefix]);
            if (nullptr != c && s1xf0 == c->sx[0][1]) {
                fprintf(stderr, "s1xf0: %02x, prefix: %04x    ", s1xf0, pxf0);
                if ((prefix & 3) == 3) {
                    fprintf(stderr, "\n");
                }
            }
            vector<uint8_t> firsts(0);
            uint8_t s1yf0 = s1xf0 ^ info.file[0].x[1] ^ info.file[0].h[1];
            second_half_step(pxf0, s1yf0, firsts, preimages);
            if (!firsts.size()) {
                continue;
            }
            for (uint16_t s1xf1 = 0; s1xf1 < 0x100; ++s1xf1) {
                vector<uint8_t> seconds(0);
                second_half_step(pxf0, s1xf1, seconds, preimages);
                if (!seconds.size()) {
                    continue;
                }
                vector<uint8_t> thirds(0);
                uint8_t s1yf1 = s1xf1 ^ info.file[1].x[1] ^ info.file[1].h[1];
                second_half_step(pxf0, s1yf1, thirds, preimages);
                if (!thirds.size()) {
                    continue;
                }
                for (auto f : firsts) {
                    for (auto s : seconds) {
                        for (auto t : thirds) {
                            uint32_t mapkey(f | (s << 8) | (t << 16));
                            for (stage1a candidate : table[mapkey]) {
                                stage1_candidate g;
                                g.chunk2 = candidate.chunk2;
                                g.chunk3 = candidate.chunk3;
                                g.cb1 = candidate.cb;
                                g.m1 =
                                    (candidate.msbk11xf0 * 0x01010101) ^ mapkey;

                                // Get ~4 possible solutions for lo24(k20) =
                                // chunks 1 and 4
                                //       A  B  C  D   k20
                                // ^  E  F  G  H      crc32tab[D]
                                //    ----------
                                //    I  J  K  L      crck20
                                // ^  M  N  O  P      crc32tab[msbk11xf0]
                                //    ----------
                                //    Q  R  S  T      (pxf0 << 2) matches k21xf0

                                // Starting at the bottom, derive 15..2 of KL
                                // from 15..2 of ST and OP
                                uint16_t crck20 =
                                    ((pxf0 << 2) ^
                                     crc32tab[candidate.msbk11xf0]) &
                                    0xfffc;

                                // Now starting at the top, iterate over 64
                                // possibilities for 15..2 of CD
                                for (uint8_t i = 0; i < 64; ++i) {
                                    uint32_t maybek20 =
                                        (preimages[candidate.s0][i] << 2);
                                    // and 4 possibilities for low two bits of D
                                    for (uint8_t lo = 0; lo < 4; ++lo) {
                                        // CD
                                        maybek20 = (maybek20 & 0xfffc) | lo;
                                        // L' = C ^ H
                                        uint8_t match =
                                            (maybek20 >> 8) ^
                                            crc32tab[maybek20 & 0xff];
                                        // If upper six bits of L == upper six
                                        // of L' then we have a candidate
                                        if ((match & 0xfc) == (crck20 & 0xfc)) {
                                            // KL ^ GH = BC.  (B = BC >> 8) &
                                            // 0xff.
                                            uint8_t b =
                                                ((crck20 ^
                                                  crc32tab[maybek20 & 0xff]) >>
                                                 8) &
                                                0xff;
                                            // BCD = (B << 16) | CD
                                            g.maybek20.push_back((b << 16) |
                                                                 maybek20);
                                        }
                                    }
                                }
                                if (0 == g.maybek20.size()) {
                                    continue;
                                }
                                candidates.push_back(g);

                                if (nullptr != c && s1xf0 == c->sx[0][1] &&
                                    s1xf1 == c->sx[1][1] &&
                                    candidate.s0 == c->sx[0][0] &&
                                    candidate.chunk2 == c->chunk2 &&
                                    candidate.chunk3 == c->chunk3 &&
                                    candidate.cb == (c->carries >> 12)) {
                                    found_correct = true;
                                    fprintf(stderr,
                                            "Correct candidates index = %lx\n",
                                            candidates.size() - 1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (c != nullptr && !found_correct) {
        fprintf(stderr,
                "Failed to use correct guess: s1xf0 = %02x, s1xf1 = %02x\n",
                c->sx[0][1], c->sx[1][1]);
    }
    fprintf(stderr, "Stage 1 candidates.size() == %04lx\n", candidates.size());
}
