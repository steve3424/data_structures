#if !defined(WIN32_MAIN_H)


struct Win32WindowDimensions {
	int width;
	int height;
};

struct Win32GameCode {
	HMODULE game_code_dll;
	FILETIME dll_last_write_time;

	game_load_platform_api* LoadPlatformAPI;
	game_update_and_render* UpdateAndRender;
	game_glew_init* GlewInit;
	
	bool is_valid;
};

struct Win32State {
	uint64_t total_size;
	void* game_memory;

	HANDLE recording_handle;
	int input_recording_index;

	HANDLE playback_handle;
	int input_playback_index;
};

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_Win32FreeFileMemory);
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_Win32ReadEntireFile);
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_Win32WriteEntireFile);
DEBUG_PLATFORM_OUTPUT_DEBUG_STRING(DEBUG_Win32OutputDebugString);

#define WIN32_MAIN_H
#endif
