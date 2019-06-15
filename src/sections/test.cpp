#include "main.h"

typedef struct {
	int shader;
	GLuint vao;
} test_section;

static test_section *local;

sTest::sTest() {
	type = SectionType::Test;
}

bool sTest::load() {
	local = (test_section*)malloc(sizeof(test_section));

	this->vars = (void *)local;
	local->shader = DEMO->shaderManager.addShader(	DEMO->dataFolder + "/resources/shaders/particleSystem/transformBasics.vert",
													DEMO->dataFolder + "/resources/shaders/particleSystem/transformBasics.frag",
													DEMO->dataFolder + "/resources/shaders/particleSystem/transformBasics.geom",
													{ "outValue" });
	if (local->shader < 0)
		return false;
	
	/*
	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();

	// Create VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create input VBO and vertex format
	GLfloat data[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	GLint inputAttrib = glGetAttribLocation(my_shader->ID, "inValue");
	glEnableVertexAttribArray(inputAttrib);
	glVertexAttribPointer(inputAttrib, 1, GL_FLOAT, GL_FALSE, 0, 0);

	// Create transform feedback buffer
	GLuint tbo;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data) * 3, nullptr, GL_STATIC_READ);

	// Create query object to collect info
	GLuint query;
	glGenQueries(1, &query);

	// Perform feedback transform
	glEnable(GL_RASTERIZER_DISCARD);

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_TRIANGLES);
	glDrawArrays(GL_POINTS, 0, 5);
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glDisable(GL_RASTERIZER_DISCARD);

	glFlush();

	// Fetch and print results
	GLuint primitives;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);

	GLfloat feedback[15];
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);

	LOG->Error("%u primitives written!", primitives);

	for (int i = 0; i < 15; i++) {
		LOG->Error("%f", feedback[i]);
	}

	glDeleteQueries(1, &query);

	glDeleteBuffers(1, &tbo);
	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);
	*/
	return true;
}

void sTest::init() {
}


void sTest::exec() {
	local = (test_section *)this->vars;
	
	
}

void sTest::end() {
}
