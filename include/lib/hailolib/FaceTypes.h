#ifndef FACETYPES_H
#define FACETYPES_H

#include <cstdint>

#define USE_FLOAT16

#ifdef USE_FLOAT16
    #include <arm_neon.h>
    typedef float16_t SIMD_TYPE;
    typedef float16x8_t SIMD_VECTOR;
    #define LOAD_SIMD vld1q_f16
    #define SUB_SIMD vsubq_f16
    #define MUL_SIMD vmulq_f16
    #define ADD_SIMD vaddq_f16
    #define SUM_SIMD vaddvq_f16
    #define DUP_SIMD vdupq_n_f16
    #define STORE_SIMD vst1q_f16
    #define NUMELEM_VECTOR_SIMD 8
#elif defined(USE_INT16)
    #include <arm_neon.h>
    typedef int16_t SIMD_TYPE;
    typedef int16x8_t SIMD_VECTOR;
    #define LOAD_SIMD vld1q_s16
    #define SUB_SIMD vsubq_s16
    #define MUL_SIMD vmulq_s16
    #define ADD_SIMD vaddq_s16
    #define SUM_SIMD vaddvq_s16
    #define DUP_SIMD vdupq_n_s16
    #define STORE_SIMD vst1q_s16
    #define NUMELEM_VECTOR_SIMD 8
#else
    #include <arm_neon.h>
    typedef float SIMD_TYPE;
    typedef float32x4_t SIMD_VECTOR;
    #define LOAD_SIMD vld1q_f32
    #define SUB_SIMD vsubq_f32
    #define MUL_SIMD vmulq_f32
    #define ADD_SIMD vaddq_f32
    #define SUM_SIMD vaddvq_f32
    #define DUP_SIMD vdupq_n_f32
    #define STORE_SIMD vst1q_f32
    #define NUMELEM_VECTOR_SIMD 4
#endif

#define NUM_ELEMS_DESC_FACIAL 512

#endif
