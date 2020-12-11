#include "engine.h"
#include "stdlib.h"
#include "..\\include\\queue.h"
#include "..\\include\\binary_tree.h"
#include "time.h"

#define MAX_TREE_SIZE 100

GLOBAL PlatformAPI global_platform_api; // this could be passed around instead of being a global ??

INTERNAL inline void ProcessBSTInput(GameState* game_state, GameInput* input, BinaryTree* bst) {
	if(input->successor.is_down) {
		Node* s = Successor(bst, game_state->selected_node);
		if(s) {
			game_state->selected_node = s;
		}
	}

	if(input->predecessor.is_down) {
		Node* p = Predecessor(bst, game_state->selected_node);
		if(p) {
			game_state->selected_node = p;
		}
	}

	if(input->d_right.is_down) {
		Delete(bst, game_state->selected_node);
		game_state->selected_node = bst->head;
	}

	if(input->a_left.is_down) {
		int num = rand() % 100;
		Node* s = Search(bst, num);
		if(!s) {
			Node* n = AllocateNode(num, num);
			Insert(bst, n);
		}

		if(bst->size == 1) {
			game_state->selected_node = bst->head;
		}
	}
}

INTERNAL void DrawBST(GameState* game_state, GameInput* input, BinaryTree* b) {

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
	Node* nodes[MAX_TREE_SIZE] = {NULL};
	Node* children[MAX_TREE_SIZE] = {NULL};
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
	float x_spacing = 0.5f; // space between the nodes at the bottom level
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


	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLineWidth(4.0f);
	glUseProgram(game_state->shader);

	int node_index = 0;
	for(int level = 0; level < num_levels; ++level) {
		for(int node_location = 0; node_location < split; node_location += 2) {
			if(nodes[node_index] != NULL) {
				if(nodes[node_index] == game_state->selected_node) {
					glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
				}
				else {
					glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
				}
				node_model = glm::translate(identity, glm::vec3(current_x_location, -y_spacing * level, 0.0f));
				glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(node_model));
				glBindVertexArray(game_state->node.vao);
				glDrawElements(GL_LINES, game_state->node.num_indices, GL_UNSIGNED_INT, (void*)0);

				digit_model = glm::scale(identity, glm::vec3(digit_scale, digit_scale, digit_scale));
				digit_model = glm::translate(digit_model, glm::vec3(current_x_location / digit_scale, -y_spacing * level / digit_scale, -0.5f / digit_scale));
				glUniform3f(color_location, 0.0f, 1.0f, 0.0f);
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

INTERNAL inline void ProcessQueueInput(GameInput* input, QueueInt* q) {
	if(input->a_left.is_down) {
		int num = rand() % 100;
		EnqueueInt(q, num);
	}
	else if(input->d_right.is_down) {
		int error = 0;
		DequeueInt(q, &error);
	}
}

INTERNAL void DrawQueue(GameState* game_state, GameInput* input, QueueInt q) {
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
	view = glm::translate(view, glm::vec3(game_state->camera_x, 0.0f, game_state->camera_z));
	glm::mat4 projection = glm::perspective(glm::radians(75.0f), (float)game_state->window_width / (float)game_state->window_height, 0.1f, 100.0f);

	glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));


	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLineWidth(4.0f);
	glUseProgram(game_state->shader);

	for(int i = 0; i < q.capacity; ++i) {
		// draw node
		glUniform3f(color_location, 0.0f, 0.0f, 1.0f);
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(node_model));
		glBindVertexArray(game_state->node.vao);
		glDrawElements(GL_LINES, game_state->node.num_indices, GL_UNSIGNED_INT, (void*)0);
		node_model = glm::translate(node_model, glm::vec3(1.0f, 0.0f, 0.0f));

		// draw digit
		if(IndexIsInQueue(&q, i)) {
			glUniform3f(color_location, 0.0f, 1.0f, 0.0f);
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(digit_model));
			glBindVertexArray(game_state->digits[q.elements[i]].vao);
			glDrawElements(GL_LINES, game_state->digits[q.elements[i]].num_indices, GL_UNSIGNED_INT, (void*)0);
		}
		digit_model = glm::translate(digit_model, glm::vec3(1.0f / digit_scale, 0.0f, 0.0f));

		// draw arrow
		if(i == q.front) {
			glUniform3f(color_location, 0.0f, 1.0f, 0.0f);
			glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(arrow_model));
			glBindVertexArray(game_state->arrow.vao);
			glDrawElements(GL_LINES, game_state->arrow.num_indices, GL_UNSIGNED_INT, (void*)0);
		}
		else if(i == q.back) {
			glUniform3f(color_location, 1.0f, 0.0f, 0.0f);
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
	if(input->w_up.is_down) {
		game_state->camera_z += 0.05f;
	}
	if(input->s_down.is_down) {
		game_state->camera_z -= 0.05f;
	}

}

extern "C" GAME_LOAD_PLATFORM_API(GameLoadPlatformAPI) {
	global_platform_api = platform_api;
	if (global_platform_api.DEBUG_ReadEntireFile &&
	    global_platform_api.DEBUG_FreeFileMemory &&
	    global_platform_api.DEBUG_WriteEntireFile) {
		return true;
	}
	return false;
}

extern "C" GAME_GLEW_INIT(GameGlewInit) {
	glewExperimental = GL_TRUE;
	bool initialized = (glewInit() == GLEW_OK);
	return initialized;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
	GameState* game_state = (GameState*)memory->storage;
	BinaryTree* bst = (BinaryTree*)game_state->data_structure;
	
	UpdateCamera(game_state, input);


	/*
	static QueueInt q;
	if(q.elements == NULL) {
		srand((unsigned int)time(NULL));
		q = CreateQueueInt(10);
		if(!game_state->initialized) {
			game_state->camera_x = ((float)q.capacity / -2.0f) + 0.5f;
			game_state->camera_z = -5.0f;
			game_state->initialized = true;
		}
	}
	ProcessQueueInput(input, &q);
	DrawQueue(game_state, input, q);
	*/
	
	if(!game_state->initialized) {
		game_state->camera_z = -5.0f;
		game_state->initialized = true;
		game_state->selected_node = Min(bst);
	}

	ProcessBSTInput(game_state, input, bst);	
	DrawBST(game_state, input, bst);
}
