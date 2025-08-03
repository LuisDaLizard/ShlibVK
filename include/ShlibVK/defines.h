#pragma once

typedef unsigned char           u8_t;
typedef unsigned short          u16_t;
typedef unsigned int            u32_t;
typedef unsigned long           u64_t;

typedef char                    i8_t;
typedef short                   i16_t;
typedef int                     i32_t;
typedef long                    i64_t;

typedef float                   f32_t;
typedef double                  f64_t;

typedef char                    b8_t;
typedef int                     b32_t;

/** ENSURE ALL TYPES ARE THE CORRECT SIZE **/

static_assert(sizeof(u8_t)  == 1);
static_assert(sizeof(u16_t) == 2);
static_assert(sizeof(u32_t) == 4);
static_assert(sizeof(u64_t) == 8);

static_assert(sizeof(i8_t)  == 1);
static_assert(sizeof(i16_t) == 2);
static_assert(sizeof(i32_t) == 4);
static_assert(sizeof(i64_t) == 8);

static_assert(sizeof(f32_t) == 4);
static_assert(sizeof(f64_t) == 8);

static_assert(sizeof(b8_t)  == 1);
static_assert(sizeof(b32_t) == 4);