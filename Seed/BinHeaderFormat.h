
#ifndef _BIN_HEADER_FORMAT_H_
#define _BIN_HEADER_FORMAT_H_

#include <Windows.h>


#define BIN_FORMAT_UINT8		"u8"
#define BIN_FORMAT_UINT16		"u16"
#define BIN_FORMAT_UINT32		"u32"
#define BIN_FORMAT_UINT64		"u64"
#define BIN_FORMAT_SINT8		"i8"
#define BIN_FORMAT_SINT16		"i16"
#define BIN_FORMAT_SINT32		"i32"
#define BIN_FORMAT_SINT64		"i64"
#define BIN_FORMAT_FLOAT32		"f32"
#define BIN_FORMAT_FLOAT64		"f64"

#define BIN_HEADER_SIZE			1024

struct BIN_HEADER_LOCAL{
	char station[32]; //台站名
	char channel[8]; //分量名
	unsigned int numerator; //采样率分子
	unsigned int denominator; //采样率分母
	FILETIME tstart; //开始时间
	char datatype[8]; //文件中的数据类型
	char reserved[960];
};

#endif