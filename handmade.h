#if !defined(HANDMADE_H)

struct GameSoundBuffer {
	uint16_t* samples;
	int sample_count;
	int samples_per_sec;
};

struct GameOffscreenBuffer {
	// pixels are 32 bits
	// memory order BB GG RR XX
	BITMAPINFO Info;
	void* Memory;
	int width;
	int height;
	int pitch;
};

void GameUpdateAndRender(GameOffscreenBuffer* buffer, int global_stripe_yOffset, GameSoundBuffer* sound_buffer);
void GameOutputSound(GameSoundBuffer* sound_buffer, int samples_to_write);
void RenderStripes(GameOffscreenBuffer* buffer, int global_stripe_yOffset);


#define HANDMADE_H
#endif
