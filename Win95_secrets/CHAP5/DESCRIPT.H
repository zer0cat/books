//==================================
// PHYS - Matt Pietrek 1995
// FILE: DESCRIPT.H
//==================================

#ifndef __DESCRIPT_H
#define __DESCRIPT_H

#pragma pack(1)

typedef struct
{
    unsigned short  limit_0_15;
    unsigned short  base_0_15;
    unsigned char   base_16_23;

    unsigned int    accessed    : 1;
    unsigned int    readable    : 1;
    unsigned int    conforming  : 1;
    unsigned int    code_data   : 1;
    unsigned int    app_system  : 1;
    unsigned int    dpl         : 2;
    unsigned int    present     : 1;

    unsigned int    limit_16_19 : 4;
    unsigned int    unused      : 1;
    unsigned int    always_0    : 1;
    unsigned int    seg_16_32   : 1;
    unsigned int    granularity : 1;

    unsigned char   base_24_31;
} CODE_SEG_DESCRIPTOR;

typedef struct
{
    unsigned short  offset_0_15;
    unsigned short  selector;

    unsigned int    param_count : 4;
    unsigned int    some_bits   : 4;

    unsigned int    type        : 4;
    unsigned int    app_system  : 1;
    unsigned int    dpl         : 2;
    unsigned int    present     : 1;

    unsigned short  offset_16_31;
} CALLGATE_DESCRIPTOR;

unsigned short GetLDTAlias(void);

typedef void (_far _pascal * GENERIC_PROC)(void);

GENERIC_PROC CreateCallgate(void far * func_address, unsigned params);
void FreeCallgate( GENERIC_PROC callgate_ptr );

#endif
