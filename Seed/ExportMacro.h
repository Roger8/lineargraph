
#pragma once

#include <Windows.h>

//dll����seed file��
#ifdef CSEED_FILE_CLASS_IMPLEMENT
#define SEED_FILE_CLASS_LIB __declspec(dllexport)
#else
#define SEED_FILE_CLASS_LIB __declspec(dllimport)
#endif

