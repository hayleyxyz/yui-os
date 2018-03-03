#pragma once

// for functions that don't have an "end" or don't want .cfi_startproc
#define LOCAL_FUNCTION_LABEL(x) .type x,STT_FUNC; x:
#define FUNCTION_LABEL(x) .global x; LOCAL_FUNCTION_LABEL(x)

#define LOCAL_FUNCTION(x) LOCAL_FUNCTION_LABEL(x) .cfi_startproc
#define FUNCTION(x) .global x; LOCAL_FUNCTION(x)

// for local or global functions
#define END_FUNCTION(x) .cfi_endproc; .size x, . - x

#define LOCAL_DATA(x) .type x,STT_OBJECT; x:
#define DATA(x) .global x; LOCAL_DATA(x)

// for local or global data
#define END_DATA(x) .size x, . - x