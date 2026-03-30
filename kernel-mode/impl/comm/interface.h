#pragma once

#define MODULE_NAME skCrypt("win32kbase.sys")
#define FUNCTION_NAME skCrypt("NtGdiPolyPolyDraw")
#define MAGIC_NUMBER 0xDEADBEEF

typedef enum _requests {
	invoke_read = 0x1,
	invoke_write = 0x2
}requests, * prequests;

typedef struct _read_invoke {
	uint32_t pid;
	uintptr_t address;
	void* buffer;
	size_t size;
} read_invoke, * pread_invoke;

typedef struct _write_invoke {
	uint32_t pid;
	uintptr_t address;
	void* buffer;
	size_t size;
} write_invoke, * pwrite_invoke;

typedef struct _invoke_data {
	uint32_t magic_number;
	requests code;
	void* data;
}invoke_data, * pinvoke_data;