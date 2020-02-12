#ifndef GPU_STAGE3_H
#define GPU_STAGE3_H

#include <stdio.h>
#include <cstdint>
#include <vector>

#include "breakzip.h"

#ifdef __CUDACC__
#define CUDA_HOSTDEVICE __device__
#else
#define CUDA_HOSTDEVICE
#endif


using namespace std;

namespace stage3 {

uint8_t get_s0(uint16_t k20);

typedef struct keys {
    uint32_t crck00, k10, k20;
} keys;

CUDA_HOSTDEVICE void gpu_stage3(const mitm::archive_info &info,
                const mitm_stage2::stage2_candidate &c2,
                /* output */ keys *k,
                const mitm::correct_guess *c = nullptr);

CUDA_HOSTDEVICE void gpu_stage4(const mitm::archive_info &info,
                const mitm_stage2::stage2_candidate &c2, const uint16_t chunk8,
                const uint16_t chunk9, const uint8_t cb30, const uint8_t cb31,
                uint32_t crck00, uint32_t k20,
                /* output */ keys *k,
                const mitm::correct_guess *c = nullptr);

CUDA_HOSTDEVICE void gpu_stages5to10(const mitm::archive_info &info, const uint32_t crck00,
                     const uint32_t k10, const uint32_t k20,
                     /* output */ keys *k,
                     const mitm::correct_guess *c = nullptr);

};  // namespace gpu_stage3

#endif
