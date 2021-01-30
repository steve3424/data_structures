#include "engine.h"
#include "stdlib.h"
#include "queue.h"
#include "binary_tree.h"
#include "array.h"
#include "time.h"

#define MAX_DIGITS 100

INTERNAL void UpdateArrayState(const GameInput* input, ArrayStruct* a) {
	const double unit_cubes_per_second = 1.0f;
	const double frames_per_second = 60.0f;
	const double velocity = unit_cubes_per_second / frames_per_second;
	const double y_movement = 1.2f;

	if(a->state == STATIC) {
		if(input->s.is_down) {
			a->selected_value_index = 1;
			a->compare_value_index = 0;
			a->selected_value = a->array[a->selected_value_index];

			a->state = LIFT_SELECTED_VALUE;
		}
	}
	else if(a->state == LIFT_SELECTED_VALUE) {
		if(a->selected_value_y_shift < y_movement) {
			a->selected_value_y_shift += velocity;
		}
		else {
			a->state = SORTING;
		}
	}
	else if(a->state == SORTING) {
		int i = a->selected_value_index;
		int j = a->compare_value_index;
		if(i < a->size) {
			if((0 <= j) && (a->selected_value < a->array[j])) {
				a->state = COPY_COMPARE_VALUE;
			}
			else {
				a->state = SLIDE_SELECTED_VALUE;
			}
		}
		else {
			a->selected_value_index = -1;
			a->compare_value_index = -1;
			a->selected_value = 0;
			a->selected_value_x_shift = 0.0f;
			a->selected_value_y_shift = 0.0f;
			a->compare_value_x_shift = 0.0f;

			a->state = STATIC;
		}
	}
	else if(a->state == COPY_COMPARE_VALUE) {
		// size of model defined in opengl code
		const double node_width = 1.0f; 
		if(a->compare_value_x_shift < node_width) {
			a->compare_value_x_shift += velocity;
		}
		else {
			// drawing done, change actual values now
			int j = a->compare_value_index;
			a->array[j + 1] = a->array[j];
			a->compare_value_index -= 1;
			a->compare_value_x_shift = 0.0f;

			a->state = SORTING;
		}
	}
	else if(a->state == SLIDE_SELECTED_VALUE) {
		const int i = a->selected_value_index;
		const int j = a->compare_value_index + 1;
		const double final_shift_position = (double)(j - i);
		if(final_shift_position < a->selected_value_x_shift) {
			a->selected_value_x_shift -= velocity;
		}
		else {
			a->state = DROP_SELECTED_VALUE;
		}
	}
	else if(a->state == DROP_SELECTED_VALUE) {
		if(0.0f < a->selected_value_y_shift) {
			a->selected_value_y_shift -= velocity;
		}
		else {
			// drawing done, change values
			int j = a->compare_value_index;
			int val = a->selected_value;
			a->array[j + 1] = val;

			a->selected_value_index += 1;
			a->compare_value_index = a->selected_value_index - 1;
			a->selected_value = a->array[a->selected_value_index];
			a->selected_value_x_shift = 0.0f;
			a->selected_value_y_shift = 0.0f;
			a->compare_value_x_shift = 0.0f;

			a->state = LIFT_SELECTED_VALUE;
		}
	}
}

INTERNAL void DrawArray(const GameState* game_state, const ArrayStruct* a) {
	int color_location = glGetUniformLocation(game_state->shader, "color");
	int model_location = glGetUniformLocation(game_state->shader, "model");
	int view_location = glGetUniformLocation(game_state->shader, "view");
	int projection_location = glGetUniformLocation(game_state->shader, "projection");

	glm::mat4 identity = glm::mat4(1.0f);
	glm::mat4 node_model = identity;
	glm::mat4 digit_model = identity;
	glm::mat4 line_model = identity;
	glm::mat4 view = identity;
	view = glm::translate(view, glm::vec3(game_state->camera_x, 
						game_state->camera_y, 
						game_state->camera_z));
	float window_ratio = (float)game_state->window_width / (float)game_state->window_height;
	glm::mat4 projection = glm::perspective(glm::radians(75.0f), 
						window_ratio, 
						0.1f, 100.0f);

	glUniformMatrix4fv(view_location, 1, 
				GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projection_location, 1, 
				GL_FALSE, glm::value_ptr(projection));

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLineWidth(4.0f);
	glUseProgram(game_state->shader);

	const double digit_scale = 0.65f;
	const double digit_z_shift = -0.5f;
	// lets the copied digit move in front of the original digit it overwrites
	const double copied_digit_z_shift = -0.49f;
	// y_shift is 1.2f so I want to start the alpha at 1.0f
	const double alpha = (a->state == DROP_SELECTED_VALUE) ? (a->selected_value_y_shift - 0.2f) : (1.0f - a->compare_value_x_shift);
	const double selected_value_color_R = 0.0f;
	const double selected_value_color_G = 1.0f;
	const double selected_value_color_B = 0.0f;
	const double compare_value_color_R = 1.0f * 0.678f;
	const double compare_value_color_G = 1.0f * 0.847f;
	const double compare_value_color_B = 1.0f * 0.902f;
	// DRAW ALL NODES
	for(int i = 0; i < a->size; ++i) {
		// select color, then draw node
		if(i == a->selected_value_index) {
			glUniform4f(color_location, 
				    selected_value_color_R, 
				    selected_value_color_G,
				    selected_value_color_B,
				    1.0f);
		}
		else if(i == a->compare_value_index) {
			glUniform4f(color_location, 
				    compare_value_color_R, 
				    compare_value_color_G,
				    compare_value_color_B,
				    1.0f);
		}
		else {
			glUniform4f(color_location, 0.0f, 0.0f, 1.0f, 1.0f);
		}

		node_model = glm::translate(identity, glm::vec3(i, 0.0f, 0.0f));
		glUniformMatrix4fv(model_location, 1, GL_FALSE, 
				   glm::value_ptr(node_model));
		glBindVertexArray(game_state->node.vao);
		glDrawElements(GL_LINES, game_state->node.num_indices, 
			       GL_UNSIGNED_INT, (void*)0);
	}

	// DRAW ALL DIGITS
	for(int i = 0; i < a->size; ++i) {
		if(i == (a->compare_value_index + 1)) {
			glUniform4f(color_location, 0.0f, 1.0f, 0.0f, (float)alpha);
		}
		else {
			glUniform4f(color_location, 0.0f, 1.0f, 0.0f, 1.0f);
		}

		digit_model = glm::scale(identity, 
					 glm::vec3(digit_scale, digit_scale, digit_scale));
		digit_model = glm::translate(digit_model, 
					     glm::vec3(i / digit_scale, 
					     	       0.0f / digit_scale, 
						       digit_z_shift / digit_scale));
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(digit_model));
		glBindVertexArray(game_state->digits[a->array[i]].vao);
		glDrawElements(GL_LINES, game_state->digits[a->array[i]].num_indices, GL_UNSIGNED_INT, (void*)0);
	}

	// DRAW SELECTED DIGIT COPY
	if(0 <= a->selected_value_index && a->selected_value_index < a->size) {
		glUniform4f(color_location, 0.0f, 1.0f, 0.0f, 1.0f);
		digit_model = glm::scale(identity, 
					 glm::vec3(digit_scale, digit_scale, digit_scale));
		digit_model = glm::translate(digit_model, 
					     glm::vec3((a->selected_value_x_shift + a->selected_value_index) / digit_scale, 
					     a->selected_value_y_shift / digit_scale,
					     copied_digit_z_shift / digit_scale));
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(digit_model));
		glBindVertexArray(game_state->digits[a->selected_value].vao);
		glDrawElements(GL_LINES, game_state->digits[a->selected_value].num_indices, 
			       GL_UNSIGNED_INT, (void*)0);
	}

	// DRAW COMPARE DIGIT COPY
	if(0 <= a->compare_value_index && a->compare_value_index < a->size) {
		glUniform4f(color_location, 0.0f, 1.0f, 0.0f, 1.0f);
		digit_model = glm::scale(identity, 
					 glm::vec3(digit_scale, digit_scale, digit_scale));
		digit_model = glm::translate(digit_model, 
					     glm::vec3((a->compare_value_x_shift + a->compare_value_index) / digit_scale, 
					     0.0f,
					     copied_digit_z_shift / digit_scale));
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(digit_model));
		glBindVertexArray(game_state->digits[a->array[a->compare_value_index]].vao);
		glDrawElements(GL_LINES, game_state->digits[a->array[a->compare_value_index]].num_indices, 
			       GL_UNSIGNED_INT, (void*)0);
	}

}

INTERNAL inline void ProcessBSTInput(GameState* game_state, const GameInput* input, BinaryTree* bst) {
	if(input->s.is_down) {
		Node* s = Successor(bst, game_state->selected_node);
		if(s) {
			game_state->selected_node = s;
		}
	}

	if(input->p.is_down) {
		Node* p = Predecessor(bst, game_state->selected_node);
		if(p) {
			game_state->selected_node = p;
		}
	}

	if(input->e.is_down) {
		Delete(bst, game_state->selected_node, true);
		game_state->selected_node = bst->head;
	}

	if(input->a.is_down) {
		if(bst->size < MAX_DIGITS) {
			int num = rand() % MAX_DIGITS;
			Node* s = Search(bst, num);
			if(!s) {
				Node* n = AllocateNode(num, num);
				Insert(bst, n, true);
			}

			if(bst->size == 1) {
				game_state->selected_node = bst->head;
			}

		}
	}
}

INTERNAL void DrawBST(const GameState* game_state, const GameInput* input, const BinaryTree* b) {
	int color_location = glGetUniformLocation(game_state->shader, "color");
	int model_location = glGetUniformLocation(game_state->shader, "model");
	int view_location = glGetUniformLocation(game_state->shader, "view");
	int projection_location = glGetUniformLocation(game_state->shader, "projection");

	glm::mat4 identity = glm::mat4(1.0f);
	glm::mat4 node_model = identity;
	float digit_scale = 0.65f;
	glm::mat4 digit_model = identity;
	glm::mat4 line_model = identity;
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(game_state->camera_x, game_state->camera_y, game_state->camera_z));
	glm::mat4 projection = glm::perspective(glm::radians(75.0f), (float)game_state->window_width / (float)game_state->window_height, 0.1f, 100.0f);

	glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));


	// MANAGE NODES THAT NEED TO BE DRAWN
	Node* nodes[MAX_DIGITS] = {NULL};
	Node* children[MAX_DIGITS] = {NULL};
	nodes[0] = b->head;

	// MANAGE THE SPACING AND DRAWING OF NODES
	// calculate the power of 2 necessary to represent the number of nodes in the tree
	// start num_levels at 1 instead of 0 because we want the number of levels starting from 0, not the power
	int num_levels = 1;
	float size = (float)b->size;
	while(size >= 2.0f) {
		num_levels += 1;
		size /= 2.0f;
	}

	// calculate the maximum number of nodes at the bottom level of the tree
	// this is necessary to determine the spacing of all of the nodes in the tree
	// here we use the power of 2 calculated above which is num_levels - 1
	int nodes_at_bottom_level = 1;
	for(int i=0; i < (num_levels - 1); ++i) {
		nodes_at_bottom_level *= 2;
	}

	const float node_width = 1.0f; // this is fixed based on the model sent to the GPU
	float x_spacing = 1.0f; // space between the nodes at the bottom level
	float y_spacing = 2.0f; // space between successive levels of the tree
	float max_width = nodes_at_bottom_level * (node_width + x_spacing) - x_spacing; // subtract one x_spacing for the far right node
	float half_width = max_width / 2.0f; // used to center the tree at the origin along the x-axis

	// start by splitting the max width of the tree in half
	// at each level, split in half again
	// the nodes at each level will be drawn at every other split
	float split = 2.0f;
	float section_width = (max_width / split);
	float current_x_location = section_width - half_width;

	// calculate values for the lines that connects the nodes
	float line_scale = 0.0f;
	float angle_in_radians = 0.0f;
	float angle_flip = -1.0f;
	float y_height = y_spacing - node_width; // the line should start at the bottom of parent node and go to the top of child node, not from center to center


	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLineWidth(4.0f);
	glUseProgram(game_state->shader);

	int node_index = 0;
	for(int level = 0; level < num_levels; ++level) {
		for(int node_location = 0; node_location < split; node_location += 2) {
			if(nodes[node_index] != NULL) {
				if(nodes[node_index] == game_state->selected_node) {
					glUniform4f(color_location, 1.0f, 0.0f, 0.0f, 1.0f);
				}
				else {
					glUniform4f(color_location, 0.0f, 0.0f, 1.0f, 1.0f);
				}

				node_model = glm::translate(identity, glm::vec3(current_x_location, -y_spacing * level, 0.0f));
				glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(node_model));
				glBindVertexArray(game_state->node.vao);
				glDrawElements(GL_LINES, game_state->node.num_indices, GL_UNSIGNED_INT, (void*)0);

				digit_model = glm::scale(identity, glm::vec3(digit_scale, digit_scale, digit_scale));
				digit_model = glm::translate(digit_model, glm::vec3(current_x_location / digit_scale, -y_spacing * level / digit_scale, -0.5f / digit_scale));
				glUniform4f(color_location, 0.0f, 1.0f, 0.0f, 1.0f);
				glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(digit_model));
				glBindVertexArray(game_state->digits[nodes[node_index]->val].vao);
				glDrawElements(GL_LINES, game_state->digits[nodes[node_index]->val].num_indices, GL_UNSIGNED_INT, (void*)0);

				line_model = glm::scale(identity, glm::vec3(line_scale, line_scale, line_scale));
				line_model = glm::translate(line_model, glm::vec3(current_x_location/line_scale, (-y_spacing * level)/line_scale + (node_width / 2.0f) / line_scale, -0.5f/line_scale));
				line_model = glm::rotate(line_model, angle_in_radians * angle_flip, glm::vec3(0.0f, 0.0f, 1.0f));
				glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(line_model));
				glBindVertexArray(game_state->line.vao);
				glDrawElements(GL_LINES, game_state->line.num_indices, GL_UNSIGNED_INT, (void*)0);

			}

			current_x_location += section_width*2.0f;
			angle_flip *= -1.0f;
			++node_index;
		}
		
		// UPDATE VARIABLES FOR NEXT LOOP
		split *= 2.0f;
		section_width = (max_width / split);
		current_x_location = section_width - half_width;
		line_scale = sqrt((y_height * y_height) + (section_width * section_width));
		angle_in_radians = asin(y_height / line_scale) + PI / 2.0f;
		node_index = 0;
		
		// enumerate all child nodes then copy child nodes into node array
		int j = 0;
		for(unsigned int i = 0; i < b->size; ++i) {
			if(nodes[i]) {
				children[j] = nodes[i]->left;
				++j;
				children[j] = nodes[i]->right;
				++j;
			}
		}

		for(unsigned int i = 0; i < b->size; ++i) {
			nodes[i] = children[i];
		}
	}
}

INTERNAL inline void ProcessQueueInput(const GameInput* input, QueueInt* q) {
	if(input->a.is_down) {
		int num = rand() % MAX_DIGITS;
		EnqueueInt(q, num);
	}
	else if(input->e.is_down) {
		int error = 0;
		DequeueInt(q, &error);
	}
}

INTERNAL void DrawQueue(GameState* game_state, GameInput* input, QueueInt* q) {
	int color_location = glGetUniformLocation(game_state->shader, "color");
	int model_location = glGetUniformLocation(game_state->shader, "model");
	int view_location = glGetUniformLocation(game_state->shader, "view");
	int projection_location = glGetUniformLocation(game_state->shader, "projection");

	glm::mat4 node_model = glm::mat4(1.0f);
	float digit_scale = 0.5f;
	glm::mat4 digit_model = glm::mat4(1.0f);
	digit_model = glm::scale(digit_model, glm::vec3(digit_scale, digit_scale, digit_scale));
	digit_model = glm::translate(digit_model, glm::vec3(0.0f, 0.0f, -0.5f));
	glm::mat4 arrow_model = glm::mat4(1.0f);
	arrow_model = glm::translate(arrow_model, glm::vec3(0.0f, 0.7f, 0.0f));
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(game_state->camera_x, game_state->camera_y, game_state->camera_z));
	glm::mat4 projection = glm::perspective(glm::radians(75.0f), (float)game_state->window_width / (float)game_state->window_height, 0.1f, 100.0f);

	glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));


	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLineWidth(4.0f);
	glUseProgram(game_state->shader);

	for(int i = 0; i < q->capacity; ++i) {
		// draw node
		glUniform4f(color_location, 0.0f, 0.0f, 1.0f, 1.0f);
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(node_model));
		glBindVertexArray(game_state->node.vao);
		glDrawElements(GL_LINES, game_state->node.num_indices, GL_UNSIGNED_INT, (void*)0);
		node_model = glm::translate(node_model, glm::vec3(1.0f, 0.0f, 0.0f));

		// draw digit
		if(IndexIsInQueue(q, i)) {
			glUniform4f(color_location, 0.0f, 1.0f, 0.0f, 1.0f);
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(digit_model));
			glBindVertexArray(game_state->digits[q->elements[i]].vao);
			glDrawElements(GL_LINES, game_state->digits[q->elements[i]].num_indices, GL_UNSIGNED_INT, (void*)0);
		}
		digit_model = glm::translate(digit_model, glm::vec3(1.0f / digit_scale, 0.0f, 0.0f));

		// draw arrow
		if(i == q->front) {
			glUniform4f(color_location, 0.0f, 1.0f, 0.0f, 1.0f);
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(arrow_model));
			glBindVertexArray(game_state->arrow.vao);
			glDrawElements(GL_LINES, game_state->arrow.num_indices, GL_UNSIGNED_INT, (void*)0);
		}
		else if(i == q->back) {
			glUniform4f(color_location, 1.0f, 0.0f, 0.0f, 1.0f);
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(arrow_model));
			glBindVertexArray(game_state->arrow.vao);
			glDrawElements(GL_LINES, game_state->arrow.num_indices, GL_UNSIGNED_INT, (void*)0);
		}
		arrow_model = glm::translate(arrow_model, glm::vec3(1.0f, 0.0f, 0.0f));
	}
}

INTERNAL inline void UpdateCamera(GameState* game_state, GameInput* input) {
	if(input->arrow_right.is_down) {
		game_state->camera_x -= 0.05f;
	}
	if(input->arrow_left.is_down) {
		game_state->camera_x += 0.05f;
	}
	if(input->arrow_up.is_down) {
		game_state->camera_y -= 0.05f;
	}
	if(input->arrow_down.is_down) {
		game_state->camera_y += 0.05f;
	}
	if(input->comma.is_down) {
		game_state->camera_z += 0.05f;
	}
	if(input->o.is_down) {
		game_state->camera_z -= 0.05f;
	}

}


INTERNAL void GameUpdateAndRender(GameMemory* memory, GameInput* input) {
	GameState* game_state = (GameState*)memory->storage;
	
	UpdateCamera(game_state, input);

	// BINARY TREE
	//BinaryTree* bst = (BinaryTree*)game_state->data_structure;
	//if(!game_state->initialized) {
	//	game_state->camera_z = -5.0f;
	//	game_state->selected_node = bst->head;
	//	game_state->initialized = true;
	//}
	//ProcessBSTInput(game_state, input, bst);	
	//DrawBST(game_state, input, bst);

	// QUEUE
	QueueInt* q = (QueueInt*)game_state->data_structure;
	if(!game_state->initialized) {
		game_state->camera_x = ((float)q->capacity / -2.0f) + 0.5f;
		game_state->camera_z = -5.0f;
		game_state->initialized = true;
	}
	ProcessQueueInput(input, q);
	DrawQueue(game_state, input, q);

	// ARRAY
	//ArrayStruct* a = (ArrayStruct*)game_state->data_structure;
	//if(!game_state->initialized) {
	//	game_state->camera_x = ((float)a->size / -2.0f) + 0.5f;
	//	game_state->camera_z = -5.0f;
	//	game_state->initialized = true;
	//}
	//UpdateArrayState(input, a);
	//DrawArray(game_state, a);
}
