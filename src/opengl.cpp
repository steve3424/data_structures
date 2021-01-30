#include <stdlib.h>

/*
*
* So far the indices_2 argument is only used in double digits 
* so the way the indices are combined is specific to this use case
*/
INTERNAL GameObject LoadObject(float* vertices, unsigned int* indices, unsigned int* indices_2, int num_vertices, int num_indices, int num_indices_2) {
	// COMBINE INDICES HERE
	// max number of indices is 28 used in the digit '88'
	// add 5 to indices_2 since it is used for the double digit
	unsigned int total_indices[28];
	int t = 0;
	for(int i = 0; i < num_indices; ++i) {
		total_indices[t] = indices[i];
		++t;
	}
	for(int i = 0; i < num_indices_2; ++i) {
		total_indices[t] = indices_2[i] + 6;
		++t;
	}

	
	unsigned int vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*sizeof(float), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (num_indices + num_indices_2)*sizeof(unsigned int), total_indices, GL_STATIC_DRAW);

	GameObject g = {vao, num_indices + num_indices_2};
	return g;
}

INTERNAL unsigned int LoadShaderProgram(char* vert_file, char* frag_file) {
	Win32FileResult vert = Win32ReadEntireFile(vert_file);
	Win32FileResult frag = Win32ReadEntireFile(frag_file);

	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &(const char*)vert.file, NULL);
	glCompileShader(vertex_shader);
	int success;
	char log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, log);
	}
	glShaderSource(fragment_shader, 1, &(const char*)frag.file, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, log);
	}

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	Win32FreeFileResult(&vert);
	Win32FreeFileResult(&frag);

	return shader_program;
}

INTERNAL void LoadLineSegment(GameState* game_state) {
	float vertices[] = {
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f
	};

	unsigned int indices[] = {
		0, 1
	};

	game_state->line = LoadObject(vertices, indices, NULL, 6, 2, 0);

}

INTERNAL void LoadDigits(GameState* game_state) {
	// clock wise from top left
	// I separated single and double digit vertices so
	// the single digits could are centered by default
	float single_digit_vertices[] = {
		-0.15f,	0.5f, 0.0f,
		 0.15f,	0.5f, 0.0f,
		 0.15f,  0.0f, 0.0f,
		 0.15f, -0.5f, 0.0f,
		-0.15f, -0.5f, 0.0f,
		-0.15f,  0.0f, 0.0f
	};

	float double_digit_vertices[] = {
		-0.5f, 0.5f, 0.0f,
		-0.2f, 0.5f, 0.0f,
		-0.2f, 0.0f, 0.0f,
		-0.2f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f, 0.0f, 0.0f,

		0.2f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.2f, -0.5f, 0.0f,
		0.2f, 0.0f, 0.0f
	};

	unsigned int zero_indices[] = {
		0, 1,
		1, 2,
		2, 3,
		3, 4,
		4, 5,
		5, 0
	};

	unsigned int one_indices[] = {
		1, 2,
		2, 3
	};

	unsigned int two_indices[] = {
		0, 1,
		1, 2,
		2, 5,
		5, 4,
		4, 3
	};

	unsigned int three_indices[] = {
		0, 1,
		1, 2,
		2, 5,
		2, 3,
		3, 4
	};

	unsigned int four_indices[] = {
		0, 5,
		5, 2,
		2, 1,
		2, 3
	};

	unsigned int five_indices[] = {
		1, 0,
		0, 5,
		5, 2,
		2, 3,
		3, 4
	};

	unsigned int six_indices[] = {
		1, 0,
		0, 5,
		5, 4,
		4, 3,
		3, 2,
		2, 5
	};

	unsigned int seven_indices[] = {
		5, 0,
		0, 1,
		1, 2,
		2, 3
	};

	unsigned int eight_indices[] = {
		0, 1,
		1, 2,
		2, 3,
		3, 4,
		4, 5,
		5, 0,
		2, 5
	};

	unsigned int nine_indices[] = {
		0, 1,
		1, 2,
		2, 3,
		3, 4,
		2, 5,
		5, 0
	};

	// TO BE USED IN LOOP
	unsigned int* indices[10] = {
		zero_indices,
		one_indices,
		two_indices,
		three_indices,
		four_indices,
		five_indices,
		six_indices,
		seven_indices,
		eight_indices,
		nine_indices
	};
	
	int num_indices[] = {
		12,
		4,
		10,
		10,
		8,
		10,
		12,
		8,
		14,
		12
	};
	
	int digit_index;
	for(digit_index = 0; digit_index < 10; ++digit_index) {
		game_state->digits[digit_index] = LoadObject(single_digit_vertices, indices[digit_index], NULL, 18, num_indices[digit_index], 0);
	}

	for(int i = 1; i < 10; ++i) {
		for(int j = 0; j < 10; ++j) {
			game_state->digits[digit_index] = LoadObject(double_digit_vertices, indices[i], indices[j], 36, num_indices[i], num_indices[j]);
			++digit_index;
		}
	}

	char* vert_path = "..\\zshaders\\generic.vert";
	char* frag_path = "..\\zshaders\\generic.frag";
	game_state->shader = LoadShaderProgram(vert_path, frag_path);

	glBindVertexArray(0);
}


INTERNAL void LoadNode(GameState* game_state) {
	// USING GL_LINES
	// top-left clockwise
	// front face, then back face
	float vertices[] {
		// face 1
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,

		// face 2
		-0.5f, 0.5f, -1.0f,
		0.5f, 0.5f, -1.0f,
		0.5f, -0.5f, -1.0f,
		-0.5f, -0.5f, -1.0f
	};

	unsigned int indices[] {
		// face 1
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		// face 2
		4, 5,
		5, 6,
		6, 7,
		7, 4,

		// connect faces
		0, 4,
		1, 5,
		2, 6,
		3, 7
	};


	game_state->node = LoadObject(vertices, indices, NULL, 24, 24, 0);
	glBindVertexArray(0);
}

INTERNAL void LoadArrow(GameState* game_state) {
	float vertices[] = {
		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f,
		-0.25f, 0.25f, 0.0f,
		0.25f, 0.25f, 0.0f
	};

	unsigned int indices[] = {
		0, 1,
		0, 2,
		0, 3
	};

	game_state->arrow = LoadObject(vertices, indices, NULL, 12, 6, 0);
	glBindVertexArray(0);
}

INTERNAL void LoadQueueData(GameState* game_state) {
	char* vert_path = "..\\zshaders\\generic.vert";
	char* frag_path = "..\\zshaders\\generic.frag";
	game_state->shader = LoadShaderProgram(vert_path, frag_path);
	LoadDigits(game_state);
	LoadNode(game_state);
	LoadArrow(game_state);
}

INTERNAL void LoadBSTData(GameState* game_state) {
	char* vert_path = "..\\zshaders\\generic.vert";
	char* frag_path = "..\\zshaders\\generic.frag";
	game_state->shader = LoadShaderProgram(vert_path, frag_path);
	LoadDigits(game_state);
	LoadNode(game_state);
	LoadLineSegment(game_state);
}

INTERNAL void LoadArrayData(GameState* game_state) {
	char* vert_path = "..\\zshaders\\generic.vert";
	char* frag_path = "..\\zshaders\\generic.frag";
	game_state->shader = LoadShaderProgram(vert_path, frag_path);
	LoadDigits(game_state);
	LoadNode(game_state);
}

/*
INTERNAL void HelloTriangle(GameState* game_state) {
	// BUFFER ALL VERTICES
	float vertices[] = {
		-0.9f, -0.9f, 0.0f,	// bottom left
		-0.5f, -0.9f, 0.0f,	// bottom right 
		-0.7f, 0.0f, 0.0f,	// top
	}; 

	float vertices_2[] = {
		-0.49f, -0.9f, 0.0f,
		-0.69f, 0.0f, 0.0f, 
		-0.29f, 0.0f, 0.0f, 
	};

	unsigned int vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	unsigned int vao_2;
	glCreateVertexArrays(1, &vao_2);
	glBindVertexArray(vao_2);

	unsigned int vbo_2;
	glGenBuffers(1, &vbo_2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_2);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), vertices_2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	// LOAD SHADERS

	DEBUG_ReadFileResult vert = DEBUG_Win32ReadEntireFile("..\\zshaders\\tri.vert");
	DEBUG_ReadFileResult frag = DEBUG_Win32ReadEntireFile("..\\zshaders\\tri.frag");
	DEBUG_ReadFileResult vert_2 = DEBUG_Win32ReadEntireFile("..\\zshaders\\tri_2.vert");
	DEBUG_ReadFileResult frag_2 = DEBUG_Win32ReadEntireFile("..\\zshaders\\tri_2.frag");

	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &(const char*)vert.contents, NULL);
	glCompileShader(vertex_shader);
	glShaderSource(fragment_shader, 1, &(const char*)frag.contents, NULL);
	glCompileShader(fragment_shader);

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);


	unsigned int vertex_shader_2 = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_shader_2 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader_2, 1, &(const char*)vert_2.contents, NULL);
	glCompileShader(vertex_shader_2);
	glShaderSource(fragment_shader_2, 1, &(const char*)frag_2.contents, NULL);
	glCompileShader(fragment_shader_2);

	unsigned int shader_program_2 = glCreateProgram();
	glAttachShader(shader_program_2, vertex_shader_2);
	glAttachShader(shader_program_2, fragment_shader_2);
	glLinkProgram(shader_program_2);

	DEBUG_Win32FreeFileMemory(vert.contents);
	DEBUG_Win32FreeFileMemory(frag.contents);
	DEBUG_Win32FreeFileMemory(vert_2.contents);
	DEBUG_Win32FreeFileMemory(frag_2.contents);

	game_state->vao[0] = vao;
	game_state->shader[0] = shader_program;
	game_state->vao[1] = vao_2;
	game_state->shader[1] = shader_program_2;
}

INTERNAL void HelloTexture(const unsigned char* image_data, int width, int height, GameState* game_state) {
	// BUFFER ALL VERTICES
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 	// bottom left
		0.5f, -0.5f, 0.0f, 	1.0f, 0.0f,	// bottom right 
		0.0f, 0.5f, 0.0f, 	0.5f, 1.0f	// top
	}; 

	unsigned int vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 15*sizeof(float), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(0);


	// CREATE TEXTURE
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// LOAD SHADERS
	DEBUG_ReadFileResult vert = DEBUG_Win32ReadEntireFile("..\\zshaders\\tex.vert");
	DEBUG_ReadFileResult frag = DEBUG_Win32ReadEntireFile("..\\zshaders\\tex.frag");


	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &(const char*)vert.contents, NULL);
	glCompileShader(vertex_shader);
	int success;
	char log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, log);
	}
	glShaderSource(fragment_shader, 1, &(const char*)frag.contents, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, log);
	}

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);


	DEBUG_Win32FreeFileMemory(vert.contents);
	DEBUG_Win32FreeFileMemory(frag.contents);

	game_state->vao[0] = vao;
	game_state->texture[0] = tex;
	game_state->shader[0] = shader_program;
}

INTERNAL void HelloRectangle(GameState* game_state, int width, int height, const unsigned char* image_data) {
	float vertices[] = {
	    // positions		// texture coords
	     0.5f,  0.5f, 0.0f,		1.0f, 1.0f,   // top right
	     0.5f, -0.5f, 0.0f,		1.0f, 0.0f,   // bottom right
	    -0.5f, -0.5f, 0.0f,		0.0f, 0.0f,   // bottom left
	    -0.5f,  0.5f, 0.0f,		0.0f, 1.0f    // top left 
	};

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	unsigned int vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 20*sizeof(float), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(unsigned int), indices, GL_STATIC_DRAW);
	
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
	glGenerateMipmap(GL_TEXTURE_2D);


	// LOAD SHADERS
	DEBUG_ReadFileResult vert = DEBUG_Win32ReadEntireFile("..\\zshaders\\rectangle.vert");
	DEBUG_ReadFileResult frag = DEBUG_Win32ReadEntireFile("..\\zshaders\\rectangle.frag");


	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &(const char*)vert.contents, NULL);
	glCompileShader(vertex_shader);
	int success;
	char log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, log);
	}
	glShaderSource(fragment_shader, 1, &(const char*)frag.contents, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, log);
	}

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glUseProgram(shader_program);

	game_state->shader[0] = shader_program;
}

INTERNAL void CreateMesh(GameState* game_state) {
	game_state->rows = 10;
	game_state->cols = 10;
	game_state->x_offset = 0.0f;
	
	// need extra vertex in x and y to create full squares
	int rows = game_state->rows;
	int cols = game_state->cols;
	int col_vertices = cols + 1;
	int row_vertices = rows + 1;

	float* vertices = (float*)malloc(col_vertices * row_vertices * 3 * sizeof(float));
	float* vertex = vertices;
	for(int y=0; y < row_vertices; ++y) {
		for(int x=0; x < col_vertices; ++x) {
			float x_val = ((float)x / ((float)cols / 10.0f)) - 5.0f;
			float y_val = (((float)y / ((float)rows / 10.0f)) - 5.0f) * -1.0f;
			float z_val = 0.0f; //(float)rand() / ((float)RAND_MAX * 3.0f);

			*vertex = x_val;
			*(++vertex) = y_val;
			*(++vertex) = z_val;
			++vertex;
		}
	}

	unsigned int* indices = (unsigned int*)malloc(sizeof(unsigned int) * cols * rows * 6);
	unsigned int* index = indices;
	int stride = col_vertices;
	for(int y=0; y < rows; ++y) {
		for(int x=0; x < cols; ++x) {
			unsigned int i = x + y*stride;
			*index = i;
			*(++index) = i + 1;
			*(++index) = i + stride;
			*(++index) = i + stride;
			*(++index) = i + 1;
			*(++index) = i + stride + 1;
			++index;
		}
	}
	
	unsigned int vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, row_vertices*col_vertices*3*sizeof(float), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, rows*cols*6*sizeof(unsigned int), indices, GL_STATIC_DRAW);
	

	// LOAD SHADERS
	DEBUG_ReadFileResult vert = DEBUG_Win32ReadEntireFile("..\\zshaders\\mesh.vert");
	DEBUG_ReadFileResult frag = DEBUG_Win32ReadEntireFile("..\\zshaders\\mesh.frag");


	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &(const char*)vert.contents, NULL);
	glCompileShader(vertex_shader);
	int success;
	char log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, log);
	}
	glShaderSource(fragment_shader, 1, &(const char*)frag.contents, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, log);
	}

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glUseProgram(shader_program);

	game_state->shader[0] = shader_program;

	DEBUG_Win32FreeFileMemory(vert.contents);
	DEBUG_Win32FreeFileMemory(frag.contents);
	free(vertices);
	free(indices);
}

INTERNAL void HelloCube(GameState* game_state) {
	float vertices[] = {
	    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	unsigned int vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 36*5*sizeof(float), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	// LOAD TEXTURE

	stbi_set_flip_vertically_on_load(true);
	int width, height, n_channels;
	unsigned char* image_data = stbi_load("..\\textures\\container.jpg", &width, &height, &n_channels, 0);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
	glGenerateMipmap(GL_TEXTURE_2D);


	// LOAD SHADERS
	DEBUG_ReadFileResult vert = DEBUG_Win32ReadEntireFile("..\\zshaders\\cube.vert");
	DEBUG_ReadFileResult frag = DEBUG_Win32ReadEntireFile("..\\zshaders\\cube.frag");


	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, &(const char*)vert.contents, NULL);
	glCompileShader(vertex_shader);
	int success;
	char log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, log);
	}
	glShaderSource(fragment_shader, 1, &(const char*)frag.contents, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, log);
	}

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glUseProgram(shader_program);

	game_state->shader[0] = shader_program;
}
*/
