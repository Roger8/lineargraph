
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
	char station[32]; //̨վ��
	char channel[8]; //������
	unsigned int numerator; //�����ʷ���
	unsigned int denominator; //�����ʷ�ĸ
	FILETIME tstart; //��ʼʱ��
	char datatype[8]; //�ļ��е���������
	char reserved[960];
};

#endif