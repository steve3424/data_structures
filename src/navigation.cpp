#include "engine.h"

INTERNAL void UpdateView(View* current_view, const GameInput* input) {
	if(input->w.is_down) {
		int temp_current_view = (int)(*current_view);
		int num_views = (int)NUM_VIEWS;
		++temp_current_view %= num_views;
		*current_view = (View)temp_current_view;
	}
	else if(input->v.is_down) {

		int temp_current_view = (int)(*current_view);
		int num_views = (int)NUM_VIEWS;
		if(0 < temp_current_view) {
			temp_current_view--;
		}
		else {
			temp_current_view = num_views - 1;
		}
		*current_view = (View)temp_current_view;
	}
}
