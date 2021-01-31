#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include "engine.h"
#include "win32_main.h"
#include "binary_tree.h"
#include "array.h"
#define STB_IMAGE_IMPLEMENTATION
#include "..\\include\\stb_image.h"
#include "..\\include\\glm\\glm.hpp"
#include "..\\include\\glm\\gtc\\matrix_transform.hpp"
#include "..\\include\\glm\\gtc\\type_ptr.hpp"

#include "opengl.cpp"
#include "navigation.cpp"
#include "engine.cpp"

#define WORKING_DIRECTORY "C:\\dev\\data_structures\\build"

GLOBAL bool global_running;
GLOBAL int64_t global_counter_frequency;

INTERNAL Win32WindowDimensions Win32GetWindowDimension(HWND Window) {
	Win32WindowDimensions dims;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	dims.width = ClientRect.right - ClientRect.left;
	dims.height = ClientRect.bottom - ClientRect.top;

	return dims;
}

INTERNAL void Win32FreeFileResult(Win32FileResult* file_result) {
	file_result->file_size = 0;
	if (file_result->file) {
		VirtualFree(file_result->file, 0, MEM_RELEASE);
	}
}

INTERNAL Win32FileResult Win32ReadEntireFile(const char* file_name) {
	Win32FileResult result = {};

	HANDLE file_handle = CreateFileA(file_name, 
					GENERIC_READ, FILE_SHARE_READ, 0,
					OPEN_EXISTING, 0, 0);

	if (file_handle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(file_handle, &file_size)) {
			uint32_t file_size32 = (uint32_t)file_size.QuadPart; 
			result.file = VirtualAlloc(0, file_size32, 
						       MEM_RESERVE|MEM_COMMIT,
						       PAGE_READWRITE);
			if (result.file) {
				DWORD bytes_read;
				if (ReadFile(file_handle, result.file, 
					     (DWORD)file_size.QuadPart, 
					     &bytes_read, 0) && 
					     (file_size32 == bytes_read)) {
					// SUCCESS !!
					result.file_size = file_size32;
				}
				else {
					Win32FreeFileResult(&result);
					result.file = NULL;
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
/*
Explanation of input system because this 
always seems to confuse me when I go back to it.

Initial solution:
	- Reset GameInput each frame.
	- Process the message queue each frame and just pass the values of 
	  each keypress event to the GameInput

	- BENEFITS:
		- Easy to understand and implement
		- Has built in repeat delay
	- DRAWBACK:
		- Cannot handle multiple keys pressed at once 
		  (E.g. holding right arrow and up arrow to move diagonally).
		  It only receives messages for the key that was pressed 
		  most recently. Since the GameInput is reset each
		  frame, this means one of the keys stops responding. 
		  I am not sure if this is a Windows thing or a 
		  keyboard hardware thing that would vary from 
		  keyboard to keyboard. Either way it is not good.

Solution:
	- Keep GameInput the same over each frame 
	  (I.e. assume it will be repeated).
	- Only process a message that indicates the 
	  key state changed (is_down != was_down)
	- In this case set the GameInput.GameButton.is_down 
	  property to whatever the message says and
	  reset the GameInput.GameButton.repeat_count to 0.
	- Implement repeat delay:
		- Look at every button that either has is_down true or 
		  has a positive repeat_count.
		- Check if they are in the dead zone 
		  (button is pressed but is being delayed) and 
		  modify the is_down state based on that.
*/

INTERNAL void Win32ProcessKeyboardMessage(GameButtonState* game_button, bool is_down, bool was_down) {
	game_button->is_down = is_down;
	game_button->repeat_count = 0;
}

INTERNAL void Win32ProcessPendingMessages(GameInput* new_input) {
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
							Win32ProcessKeyboardMessage(&new_input->p, is_down, was_down);
						} break;

						case 'S':
						{
							Win32ProcessKeyboardMessage(&new_input->s, is_down, was_down);
						} break;


						case VK_OEM_COMMA:
						{
							Win32ProcessKeyboardMessage(&new_input->comma, is_down, was_down);
						} break;

						case 'A':
						{
							Win32ProcessKeyboardMessage(&new_input->a, is_down, was_down);
						} break;

						case 'O':
						{
							Win32ProcessKeyboardMessage(&new_input->o, is_down, was_down);
						} break;

						case 'E':
						{
							Win32ProcessKeyboardMessage(&new_input->e, is_down, was_down);
						} break;

						case 'W':
						{
							Win32ProcessKeyboardMessage(&new_input->w, is_down, was_down);
						} break;

						case 'V':
						{
							Win32ProcessKeyboardMessage(&new_input->v, is_down, was_down);
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
		if(Window) {
			HDC device_context = GetDC(Window);

			bool opengl_initialized = Win32InitOpenGL(device_context);
			Win32WindowDimensions dim = Win32GetWindowDimension(Window);
			glViewport(0, 0, dim.width, dim.height);
			glewExperimental = GL_TRUE;
			bool glew_initialized = (glewInit() == GLEW_OK);


			LPVOID base_address = 0;
			GameMemory game_memory = {};
			game_memory.storage_size = Megabytes(1);
			game_memory.storage = VirtualAlloc(base_address,
							   (size_t)game_memory.storage_size, 
							   MEM_RESERVE|MEM_COMMIT, 
							   PAGE_READWRITE);

			if(game_memory.storage && 
			    opengl_initialized &&
			    glew_initialized) {

				LARGE_INTEGER last_counter = Win32GetWallClock();
				uint64_t last_cycle_counter = __rdtsc();

			
				GameInput new_input = {};
				GameState* game_state = (GameState*)game_memory.storage;
				game_state->current_view = BINARY_TREE;

				srand((unsigned int)time(NULL));
				LoadAllData(game_state);

				// BINARY TREE INIT
				const int tree_size = 20;
				Node** nodes = (Node**)malloc(sizeof(Node*) * tree_size);
				for(int i = 0; i < tree_size; ++i) {
					nodes[i] = AllocateNode(i, i);
				}
				BinaryTree bst = InitBinaryTree(nodes, tree_size);
				game_state->data_structure[BINARY_TREE] = &bst;
				free(nodes);

				// QUEUE INIT
				//LoadQueueData(game_state);
				QueueInt q = CreateQueueInt(10);
				game_state->data_structure[QUEUE] = &q;

				// ARRAY INIT
				//LoadArrayData(game_state);
				ArrayStruct a = {};
				a.size = 10;
				a.array = (int*)malloc(sizeof(int) * a.size);
				if(a.array) {
					for(int i = 0; i < a.size; ++i) {
						a.array[i] = rand() % MAX_DIGITS;
					}
				}
				a.state = STATIC;
				a.selected_value_index = -1;
				a.compare_value_index = -1;
				game_state->data_structure[INSERTION_SORT] = &a;


				// ***** MAIN LOOP *****
				int frame_count = 0;
				global_running = true;
				while (global_running) {

					Win32ProcessPendingMessages(&new_input);

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

					// for perspective projection matrix
					Win32WindowDimensions dims = Win32GetWindowDimension(Window);
					game_state->window_width = dims.width;
					game_state->window_height = dims.height;

					GameUpdateAndRender(&game_memory, &new_input);

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
					OutputDebugStringA(string_buffer);

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
			        glew_initialized
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
