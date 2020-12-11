#if !defined(ENGINE_H)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "platform_api.h"
#include "..\\include\\binary_tree.h"
#define GLEW_STATIC
#include "..\include\glew.h"
#include "..\\include\\glm\\glm.hpp"
#include "..\\include\\glm\\gtc\\matrix_transform.hpp"
#include "..\\include\\glm\\gtc\\type_ptr.hpp"

#define INTERNAL static
#define LOCALPERSIST static
#define GLOBAL static
#define PI 3.14159265359f

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)
#define ArrayCount(array) (sizeof(array)/sizeof(array[0]))

#if DEBUG
#define Assert(Expression) if(!(Expression)) {*(int*)0 = 0;}
#define GLCall(function) GLClearErrors(); function; GLCheckErrors()
#else
#define Assert(Expression)
#define GLCall(function) function
#endif


inline uint32_t SafeTruncateUInt64(uint64_t value) {
	Assert(value <= 0xFFFFFFFF);
	uint32_t value32 = (uint32_t)value;
	return value32;
}

struct GameButtonState {
	bool is_down;
	int repeat_count;
};

struct GameInput {
	int num_buttons = 10;
	union {
		GameButtonState buttons[10];
		struct {
			GameButtonState w_up;
			GameButtonState s_down;
			GameButtonState a_left;
			GameButtonState d_right;

			GameButtonState arrow_up;
			GameButtonState arrow_down;
			GameButtonState arrow_left;
			GameButtonState arrow_right;

			GameButtonState successor;
			GameButtonState predecessor;
		};
	};
};

struct GameMemory {
	uint64_t storage_size;
	void* storage;
};

struct GameObject {
	unsigned int vao;
	int num_indices;
};

struct GameState {
	bool initialized;
	GameObject digits[100];
	GameObject node;
	GameObject arrow;
	GameObject line;
	unsigned int shader;
	int window_width;
	int window_height;
	float camera_x;
	float camera_y;
	float camera_z;
	void* data_structure;
	Node* selected_node;
};

#define GAME_LOAD_PLATFORM_API(name) bool name(PlatformAPI platform_api)
typedef GAME_LOAD_PLATFORM_API(game_load_platform_api);
GAME_LOAD_PLATFORM_API(GameLoadPlatformAPIStub) {
	return false;
}

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory* memory, GameInput* input)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub) {

}

#define GAME_GLEW_INIT(name) bool name()
typedef GAME_GLEW_INIT(game_glew_init);
GAME_GLEW_INIT(GameGlewInitStub) {
	return false;
}

#define ENGINE_H
#endif
