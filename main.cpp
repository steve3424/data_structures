#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <dsound.h>
#include <math.h>

#define INTERNAL static
#define LOCALPERSIST static
#define GLOBAL static
#define PI 3.14159265359f


struct win32_offscreen_buffer {
	// pixels are 32 bits
	// memory order BB GG RR XX
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

struct win32_window_dimension {
	int Width;
	int Height;
};

struct win32_sound_output {
	int samples_per_sec;
	int bytes_per_sample;
	int buffer_size;
	int running_sample_num;
	int tone_hz;
	int period;
	int16_t volume; 
};

GLOBAL bool GlobalRunning = true;
GLOBAL win32_offscreen_buffer GlobalGraphicsBuffer;
GLOBAL LPDIRECTSOUNDBUFFER GlobalSoundBuffer;
GLOBAL int Global_stripe_yOffset = 0;

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

		if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &GlobalSoundBuffer, 0))) {
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

INTERNAL void Win32FillSoundBuffer(win32_sound_output* sound_output, DWORD byte_to_lock, DWORD bytes_to_write) {
	VOID* region_1 = NULL;
	VOID* region_2 = NULL;
	DWORD region_1_size = 0;
	DWORD region_2_size = 0;
	if (SUCCEEDED(GlobalSoundBuffer->Lock(byte_to_lock, bytes_to_write, &region_1, &region_1_size, &region_2, &region_2_size, 0))) {
		DWORD region_1_sample_count = region_1_size / sound_output->bytes_per_sample;
		uint16_t* sample_to_write = (uint16_t*)region_1;
		for (int sample_index = 0; 
		     sample_index < region_1_sample_count; 
		     ++sample_index) {
			// write left and right channels
			float x = (float)sound_output->running_sample_num;
			float scalar = (2.0f*PI) / (float)sound_output->period;
			float sin_val = sinf(x * scalar);
			int16_t val = (int16_t)(sin_val * sound_output->volume);

			*sample_to_write++ = val;
			*sample_to_write++ = val;

			sound_output->running_sample_num++;
		}
		
		DWORD region_2_sample_count = region_2_size / sound_output->bytes_per_sample;
		sample_to_write = (uint16_t*)region_2;
		for (int sample_index = 0; 
		     sample_index < region_2_sample_count;
		     ++sample_index) {
			// write left and right channels
			float x = (float)sound_output->running_sample_num;
			float scalar = (2.0f*PI) / (float)sound_output->period;
			float sin_val = sinf(x * scalar);
			int16_t val = (int16_t)(sin_val * sound_output->volume);

			*sample_to_write++ = val;
			*sample_to_write++ = val;

			sound_output->running_sample_num++;
		}
	
		if (!SUCCEEDED(GlobalSoundBuffer->Unlock(region_1, region_1_size, region_2, region_2_size))) {
			// Unlock() error
		}
	}
	else {
		// Lock() error
	}
}

INTERNAL win32_window_dimension Win32GetWindowDimension(HWND Window) {
	win32_window_dimension dims;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	dims.Width = ClientRect.right - ClientRect.left;
	dims.Height = ClientRect.bottom - ClientRect.top;

	return dims;
}

// write pixels as XX RR GG BB
INTERNAL void RenderStripes(win32_offscreen_buffer buffer) {
	int width = buffer.Width;
	int height = buffer.Height;
	int stripe_height = buffer.Height / 10;
	int stripe_color = ((3 << 16) | (102 << 8) | 252);
	int current_color = stripe_color;

	// NOTE: -9 through 9 will have the same color value, so when scrolling up
	// 	 from the beginning there will be a large stripe here
	uint32_t* pixel = (uint32_t*)buffer.Memory;
	for (int row=0; row < height; ++row) {
		if (((row + Global_stripe_yOffset) / stripe_height) % 2 == 0) {
			current_color = stripe_color;
		}
		else {
			current_color = 0;
		}

		for (int p=0; p < width; ++p) {
			*pixel = current_color; 
			++pixel;
		}
	}
}

INTERNAL void Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height) {
	if (buffer->Memory) {
		VirtualFree(buffer->Memory, 0, MEM_RELEASE);
	}

	buffer->Width = width;
	buffer->Height = height;
	
	// negative height tells windows to treat this buffer as top-down
	// instead of bottom-up
	buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
	buffer->Info.bmiHeader.biWidth = width;
	buffer->Info.bmiHeader.biHeight = -height;
	buffer->Info.bmiHeader.biPlanes = 1;
	buffer->Info.bmiHeader.biBitCount = 32;
	buffer->Info.bmiHeader.biCompression = BI_RGB;

	int bytes_per_pixel = 4;
	int bitmap_memory_size = width * height * bytes_per_pixel;
	buffer->Memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
	buffer->Pitch = width * bytes_per_pixel;
}

INTERNAL void Win32DisplayBufferInWindow(HDC device_context, 
					int window_width, int window_height,
					win32_offscreen_buffer* buffer) {
	StretchDIBits(device_context,
			0, 0, window_width, window_height,
			0, 0, buffer->Width, buffer->Height,
			buffer->Memory,
			&buffer->Info,
			DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK WindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	
	switch (message) {
		case WM_CLOSE:
		{
			GlobalRunning = false;
		} break;

		case WM_ACTIVATEAPP:
		{
		} break;

		case WM_DESTROY:
		{
			GlobalRunning = false;
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(window, &paint);
			win32_window_dimension dims = Win32GetWindowDimension(window);
			Win32DisplayBufferInWindow(device_context, 
						dims.Width, dims.Height,
						&GlobalGraphicsBuffer);
			EndPaint(window, &paint);
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32_t vk_code = wParam;
			bool is_down = (lParam & (1 << 31)) == 0;
			if (is_down) {
				switch (vk_code) {
					case VK_UP:
					{
						Global_stripe_yOffset -= 10;
					} break;

					case VK_DOWN:
					{
						Global_stripe_yOffset += 10;
					} break;

					case VK_LEFT:
					{
						Global_stripe_yOffset -= 10;
					} break;

					case VK_RIGHT:
					{
						Global_stripe_yOffset += 10;
					} break;
				}
			}
		} break;

		default:
		{
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return result;
}


int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	WNDCLASS WindowClass = {};
	WindowClass.lpfnWndProc = WindowCallback;
	WindowClass.hInstance = instance;
	WindowClass.lpszClassName = "HandmadeHeroWindow";

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
			Win32ResizeDIBSection(&GlobalGraphicsBuffer, 1280, 720);

			// init direct sound
			win32_sound_output sound_output = {};
			sound_output.samples_per_sec = 48000;
			sound_output.bytes_per_sample = 2 * sizeof(uint16_t);
			sound_output.buffer_size = sound_output.samples_per_sec * sound_output.bytes_per_sample;
			sound_output.running_sample_num = 0;
			sound_output.tone_hz = 261;
			sound_output.period = sound_output.samples_per_sec / sound_output.tone_hz;
			sound_output.volume = 2000;
			Win32InitDirectSound(Window, sound_output.samples_per_sec, sound_output.buffer_size);
			Win32FillSoundBuffer(&sound_output, 0, sound_output.buffer_size);
			GlobalSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);

			// ***** WINDOW LOOP *****
			while (GlobalRunning) {
				MSG message;
				while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
					if (message.message == WM_QUIT) {
						GlobalRunning = false;
					}

					TranslateMessage(&message);
					DispatchMessageA(&message);
				}

				// write graphics buffer
				RenderStripes(GlobalGraphicsBuffer);
				win32_window_dimension dims = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(device_context, dims.Width, dims.Height, &GlobalGraphicsBuffer);

				// write sound buffer
				DWORD write_cursor;
				DWORD play_cursor;
				if (SUCCEEDED(GlobalSoundBuffer->GetCurrentPosition(&write_cursor, &play_cursor))) {
					DWORD byte_to_lock = (sound_output.running_sample_num * sound_output.bytes_per_sample) % sound_output.buffer_size;
					DWORD bytes_to_write = 0;
					if (byte_to_lock < play_cursor) {
						bytes_to_write = play_cursor - byte_to_lock;
					}
					else if (play_cursor < byte_to_lock) {
						bytes_to_write = sound_output.buffer_size - byte_to_lock;
						bytes_to_write += play_cursor;
					}

					Win32FillSoundBuffer(&sound_output, byte_to_lock, bytes_to_write);

				}
				else {
					// GetCurrentPosition() error
				}
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
