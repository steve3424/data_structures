#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)
#define ArrayCount(array) (sizeof(array)/sizeof(array[0]))
/*
HANDMADE_INTERNAL
0 - public release
1 - build for developer

HANDMADE_SLOW
0 - NO SLOW CODE
1 - assertions and other slow code allowed
*/
#if HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int*)0 = 0;}
#else
#define Assert(Expression)
#endif

#if HANDMADE_INTERNAL
struct debug_read_file_result {
	uint32_t content_size;
	void* contents;
};

INTERNAL debug_read_file_result DEBUG_PlatformReadEntireFile(char* file_name);
INTERNAL void DEBUG_PlatformFreeFileMemory(void* file_memory);
INTERNAL bool DEBUG_PlatformWriteEntireFile(char* file_name, void* file_memory, uint32_t memory_size);
#elif
#endif

inline uint32_t SafeTruncateUInt64(uint64_t value) {
	Assert(value <= 0xFFFFFFFF);
	uint32_t value32 = (uint32_t)value;
	return value32;
}

struct GameSoundBuffer {
	uint16_t* samples;
	int sample_count;
	int samples_per_sec;
};

struct GameGraphicsBuffer {
	// pixels are 32 bits
	// memory order BB GG RR XX
	void* memory;
	int width;
	int height;
	int pitch;
};

struct GameButtonState {
	int transition_count;
	bool ended_down;
};

struct GameInput {
	// add game clocks
	GameButtonState move_up;
	GameButtonState move_down;
	GameButtonState move_left;
	GameButtonState move_right;

	GameButtonState action_up;
	GameButtonState action_down;
	GameButtonState action_left;
	GameButtonState action_right;

	GameButtonState left_shoulder;
	GameButtonState right_shoulder;
};

struct GameMemory {
	// memory must be cleared to 0 at initialization
	bool is_initialized;
	uint64_t permanent_storage_size;
	void* permanent_storage;
	uint64_t temp_storage_size;
	void* temp_storage;
};

struct GameState {
	int yOffset;
	int tone_hz;
};

INTERNAL void GameOutputSound(GameSoundBuffer* sound_buffer, int tone_hz) {
	LOCALPERSIST float tsine = 0;
	int wave_period = sound_buffer->samples_per_sec / tone_hz;
	int16_t volume = 3000;
	uint16_t* sample_to_write = sound_buffer->samples; 
	for (int sample_index = 0; 
	     sample_index < sound_buffer->sample_count; 
	     ++sample_index) {
		// write left and right channels
		float sine_val = sinf(tsine);
		uint16_t val = (uint16_t)(sine_val * volume);

		*sample_to_write++ = val;
		*sample_to_write++ = val;

		tsine += (2.0f*PI) / (float)wave_period;
	}
}

INTERNAL void RenderStripes(GameGraphicsBuffer* graphics_buffer, int global_stripe_yOffset) {
	int height = graphics_buffer->height;
	int width = graphics_buffer->width;
	int stripe_height = graphics_buffer->height / 10;
	int stripe_color = ((3 << 16) | (102 << 8) | 252);
	int current_color = stripe_color;

	// NOTE: -9 through 9 will have the same color value, so when scrolling up
	// 	 from the beginning there will be a large stripe here
	uint32_t* pixel = (uint32_t*)graphics_buffer->memory;
	for (int row=0; row < height; ++row) {
		if (((row + global_stripe_yOffset) / stripe_height) % 2 == 0) {
			if (row + global_stripe_yOffset > 0) {
				current_color = stripe_color;
			}
			else {
				current_color = 0;
			}
		}
		else {
			if (row + global_stripe_yOffset > 0) {
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

 INTERNAL void GameUpdateAndRender(GameMemory* memory, GameInput* input, GameGraphicsBuffer* graphics_buffer, GameSoundBuffer* sound_buffer) {

 	Assert(sizeof(GameState) <= memory->permanent_storage_size);

	GameState* game_state = (GameState*)memory->permanent_storage;

	if (!memory->is_initialized) {
		game_state->tone_hz = 256;
		memory->is_initialized = true;
	}

	if (input->move_up.ended_down) {
		game_state->yOffset += 1;
	}

	if (input->move_down.ended_down) {
		game_state->yOffset -= 1;
	}

	if(input->move_left.ended_down) {
		game_state->tone_hz -= 1;
	}

	if(input->move_right.ended_down) {
		game_state->tone_hz += 1;
	}

	if(input->left_shoulder.ended_down) {
		game_state->tone_hz -= 1;
	}

	if(input->right_shoulder.ended_down) {
		game_state->tone_hz += 1;
	}

	GameOutputSound(sound_buffer, game_state->tone_hz);
	RenderStripes(graphics_buffer, game_state->yOffset);
}
