
#pragma once

#include <Windows.h>

//dllµ¼³öseed fileÀà
#ifdef CSEED_FILE_CLASS_IMPLEMENT
#define SEED_FILE_CLASS_LIB __declspec(dllexport)
#else
#define SEED_FILE_CLASS_LIB __declspec(dllimport)
#endif

