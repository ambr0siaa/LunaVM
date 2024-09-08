#ifndef CORE_H_
#define CORE_H_

#include "arena.h"
#include "common.h"

typedef u64 Address;

typedef enum {
    R0=0, R1, R2, R3, R4,
    R5,   R6, R7, R8, R9, RC
} reg_t;

#define LUNA_INIT_PROGRAM_CAP 128
#define LUNA_EXEC_LIMIT 27 * 10

LUNA_API void luna_core_init(Arena *a, Luna *L);

LUNA_API LObject luna_fetchO(Luna *L, LObject_Type type);
LUNA_API Instruction luna_fetchI(Luna *L);
LUNA_API Address luna_fetchA(Luna *L);
LUNA_API reg_t luna_fetchR(Luna *L);

LUNA_API void luna_exec_inst(Arena *a, Luna *L);

#endif /* CORE_H_ */
