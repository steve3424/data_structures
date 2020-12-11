#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include "engine.h"
#include "platform_api.h"
#include "win32_main.h"
#define STB_IMAGE_IMPLEMENTATION
#include "..\\include\\stb_image.h"
#include "..\\include\\glm\\glm.hpp"
#include "..\\include\\glm\\gtc\\matrix_transform.hpp"
#include "..\\include\\glm\\gtc\\type_ptr.hpp"
#include "opengl.cpp"
#include "binary_tree.h"


#define WORKING_DIRECTORY "C:\\dev\\data_structures\\build"
#define LOG_FOLDER "C:\\dev\\data_structures\\logs\\"

#define TREE_SIZE 15

GLOBAL bool global_running;
GLOBAL int64_t global_counter_frequency;


DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_Win32FreeFileMemory) {
	if (file_memory) {
		VirtualFree(file_memory, 0, MEM_RELEASE);
	}
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_Win32ReadEntireFile) {
	DEBUG_ReadFileResult result = {};

	HANDLE file_handle = CreateFileA(file_name, 
					GENERIC_READ, FILE_SHARE_READ, 0,
					OPEN_EXISTING, 0, 0);

	if (file_handle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(file_handle, &file_size)) {
			uint32_t file_size32 = SafeTruncateUInt64(file_size.QuadPart); 
			result.contents = VirtualAlloc(0, file_size32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			if (result.contents) {
				DWORD bytes_read;
				if (ReadFile(file_handle, result.contents, (DWORD)file_size.QuadPart, &bytes_read, 0) && (file_size32 == bytes_read)) {
					// SUCCESS !!
					result.content_size = file_size32;
				}
				else {
					DEBUG_Win32FreeFileMemory(result.contents);
					result.contents = NULL;
				}
			}
			else {
				// VirtualAlloc() error
			}
		}
		else {
			// GetFileSize() error
		}

		CloseHandle(file_handle);
	}
	else {
		// CreateFile() error
	}

	return result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_Win32WriteEntireFile) {
	bool result = false;
	HANDLE file_handle = CreateFileA(file_name, 
					GENERIC_WRITE, 0, 0,
					CREATE_ALWAYS, 0, 0);

	if (file_handle != INVALID_HANDLE_VALUE) {
		DWORD bytes_written;
		if (WriteFile(file_handle, file_memory, memory_size, &bytes_written, 0)) {
			result = (bytes_written == memory_size);
		}
		else {
			// WriteFile() error
		}

		CloseHandle(file_handle);
	}
	else {
		// CreateFile() error
	}

	return result;

}

DEBUG_PLATFORM_OUTPUT_DEBUG_STRING(DEBUG_Win32OutputDebugString) {
	OutputDebugStringA(debug_string);
}

INTERNAL HANDLE LogCreateFile() {
	SYSTEMTIME current_time = {0};
	GetLocalTime(&current_time);
	char log_file_name[256] = LOG_FOLDER;
	_snprintf_s(log_file_name + (int)strlen(log_file_name), 
		sizeof(log_file_name), 
		sizeof(log_file_name), 
		"log_%d%d%d-%d%d%d.txt", 
		(int)(current_time.wMonth), 
		(int)(current_time.wDay), 
		(int)(current_time.wYear), 
		(int)(current_time.wHour), 
		(int)(current_time.wMinute), 
		(int)(current_time.wSecond));

	HANDLE log_file = CreateFileA(log_file_name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	
	char log_file_header[256];
	_snprintf_s(log_file_header, sizeof(log_file_header), "LOG FILE CREATED ON %d-%d-%d---%d:%d:%d\r\n", (int)(current_time.wMonth), (int)(current_time.wDay), (int)(current_time.wYear), (int)(current_time.wHour), (int)(current_time.wMinute), (int)(current_time.wSecond));
	DWORD bytes_written;
	WriteFile(log_file, log_file_header, (int)strlen(log_file_header), &bytes_written, 0);

	char current_directory[256] = "CURRENT DIRECTORY: ";
	GetCurrentDirectory(sizeof(current_directory), current_directory + (int)strlen(current_directory));
	int num_characters = (int)strlen(current_directory);
	current_directory[num_characters] = '\r';
	current_directory[num_characters + 1] = '\n';
	WriteFile(log_file, current_directory, (int)strlen(current_directory), &bytes_written, 0);

	return log_file;
}

INTERNAL inline void LogFunctionBegin(HANDLE log_file, char* function_name) {
	if (log_file == INVALID_HANDLE_VALUE) {
		return;
	}

	SYSTEMTIME current_time = {0};
	GetLocalTime(&current_time);
	char time_stamp[256];
	_snprintf_s(time_stamp,
		sizeof(time_stamp), 
		sizeof(time_stamp), 
		"\r\ncalled %d:%d:%d\r\n", 
		(int)(current_time.wHour), 
		(int)(current_time.wMinute), 
		(int)(current_time.wSecond));

	DWORD bytes_written;
	WriteFile(log_file, time_stamp, (int)strlen(time_stamp), &bytes_written, 0);
	WriteFile(log_file, function_name, (int)strlen(function_name), &bytes_written, 0);
	WriteFile(log_file, "() {\r\n", 6, &bytes_written, 0);
}

INTERNAL inline void LogFunctionEnd(HANDLE log_file) {
	if (log_file == INVALID_HANDLE_VALUE) {
		return;
	}

	DWORD bytes_written;
	WriteFile(log_file, "}\r\n", 3, &bytes_written, 0);
}

INTERNAL inline void LogFunctionLine(HANDLE log_file, char* line) {
	if (log_file == INVALID_HANDLE_VALUE) {
		return;
	}

	DWORD bytes_written;
	WriteFile(log_file, "\t", 1, &bytes_written, 0);
	WriteFile(log_file, line, (int)strlen(line), &bytes_written, 0);
	WriteFile(log_file, "\r\n", 2, &bytes_written, 0);
}

INTERNAL void Win32BeginRecordingInput(Win32State* win32_state, int input_recording_index) {
	win32_state->input_recording_index = input_recording_index;

	char* file_name = "input.rec";
	win32_state->recording_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

	DWORD bytes_to_write = (DWORD)win32_state->total_size;
	Assert(win32_state->total_size == bytes_to_write);
	DWORD bytes_written;
	WriteFile(win32_state->recording_handle, win32_state->game_memory, bytes_to_write, &bytes_written, 0);
}


INTERNAL void Win32EndRecordingInput(Win32State* win32_state) {
	CloseHandle(win32_state->recording_handle);
	win32_state->input_recording_index = 0;
}

INTERNAL void Win32RecordInput(Win32State* win32_state, GameInput* new_input) {
	DWORD bytes_written = 0;
	WriteFile(win32_state->recording_handle, new_input, sizeof(*new_input), &bytes_written, 0);
}

INTERNAL void Win32BeginPlaybackInput(Win32State* win32_state, int input_playback_index) {
	win32_state->input_playback_index = input_playback_index;

	char* file_name = "input.rec";
	win32_state->playback_handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	DWORD bytes_to_read = (DWORD)win32_state->total_size;
	Assert(win32_state->total_size == bytes_to_read);
	DWORD bytes_read;
	ReadFile(win32_state->playback_handle, win32_state->game_memory, bytes_to_read, &bytes_read, 0);
}

INTERNAL void Win32EndPlaybackInput(Win32State* win32_state) {
	CloseHandle(win32_state->playback_handle);
	win32_state->input_playback_index = 0;
}

INTERNAL void Win32PlaybackInput(Win32State* win32_state, GameInput* new_input) {
	DWORD bytes_read = 0;
	if (ReadFile(win32_state->playback_handle, new_input, sizeof(*new_input), &bytes_read, 0)) {
		if (bytes_read == 0) {
			int playback_index = win32_state->input_playback_index;
			Win32EndPlaybackInput(win32_state);
			Win32BeginPlaybackInput(win32_state, playback_index);
			ReadFile(win32_state->playback_handle, new_input, sizeof(*new_input), &bytes_read, 0);
		}
	}
}

INTERNAL inline FILETIME GetLastWriteTime(char* file_name) {
	FILETIME last_write_time = {};

	WIN32_FILE_ATTRIBUTE_DATA data;
	if (GetFileAttributesEx(file_name, GetFileExInfoStandard, &data)) {
		last_write_time = data.ftLastWriteTime;
	}

	return last_write_time;
}

INTERNAL Win32GameCode Win32LoadGameCode(char* source_dll_name, HANDLE log_file) {
	LogFunctionBegin(log_file, __func__);
	
	Win32GameCode result = {};
	result.dll_last_write_time = GetLastWriteTime(source_dll_name);

	char last_write_time[256];
	_snprintf_s(last_write_time, sizeof(last_write_time), "last write time low: %d\r\n\tlast write time high: %d", (int)result.dll_last_write_time.dwLowDateTime, (int)result.dll_last_write_time.dwHighDateTime);
	LogFunctionLine(log_file, last_write_time);

	char* temp_dll_name = "engine_temp.dll";
	CopyFile(source_dll_name, temp_dll_name, FALSE);

	result.game_code_dll = LoadLibraryA(temp_dll_name);
	if (result.game_code_dll) {
		LogFunctionLine(log_file, "game code loaded");

		result.LoadPlatformAPI = (game_load_platform_api*)GetProcAddress(result.game_code_dll, "GameLoadPlatformAPI");
		result.UpdateAndRender = (game_update_and_render*)GetProcAddress(result.game_code_dll, "GameUpdateAndRender");
		result.GlewInit = (game_glew_init*)GetProcAddress(result.game_code_dll, "GameGlewInit");

		result.is_valid = (result.LoadPlatformAPI && result.UpdateAndRender && result.GlewInit);
	}

	if (!result.is_valid) {
		LogFunctionLine(log_file, "game code not loaded");

		result.LoadPlatformAPI = GameLoadPlatformAPIStub;
		result.UpdateAndRender = GameUpdateAndRenderStub; 
		result.GlewInit = GameGlewInitStub;
	}

	LogFunctionEnd(log_file);
	return result;
}

INTERNAL void Win32UnloadGameCode(Win32GameCode* game_code) {
	if(game_code->game_code_dll) {
		FreeLibrary(game_code->game_code_dll);
		game_code->game_code_dll = 0;
	}

	game_code->is_valid = false;
	game_code->LoadPlatformAPI = GameLoadPlatformAPIStub;
	game_code->UpdateAndRender = GameUpdateAndRenderStub; 
	game_code->GlewInit = GameGlewInitStub;
}


INTERNAL Win32WindowDimensions Win32GetWindowDimension(HWND Window) {
	Win32WindowDimensions dims;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	dims.width = ClientRect.right - ClientRect.left;
	dims.height = ClientRect.bottom - ClientRect.top;

	return dims;
}

/*
Explanation of input system because this always seems to confuse me when I go back to it.

Initial solution:
	- Reset GameInput each frame.
	- Process the message queue each frame and just pass the values of each keypress event to the GameInput
	- BENEFITS:
		- Easy to understand and implement
		- Has built in repeat delay
	- DRAWBACK:
		- Cannot handle multiple keys pressed at once (E.g. holding right arrow and up arrow to move diagonally).
		  It only receives messages for the key that was pressed most recently. Since the GameInput is reset each
		  frame, this means one of the keys stops responding. I am not sure if this is a Windows thing or a 
		  keyboard hardware thing that would vary from keyboard to keyboard. Either way it is not good.

Solution:
	- Keep GameInput the same over each frame (I.e. assume it will be repeated).
	- Only process a message that indicates the key state changed (is_down != was_down)
	- In this case set the GameInput.GameButton.is_down property to whatever the message says and
	  reset the GameInput.GameButton.repeat_count to 0.
	- Implement repeat delay:
		- Look at every button that either has is_down true or has a positive repeat_count.
		- Check if they are in the dead zone (button is pressed but is being delayed) and modify the is_down
		  state based on that

*/

INTERNAL void Win32ProcessKeyboardMessage(GameButtonState* game_button, bool is_down, bool was_down) {
	game_button->is_down = is_down;
	game_button->repeat_count = 0;
}

INTERNAL void Win32ProcessPendingMessages(Win32State* win32_state, GameInput* new_input) {
	MSG message;
	while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		switch(message.message) {
			case WM_QUIT:
			{
				global_running = false;
			} break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				uint32_t vk_code = (uint32_t)message.wParam;
				bool was_down = ((message.lParam & (1 << 30)) != 0);
				bool is_down = (message.lParam & (1 << 31)) == 0;
				if(is_down != was_down) {
					switch (vk_code) {
						// CASES ARE FOR DVORAK LAYOUT

						case 'P':
						{
							Win32ProcessKeyboardMessage(&new_input->predecessor, is_down, was_down);
						} break;

						case 'S':
						{
							Win32ProcessKeyboardMessage(&new_input->successor, is_down, was_down);
						} break;


						case VK_OEM_COMMA:
						{
							Win32ProcessKeyboardMessage(&new_input->w_up, is_down, was_down);
						} break;

						case 'A':
						{
							Win32ProcessKeyboardMessage(&new_input->a_left, is_down, was_down);
						} break;

						case 'O':
						{
							Win32ProcessKeyboardMessage(&new_input->s_down, is_down, was_down);
						} break;

						case 'E':
						{
							Win32ProcessKeyboardMessage(&new_input->d_right, is_down, was_down);
						} break;

						case VK_UP:
						{
							Win32ProcessKeyboardMessage(&new_input->arrow_up, is_down, was_down);
						} break;

						case VK_DOWN:
						{
							Win32ProcessKeyboardMessage(&new_input->arrow_down, is_down, was_down);
						} break;

						case VK_LEFT:
						{
							Win32ProcessKeyboardMessage(&new_input->arrow_left, is_down, was_down);
						} break;

						case VK_RIGHT:
						{
							Win32ProcessKeyboardMessage(&new_input->arrow_right, is_down, was_down);
						} break;

						case VK_ESCAPE:
						{
							global_running = false;
						} break;

						case VK_SPACE:
						{
						} break;
#if DEBUG
						case 'R':
						{
							if (is_down) {
								bool recording = win32_state->input_recording_index == 1;
								bool playing_back = win32_state->input_playback_index == 1;
								if(!recording && !playing_back) {
									Win32BeginRecordingInput(win32_state, 1);
								}
								else if (recording && !playing_back) {
									Win32EndRecordingInput(win32_state);
									Win32BeginPlaybackInput(win32_state, 1);
								}
								else if (!recording && playing_back) {
									Win32EndPlaybackInput(win32_state);
									*new_input = {};
								}
							}
						} break;
#endif
					}

				}

				bool alt_key_was_down = (message.lParam & (1 << 29));
				if((vk_code == VK_F4) && alt_key_was_down) {
					global_running = false;
				}
			} break;

			default:
			{
				TranslateMessage(&message);
				DispatchMessageA(&message);
			} break;
		}

	}

	// REPEAT DELAY
	int repeat_sensitivity = 20;
	for(int i = 0; i < new_input->num_buttons; ++i) {
		if(new_input->buttons[i].is_down || new_input->buttons[i].repeat_count > 0) {
			new_input->buttons[i].repeat_count += 1;
			bool should_delay = (1 < new_input->buttons[i].repeat_count && new_input->buttons[i].repeat_count < repeat_sensitivity);
			if(should_delay) {
				new_input->buttons[i].is_down = false;
			}
			else {
				new_input->buttons[i].is_down = true;
			}
		}
	}
}

LRESULT CALLBACK WindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	
	switch (message) {
		case WM_CLOSE:
		{
			global_running = false;
		} break;

		case WM_ACTIVATEAPP:
		{
		} break;

		case WM_DESTROY:
		{
			global_running = false;
		} break;

		case WM_SIZE:
		{
			Win32WindowDimensions dim = Win32GetWindowDimension(window);
			glViewport(0, 0, dim.width, dim.height);
			
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(window, &paint);
			EndPaint(window, &paint);

		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			Assert(!"Keyboard input came in through a non-dispatch message!");
		} break;

		default:
		{
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return result;
}

inline LARGE_INTEGER Win32GetWallClock() {
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return result;
}

inline float Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
	float seconds_elapsed = ((float)(end.QuadPart - start.QuadPart) / (float)global_counter_frequency);
	return seconds_elapsed;
}

INTERNAL bool Win32InitOpenGL(HDC window_dc) {
	bool result = true;

	PIXELFORMATDESCRIPTOR desired_pixel_format = {};
	desired_pixel_format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	desired_pixel_format.nVersion = 1;
	desired_pixel_format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	desired_pixel_format.cColorBits = 32;
	desired_pixel_format.cAlphaBits = 8;
	desired_pixel_format.iLayerType = PFD_MAIN_PLANE;

	int suggested_pixel_format_index = ChoosePixelFormat(window_dc, &desired_pixel_format);
	PIXELFORMATDESCRIPTOR suggested_pixel_format; 
	DescribePixelFormat(window_dc, suggested_pixel_format_index, sizeof(suggested_pixel_format), &suggested_pixel_format);
	SetPixelFormat(window_dc, suggested_pixel_format_index, &suggested_pixel_format);

	HGLRC opengl_rc = wglCreateContext(window_dc);
	if(!wglMakeCurrent(window_dc, opengl_rc)) {
		result = false;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {

	SetCurrentDirectory(WORKING_DIRECTORY);

	LARGE_INTEGER perf_frequency;
	QueryPerformanceFrequency(&perf_frequency);
	global_counter_frequency = perf_frequency.QuadPart;
	UINT scheduler_granularity = 1; // milliseconds
	bool sleep_is_granular = timeBeginPeriod(scheduler_granularity) == TIMERR_NOERROR;
	int monitor_refresh_hz = 60;
	int game_update_hz = monitor_refresh_hz; // / 2;
	float target_seconds_per_frame = 1.0f / (float)game_update_hz;

	WNDCLASS WindowClass = {};
	WindowClass.lpfnWndProc = WindowCallback;
	WindowClass.hInstance = instance;
	WindowClass.lpszClassName = "data structures";

	if (RegisterClassA(&WindowClass)) {
		HWND Window = CreateWindowExA(WS_EX_CONTROLPARENT, 
						WindowClass.lpszClassName, 
						"data structures",
						WS_OVERLAPPEDWINDOW | WS_VISIBLE,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						0,
						0,
						instance,
						0);
		if (Window) {
			HDC device_context = GetDC(Window);

			bool opengl_initialized = Win32InitOpenGL(device_context);
			Win32WindowDimensions dim = Win32GetWindowDimension(Window);
			glViewport(0, 0, dim.width, dim.height);
			glewExperimental = GL_TRUE;
			bool glew_initialized = (glewInit() == GLEW_OK);


			HANDLE log_file = INVALID_HANDLE_VALUE; //LogCreateFile();
			char* source_dll_name = "engine.dll";
			Win32GameCode game = Win32LoadGameCode(source_dll_name, log_file);
			bool game_glew_initialized = game.GlewInit();

			PlatformAPI win32_platform_api = {};
			win32_platform_api.DEBUG_ReadEntireFile = DEBUG_Win32ReadEntireFile;
			win32_platform_api.DEBUG_FreeFileMemory= DEBUG_Win32FreeFileMemory;
			win32_platform_api.DEBUG_WriteEntireFile = DEBUG_Win32WriteEntireFile;
			win32_platform_api.DEBUG_OutputDebugString = DEBUG_Win32OutputDebugString;
			bool platform_api_loaded = game.LoadPlatformAPI(win32_platform_api);
#if DEBUG
			LPVOID base_address = (LPVOID)Terabytes(2);
#else
			LPVOID base_address = 0;
#endif
			GameMemory game_memory = {};
			game_memory.storage_size = Megabytes(1);
			game_memory.storage = VirtualAlloc(base_address, (size_t)game_memory.storage_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			Win32State win32_state = {};
			win32_state.total_size = game_memory.storage_size;
			win32_state.game_memory = game_memory.storage;

			if (game_memory.storage && 
			    opengl_initialized &&
			    glew_initialized &&
			    game_glew_initialized &&
			    platform_api_loaded) {

				LARGE_INTEGER last_counter = Win32GetWallClock();
				uint64_t last_cycle_counter = __rdtsc();

			
				GameInput new_input = {};
				GameState* game_state = (GameState*)game_memory.storage;
				//LoadQueueData(game_state);
				LoadBSTData(game_state);

				// INIT DATA STRUCTURE
				Node* nodes[TREE_SIZE] = {NULL};
				for(int i = 0; i < TREE_SIZE; ++i) {
					nodes[i] = AllocateNode(i, i);
				}
				BinaryTree bst = InitBinaryTree(nodes, TREE_SIZE);
				game_state->data_structure = (void*)&bst;

				int frame_count = 0;
				srand((unsigned int)time(NULL));

				// ***** MAIN LOOP *****
				global_running = true;
				while (global_running) {
					FILETIME new_dll_write_time = GetLastWriteTime(source_dll_name);
					if(CompareFileTime(&new_dll_write_time, &game.dll_last_write_time) != 0) {
						Win32UnloadGameCode(&game);
						game = Win32LoadGameCode(source_dll_name, log_file);
						game.GlewInit();
						game.LoadPlatformAPI(win32_platform_api);
					}

					Win32ProcessPendingMessages(&win32_state, &new_input);

					// automatic keyboard input for the queue
					/*
					new_input.a_left = {};
					new_input.d_right = {};
					if((frame_count % 90) == 0) {
						int r = rand() % 10;
						if(r > 3) {
							new_input.a_left.is_down = true;
						}
						else {
							new_input.d_right.is_down = true;
						}
					}
					frame_count++;
					*/

					if (win32_state.input_recording_index) {
						Win32RecordInput(&win32_state, &new_input);
					}
					else if (win32_state.input_playback_index) {
						Win32PlaybackInput(&win32_state, &new_input);
					}

					// for perspective projection matrix
					Win32WindowDimensions dims = Win32GetWindowDimension(Window);
					game_state->window_width = dims.width;
					game_state->window_height = dims.height;

					game.UpdateAndRender(&game_memory, &new_input);

					LARGE_INTEGER work_counter = Win32GetWallClock();
					float seconds_elapsed_for_work = Win32GetSecondsElapsed(last_counter, work_counter);
					float seconds_elapsed_for_frame = seconds_elapsed_for_work;
					if (seconds_elapsed_for_frame < target_seconds_per_frame) {
						if (sleep_is_granular) {
							DWORD ms_to_sleep = (DWORD)(1000.0f * (target_seconds_per_frame - seconds_elapsed_for_frame));
							if (ms_to_sleep > 0) {
								Sleep(ms_to_sleep);
							}
						}

						while (seconds_elapsed_for_frame < target_seconds_per_frame) {
							seconds_elapsed_for_frame = Win32GetSecondsElapsed(last_counter, Win32GetWallClock());
						}

					}
					else {
						// missed frame rate
						OutputDebugStringA("missed frame rate\n");
					}

					LARGE_INTEGER end_counter = Win32GetWallClock();
					float ms_per_frame = 1000.0f * Win32GetSecondsElapsed(last_counter, end_counter);
					last_counter = end_counter;

					SwapBuffers(device_context);

					char string_buffer[256];
					_snprintf_s(string_buffer, sizeof(string_buffer), "%.02f ms/f\n", ms_per_frame);
					//OutputDebugStringA(string_buffer);

					uint64_t end_cycle_counter = __rdtsc();
					uint64_t cycles_elapsed = end_cycle_counter - last_cycle_counter;
					last_cycle_counter = end_cycle_counter;

				}
			}
			else {
				// could not get one of these
				/*
			        game_memory.storage && 
			        opengl_initialized &&
			        glew_initialized &&
			        game_glew_initialized &&
			        platform_api_loaded
				*/
			}
		}
		else {
			// CreateWindowExA() error
		}
	}
	else {
		// RegisterClassA() error
	}

	return 0;
}
