#pragma once

typedef struct {
    char* file;
    char* func;
    int line;
    u16 distance;
    u16 func_offset;
} symtable_info_t;

symtable_info_t get_symbol_info(u32 vaddr);
extern char* parse_map(u32 pc, u32 andOffset);
extern symtable_info_t walk_stack(u32* addr);
extern char* __symbolize(void* vaddr, char* buf, int size, u32 andOffset);
