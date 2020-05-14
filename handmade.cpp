#include "handmade.h"

static void GameOutputSound(GameSoundBuffer* sound_buffer) {
	static float tsine = 0;
	int tone_hz = 256;
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

static void RenderStripes(GameOffscreenBuffer* buffer, int global_stripe_yOffset) {
	int height = buffer->height;
	int width = buffer->width;
	int stripe_height = buffer->height / 10;
	int stripe_color = ((3 << 16) | (102 << 8) | 252);
	int current_color = stripe_color;

	// NOTE: -9 through 9 will have the same color value, so when scrolling up
	// 	 from the beginning there will be a large stripe here
	uint32_t* pixel = (uint32_t*)buffer->Memory;
	for (int row=0; row < height; ++row) {
		if (((row + global_stripe_yOffset) / stripe_height) % 2 == 0) {
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

static void GameUpdateAndRender(GameOffscreenBuffer* buffer, int global_stripe_yOffset, GameSoundBuffer* sound_buffer) {
	GameOutputSound(sound_buffer);
	RenderStripes(buffer, global_stripe_yOffset);
}
