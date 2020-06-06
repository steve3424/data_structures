#define INTERNAL static
#define LOCALPERSIST static
#define GLOBAL static
#define PI 3.14159265359f

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "handmade.cpp"

#include <windows.h>
#include <dsound.h>


struct Win32GraphicsBuffer {
	// pixels are 32 bits
	// memory order BB GG RR XX
	BITMAPINFO info;
	void* memory;
	int width;
	int height;
	int pitch;
};

struct Win32WindowDimensions {
	int width;
	int height;
};

struct Win32SoundOutput {
	int samples_per_sec;
	int bytes_per_sample;
	int buffer_size;
	int running_sample_num;
	int latency_sample_count;
};

GLOBAL bool global_running;
GLOBAL Win32GraphicsBuffer global_graphics_buffer;
GLOBAL LPDIRECTSOUNDBUFFER global_sound_buffer;
GLOBAL int64_t global_counter_frequency;


INTERNAL debug_read_file_result DEBUG_PlatformReadEntireFile(char* file_name) {
	debug_read_file_result result = {};

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
					DEBUG_PlatformFreeFileMemory(result.contents);
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

INTERNAL void DEBUG_PlatformFreeFileMemory(void* file_memory) {
	if (file_memory) {
		VirtualFree(file_memory, 0, MEM_RELEASE);
	}
}

INTERNAL bool DEBUG_PlatformWriteEntireFile(char* file_name, void* file_memory, uint32_t memory_size) {
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

INTERNAL void Win32InitDirectSound(HWND window, int samples_per_sec, int buffer_size) {
	LPDIRECTSOUND direct_sound;
	if (SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0))) {
		WAVEFORMATEX wave_format = {};
		wave_format.wFormatTag = WAVE_FORMAT_PCM;
		wave_format.nChannels = 2;
		wave_format.nSamplesPerSec = samples_per_sec;
		wave_format.wBitsPerSample = 16;
		wave_format.nBlockAlign = (wave_format.nChannels*wave_format.wBitsPerSample) / 8;
		wave_format.nAvgBytesPerSec = wave_format.nBlockAlign * wave_format.nSamplesPerSec;
		wave_format.cbSize = 0;

		if (SUCCEEDED(direct_sound->SetCooperativeLevel(window, DSSCL_PRIORITY))) {

			// create primary buffer in order to set WaveFormat
			DSBUFFERDESC buffer_description = {};
			buffer_description.dwSize = sizeof(buffer_description);
			buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

			LPDIRECTSOUNDBUFFER primary_buffer;
			if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description,
								      &primary_buffer,
								      0))) {
				if (SUCCEEDED(primary_buffer->SetFormat(&wave_format))) {
					OutputDebugStringA("Primary sound buffer created.\n");
				}
				else {
					// SetFormat() error
				}
			}
			else {
				// CreateSoundBuffer() primary error
			}
		}
		else {
			// SetCooperativeLevel() error
		}
		
		// create secondary buffer
		DSBUFFERDESC buffer_description = {};
		buffer_description.dwSize = sizeof(buffer_description);
		buffer_description.dwFlags = 0;
		buffer_description.dwBufferBytes = buffer_size;
		buffer_description.lpwfxFormat = &wave_format;

		if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &global_sound_buffer, 0))) {
			OutputDebugStringA("Secondary sound buffer created.\n");
		}
		else {
			// CreateSoundBuffer() secondary error 
		}
	}
	else {
		// DirectSoundCreate() error
	}
}


INTERNAL void Win32ClearSoundBuffer(Win32SoundOutput* sound_output) {
	VOID* region_1 = NULL;
	VOID* region_2 = NULL;
	DWORD region_1_size = 0;
	DWORD region_2_size = 0;
	if (SUCCEEDED(global_sound_buffer->Lock(0, sound_output->buffer_size, &region_1, &region_1_size, &region_2, &region_2_size, 0))) {
		uint8_t* dest_sample = (uint8_t*)region_1;
		for (DWORD byte_index = 0; 
		     byte_index < region_1_size; 
		     ++byte_index) {

		     *dest_sample++ = 0;
		}

		dest_sample = (uint8_t*)region_2;
		for (DWORD byte_index = 0; 
		     byte_index < region_2_size;
		     ++byte_index) {

		     *dest_sample++ = 0;
		}

		if (!SUCCEEDED(global_sound_buffer->Unlock(region_1, region_1_size, region_2, region_2_size))) {
			// Unlock() error
		}
	}
	else {
		// Lock() failure
	}
}

INTERNAL void Win32FillSoundBuffer(Win32SoundOutput* sound_output, DWORD byte_to_lock, DWORD bytes_to_write, GameSoundBuffer* sound_buffer) {
	VOID* region_1 = NULL;
	VOID* region_2 = NULL;
	DWORD region_1_size = 0;
	DWORD region_2_size = 0;
	if (SUCCEEDED(global_sound_buffer->Lock(byte_to_lock, bytes_to_write, &region_1, &region_1_size, &region_2, &region_2_size, 0))) {
		DWORD region_1_sample_count = region_1_size / sound_output->bytes_per_sample;
		uint16_t* sample_to_write = (uint16_t*)region_1;
		uint16_t* sample_val = sound_buffer->samples;
		for (DWORD sample_index = 0; 
		     sample_index < region_1_sample_count; 
		     ++sample_index) {
			// write left and right channels
			*sample_to_write++ = *sample_val++;
			*sample_to_write++ = *sample_val++;

			sound_output->running_sample_num++;
		}
		
		DWORD region_2_sample_count = region_2_size / sound_output->bytes_per_sample;
		sample_to_write = (uint16_t*)region_2;
		for (DWORD sample_index = 0; 
		     sample_index < region_2_sample_count;
		     ++sample_index) {
			// write left and right channels
			*sample_to_write++ = *sample_val++;
			*sample_to_write++ = *sample_val++;

			sound_output->running_sample_num++;
		}
	
		if (!SUCCEEDED(global_sound_buffer->Unlock(region_1, region_1_size, region_2, region_2_size))) {
			// Unlock() error
		}
	}
	else {
		// Lock() error
	}
}

INTERNAL Win32WindowDimensions Win32GetWindowDimension(HWND Window) {
	Win32WindowDimensions dims;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	dims.width = ClientRect.right - ClientRect.left;
	dims.height = ClientRect.bottom - ClientRect.top;

	return dims;
}

INTERNAL void Win32ResizeDIBSection(Win32GraphicsBuffer *buffer, int width, int height) {
	if (buffer->memory) {
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;
	
	// negative height tells windows to treat this buffer as top-down
	// instead of bottom-up
	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = width;
	buffer->info.bmiHeader.biHeight = -height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

	int bytes_per_pixel = 4;
	int bitmap_memory_size = width * height * bytes_per_pixel;
	buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	buffer->pitch = width * bytes_per_pixel;
}

INTERNAL void Win32DisplayBufferInWindow(HDC device_context, 
					int window_width, int window_height,
					Win32GraphicsBuffer* buffer) {
	StretchDIBits(device_context,
			0, 0, window_width, window_height,
			0, 0, buffer->width, buffer->height,
			buffer->memory,
			&buffer->info,
			DIB_RGB_COLORS, SRCCOPY);
}

INTERNAL void Win32ProcessKeyboardMessage(GameButtonState* new_button_state, bool is_down) {
	Assert(new_button_state->ended_down != is_down);
	new_button_state->ended_down = is_down;
	++new_button_state->transition_count;
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
				if (was_down != is_down) {
					switch (vk_code) {
						// CASES ARE FOR DVORAK LAYOUT

						case VK_OEM_COMMA:
						{
							Win32ProcessKeyboardMessage(&new_input->move_up, is_down);
						} break;

						case 'A':
						{
							Win32ProcessKeyboardMessage(&new_input->move_left, is_down);
						} break;

						case 'O':
						{
							Win32ProcessKeyboardMessage(&new_input->move_down, is_down);
						} break;

						case 'E':
						{
							Win32ProcessKeyboardMessage(&new_input->move_right, is_down);
						} break;

						case VK_OEM_7:
						{
							Win32ProcessKeyboardMessage(&new_input->left_shoulder, is_down);
						} break;

						case VK_OEM_PERIOD:
						{
							Win32ProcessKeyboardMessage(&new_input->right_shoulder, is_down);
						} break;

						case VK_UP:
						{
							Win32ProcessKeyboardMessage(&new_input->action_up, is_down);
						} break;

						case VK_DOWN:
						{
							Win32ProcessKeyboardMessage(&new_input->action_down, is_down);
						} break;

						case VK_LEFT:
						{
							Win32ProcessKeyboardMessage(&new_input->action_left, is_down);
						} break;

						case VK_RIGHT:
						{
							Win32ProcessKeyboardMessage(&new_input->action_right, is_down);
						} break;

						case VK_ESCAPE:
						{
							global_running = false;
						} break;

						case VK_SPACE:
						{
						} break;
					}

					bool alt_key_was_down = (message.lParam & (1 << 29));
					if((vk_code == VK_F4) && alt_key_was_down) {
						global_running = false;
					}
				}
			} break;

			default:
			{
				TranslateMessage(&message);
				DispatchMessageA(&message);
			} break;
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

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(window, &paint);
			Win32WindowDimensions dims = Win32GetWindowDimension(window);
			Win32DisplayBufferInWindow(device_context, 
						dims.width, dims.height,
						&global_graphics_buffer);
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

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	LARGE_INTEGER perf_frequency;
	QueryPerformanceFrequency(&perf_frequency);
	global_counter_frequency = perf_frequency.QuadPart;

	UINT scheduler_granularity = 1; // milliseconds
	bool sleep_is_granular = timeBeginPeriod(scheduler_granularity) == TIMERR_NOERROR;

	WNDCLASS WindowClass = {};
	WindowClass.lpfnWndProc = WindowCallback;
	WindowClass.hInstance = instance;
	WindowClass.lpszClassName = "HandmadeHeroWindow";

	int monitor_refresh_hz = 60;
	int game_update_hz = monitor_refresh_hz / 2;
	float target_seconds_per_frame = 1.0f / (float)game_update_hz;

	if (RegisterClassA(&WindowClass)) {
		HWND Window = CreateWindowExA(0, 
						WindowClass.lpszClassName, 
						"Handmade Hero",
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
			Win32ResizeDIBSection(&global_graphics_buffer, 1280, 720);

			// init direct sound
			Win32SoundOutput sound_output = {};
			sound_output.samples_per_sec = 48000;
			sound_output.bytes_per_sample = 2 * sizeof(int16_t);
			sound_output.buffer_size = sound_output.samples_per_sec * sound_output.bytes_per_sample;
			sound_output.running_sample_num = 0;
			sound_output.latency_sample_count = sound_output.samples_per_sec / 15;
			Win32InitDirectSound(Window, sound_output.samples_per_sec, sound_output.buffer_size);
			Win32ClearSoundBuffer(&sound_output);
			global_sound_buffer->Play(0, 0, DSBPLAY_LOOPING);

			uint16_t* game_samples = (uint16_t*)VirtualAlloc(0, sound_output.buffer_size, MEM_COMMIT, PAGE_READWRITE);

			GameMemory game_memory = {};
			game_memory.permanent_storage_size = Megabytes(64); 
			game_memory.temp_storage_size = Gigabytes(1);
			uint64_t total_size = game_memory.permanent_storage_size + game_memory.temp_storage_size;
			game_memory.permanent_storage = VirtualAlloc(0, (size_t)total_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			game_memory.temp_storage = (uint8_t*)game_memory.permanent_storage + game_memory.permanent_storage_size;

			if (game_samples && game_memory.permanent_storage && game_memory.temp_storage) {
				GameInput input[2] = {};
				GameInput* new_input = &input[0];
				GameInput* old_input = &input[1];

				// ***** WINDOW LOOP *****
				global_running = true;
				LARGE_INTEGER last_counter = Win32GetWallClock();
				uint64_t last_cycle_counter = __rdtsc();
				while (global_running) {
					
					*new_input = {};
					new_input->action_up.ended_down = old_input->action_up.ended_down;
					new_input->action_down.ended_down = old_input->action_down.ended_down;
					new_input->action_left.ended_down = old_input->action_left.ended_down;
					new_input->action_right.ended_down = old_input->action_right.ended_down;
					new_input->move_up.ended_down = old_input->move_up.ended_down;
					new_input->move_down.ended_down = old_input->move_down.ended_down;
					new_input->move_left.ended_down = old_input->move_left.ended_down;
					new_input->move_right.ended_down = old_input->move_right.ended_down;
					new_input->left_shoulder.ended_down = old_input->left_shoulder.ended_down;
					new_input->right_shoulder.ended_down = old_input->right_shoulder.ended_down;

					Win32ProcessPendingMessages(new_input);

					DWORD byte_to_lock = 0;
					DWORD target_cursor = 0;
					DWORD bytes_to_write = 0;
					DWORD play_cursor = 0;
					DWORD write_cursor = 0;
					bool sound_is_valid = false;
					if (SUCCEEDED(global_sound_buffer->GetCurrentPosition(&play_cursor, &write_cursor))) {
						byte_to_lock = (sound_output.running_sample_num * sound_output.bytes_per_sample) % sound_output.buffer_size;

						target_cursor = ((play_cursor + (sound_output.latency_sample_count*sound_output.bytes_per_sample)) % sound_output.buffer_size);

						if (byte_to_lock > target_cursor) {
							bytes_to_write = sound_output.buffer_size - byte_to_lock + target_cursor;
						}
						else {
							bytes_to_write = target_cursor - byte_to_lock;
						}

						sound_is_valid = true;
					}
					else {
						// GetCurrentPosition() error
					}

					GameSoundBuffer sound_buffer = {};
					sound_buffer.samples_per_sec = sound_output.samples_per_sec;
					sound_buffer.sample_count = bytes_to_write / sound_output.bytes_per_sample; 
					sound_buffer.samples = game_samples;

					GameGraphicsBuffer graphics_buffer = {};
					graphics_buffer.memory = global_graphics_buffer.memory;
					graphics_buffer.width = global_graphics_buffer.width;
					graphics_buffer.height = global_graphics_buffer.height;
					graphics_buffer.pitch = global_graphics_buffer.pitch;


					GameUpdateAndRender(&game_memory, new_input, &graphics_buffer, &sound_buffer);
					
					if (sound_is_valid) {
						Win32FillSoundBuffer(&sound_output, byte_to_lock, bytes_to_write, &sound_buffer);
					}


					LARGE_INTEGER work_counter = Win32GetWallClock();
					float seconds_elapsed_for_work = Win32GetSecondsElapsed(last_counter, work_counter);
					float seconds_elapsed_for_frame = seconds_elapsed_for_work;
					if (seconds_elapsed_for_frame < target_seconds_per_frame) {
						while (seconds_elapsed_for_frame < target_seconds_per_frame) {
							if (sleep_is_granular) {
								DWORD ms_to_sleep = (DWORD)(1000.0f * (target_seconds_per_frame - seconds_elapsed_for_frame));
								Sleep(ms_to_sleep);
							}
							seconds_elapsed_for_frame = Win32GetSecondsElapsed(last_counter, Win32GetWallClock());
						}

					}
					else {
						// missed frame rate
					}

					Win32WindowDimensions dims = Win32GetWindowDimension(Window);
					Win32DisplayBufferInWindow(device_context, dims.width, dims.height, &global_graphics_buffer);

					/*
					double ms_per_frame = ((1000.0*(double)counter_elapsed) / (double)global_counter_frequency);
					double frames_per_sec = (double)1000.0 / ms_per_frame;
					double megacycles_per_frame = (double)(cycles_elapsed / (1000*1000));

					char buffer[256];
					wsprintf(buffer, "%d ms/f, %d f/s, %d Mc/f\n", ms_per_frame, frames_per_sec, megacycles_per_frame);
					OutputDebugStringA(buffer);
					*/

					GameInput* temp = new_input;
					new_input = old_input;
					old_input = temp;

					LARGE_INTEGER end_counter = Win32GetWallClock();
					last_counter = end_counter;

					uint64_t end_cycle_counter = __rdtsc();
					uint64_t cycles_elapsed = end_cycle_counter - last_cycle_counter;
					last_cycle_counter = end_cycle_counter;
				}
			}
			else {
				// could not get either sound buffer or game memory
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
