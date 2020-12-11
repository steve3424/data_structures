#if !defined(PLATFORM_API_H)

#if DEBUG
struct DEBUG_ReadFileResult {
	uint32_t content_size;
	void* contents;
};

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DEBUG_ReadFileResult name(char* file_name)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_read_entire_file);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void* file_memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool name(char* file_name, void* file_memory, uint32_t memory_size)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#define DEBUG_PLATFORM_OUTPUT_DEBUG_STRING(name) void name(char* debug_string)
typedef DEBUG_PLATFORM_OUTPUT_DEBUG_STRING(debug_platform_output_debug_string);
#endif

struct PlatformAPI {
#if DEBUG
	debug_read_entire_file* DEBUG_ReadEntireFile;
	debug_platform_free_file_memory* DEBUG_FreeFileMemory;
	debug_platform_write_entire_file* DEBUG_WriteEntireFile;
	debug_platform_output_debug_string* DEBUG_OutputDebugString;
#endif
};



#define PLATFORM_API_H
#endif
