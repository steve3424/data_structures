#ifndef WIN32_MAIN_H
#define WIN32_MAIN_H

struct Win32WindowDimensions {
	int width;
	int height;
};

struct Win32FileResult {
	int file_size;
	void* file;
};

INTERNAL void Win32FreeFileResult(Win32FileResult* file_result);
INTERNAL Win32FileResult Win32ReadEntireFile(const char* file_name);

#endif
