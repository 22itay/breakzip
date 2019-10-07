/***
 * Author: Nash E. Foster <leaf@pyrofex.net>
 */

#include <vector>
#include <cstdint>

namespace breakzip {

    using namespace std;

    typedef struct guess {
        uint64_t stage1_bits : 42;
        uint32_t stage2_bits : 26;
        uint32_t stage3_bits : 18;
        uint32_t stage4_bits : 18;
    } guess_t;

    // Notation:
    // 
    // key00, key10, key20 are the keys after having processed the password.
    // Bytes produced by rand() are x0, x1, x2, ...
    // Stream bytes during first encryption are s0, s1x, s2x, ...
    // key0nx, key1nx, key2nx are the keys after having processed the password and
    // the first n bytes of x.
    //
    // y0 = x0 ^ s0, y1 = x1 ^ s1x, y2 = x2 ^ s2x, ...
    // Stream bytes during second encryption are s0, s1y, s2y, ...
    // Header bytes in zip file are h0 = y0 ^ s0 = x0, h1 = y1 ^ s1y, h2 = y2 ^ s2y, ...
    // key0ny, key1ny, key2ny are the keys after having processed the password and the
    // first n bytes of y.
    // 
    // stage 1:
    // 
    // We guess [chunk1 = bits 0..15 of key20 (16 bits)] 
    // We guess [chunk2 = LSB(CRC(key00, 0)) (8 bits)]
    // We guess [chunk3 = MSB(key10 * 0x08088405), carry for x, carry for y (10 bits)]
    // We guess [chunk4 = bits 16..23 of key20 (8 bits)]
    // (42 bits total)
    // 
    // From chunk1 we compute s0.
    // For each file
    // We get x0 from the header
    // From that and chunk2 we compute LSB(key01x) * 0x08088405 + 1
    // From that and chunk3 we compute MSB(key11x)
    // From that and chunk4 we compute s1x
    //           
    // y0 = x0 ^ s0
    // From that and chunk2 we compute LSB(key01y) * 0x08088405 + 1
    // From that and chunk3 we compute MSB(key11y)
    // From that and chunk4 we compute s1y
    // 
    // We compute x1 ^ s1x ^ s1y and compare it to h1.  If it's wrong, our guess was wrong.
    // 
    // We get 16 bits of filter from h1 in each of the two files.
    // We expect 2**{42 - 16} = 2**26 chunk1-4 tuples to pass.


    /***
     * stage1: begin with guess start and continue guessing until end. Each guess that
     * passes is placed into the output vector. Returns 1 if no error occurred. When
     * errors happen, returns 0 and sets errno.
     */

    int stage1(const uint64_t start, const uint64_t end, vector<guess_t> out);

    // stage 2:

    // We guess [chunk5 = bits 24..32 of key20 (8 bits)]
    // We guess [chunk6 = bits 16..23 of key00 (8 bits)]
    // We guess [chunk7 = MSB(key10 * 0xD4652819), carry for x,
    // carry for y (10 bits)]
    // (26 bits total)

    // Similar process as before, but filtering with h2 in each file.  (I'll
    // flesh this out later.)  We expect 2**{26 + 25 - 16} = 2**{35} chunk1-7
    // tuples to pass, 2**{26 + 25} = 2**51 work where the 26 in the exponent
    // is from stage 1 and the 25 from stage 2.


    /* stage2 depends on guesses from stage1. Each guess from the in vector
     * is used to generate a series of additional guesses. Each guess that
     * passed stage 1 will get 2^26 guesses in this stage. A guess that passes
     * stage2 will include 42 bits from stage 1 as a 64-bit integer and 26 bits
     * from stage2 as a 32-bit integer.
     */
    int stage2(const vector<guess_t> in, vector<guess_t> out);

    // stage 3:
    // We guess [chunk8 = bits 24..32 of key00 (8 bits)]
    // We guess [chunk9 = MSB(key10 * 0x576eac7d), carry for x, carry for y (10
    // bits)]
    // (18 bits total)

    // Similar process as before, but filtering with h3 in each file.  We
    // expect 2**{35 + 18 - 16} = 2**{37} chunk1-9 tuples to pass, 2**{35 + 18}
    // = 2**53 work where the 35 in the exponent is from stage 2 and the 18
    // from stage 3.

    /*
     * stage3 depends on guesses from stage2. 
     */
    int stage3(const vector<guess_t> in, vector<guess_t> out);


    // stage 4:
    // We guess [chunk10 = bits 0..7 of key00 (8 bits)]
    // We guess [chunk11 = MSB(key10 * 0x1201d271), carry for x, carry for y (10 bits)]
    // (18 bits total)

    // Similar process as before, but filtering with h3 in each file.  We
    // expect 2**{37 + 18 - 16} = 2**{39} chunk1-11 tuples to pass, 2**{37 +
    // 18} = 2**55 work where the 37 in the exponent is from stage 3 and the 18
    // from stage 4.

    int stage4(const vector<guess_t> in, vector<guess_t> out);

    // 
    // stage 5:
    // No guesses, just filtration with h4 in each file.  
    // We expect 2**{38 - 16} = 2**{22} chunk1-11 tuples to pass, 2**38 work
    // where the 38 in the exponent is from stage 4.
    int stage5(const vector<guess_t> in, vector<guess_t> out);

    // 
    // stage 6:
    // No guesses, just filtration with h5 in each file.  
    // We expect 2**{22 - 16} = 2**{6} chunk1-11 tuples to pass, 2**22 work
    // where the 22 in the exponent is from stage 5.
    int stage6(const vector<guess_t> in, vector<guess_t> out);

    // 
    // stage 7:
    // No guesses, just filtration with h5 in each file.  
    // We expect 2**{6 - 16} = 2**{-10} chunk1-11 tuples to pass, 2**6 work
    // i.e. only the right one, where the 6 in the exponent is from stage 6.
    // 
    int stage7(const vector<guess_t> in, vector<guess_t> out);

}; // namespace
