#include <windows.h>
#include <stdint.h>

#define INTERNAL static
#define LOCALPERSIST static
#define GLOBAL static




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

GLOBAL bool GlobalRunning = true;
GLOBAL win32_offscreen_buffer GlobalBackBuffer;

win32_window_dimension Win32GetWindowDimension(HWND Window) {
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
	int current_color = 0;

	uint32_t* pixel = (uint32_t*)buffer.Memory;
	for (int row=0; row < height; ++row) {
		if (row % stripe_height == 0) {
			if (current_color == stripe_color) {
				current_color = 0;
			}
			else {
				current_color = stripe_color;
			}
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
					win32_offscreen_buffer buffer) {
	StretchDIBits(device_context,
			0, 0, window_width, window_height,
			0, 0, buffer.Width, buffer.Height,
			buffer.Memory,
			&buffer.Info,
			DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK WindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	
	switch (message) {
		case WM_SIZE:
		{
		} break;

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
						GlobalBackBuffer);
			EndPaint(window, &paint);
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
			Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

			while (GlobalRunning) {
				MSG message;
				while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
					if (message.message == WM_QUIT) {
						GlobalRunning = false;
					}

					TranslateMessage(&message);
					DispatchMessageA(&message);
				}

				RenderStripes(GlobalBackBuffer);

				win32_window_dimension dims = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(device_context, 
							dims.Width, dims.Height,
							GlobalBackBuffer);
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
