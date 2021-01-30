#if !defined(ENGINE_H)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
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

struct GameButtonState {
	bool is_down;
	int repeat_count;
};

// buttons are for DVORAK layout right now
struct GameInput {
	int num_buttons = 11;
	union {
		GameButtonState buttons[11];
		struct {
			GameButtonState comma;
			GameButtonState a;
			GameButtonState o;
			GameButtonState e;
			GameButtonState s;
			GameButtonState p;
			GameButtonState w;
			GameButtonState arrow_up;
			GameButtonState arrow_down;
			GameButtonState arrow_left;
			GameButtonState arrow_right;
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

	int num_visualizations;
	int selected_visualization;
	void* data_structure[3];
	Node* selected_node;

};

INTERNAL void GameUpdateAndRender(GameMemory* memory, GameInput* input);

#define ENGINE_H
#endif
