#ifndef LUNA_H_
#define LUNA_H_

#include "common.h"
#include "parser.h"
#include "core.h"

LUNA_API Luna *luna_init(void);
LUNA_API void luna_clean(Luna *L);

LUNA_API void luna_readfile(Luna *L, int options);
LUNA_API void luna_translator(Luna *L, Arena *a);
LUNA_API void luna_interpreter(Luna *L, int options);
LUNA_API int luna_bytecode_out(Luna *L, const char *file_path);
LUNA_API int luna_load_bytecode(Luna *L);

#endif /* LUNA_H_ */
