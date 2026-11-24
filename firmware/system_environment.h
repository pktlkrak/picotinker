#pragma once
#ifndef PICO_BOARD

#include <setjmp.h>
jmp_buf before_application_loading;
#define ENVIRONMENT_IN_ROOT_FUNC_INIT setjmp(before_application_loading)

#else
#define ENVIRONMENT_IN_ROOT_FUNC_INIT
#endif
