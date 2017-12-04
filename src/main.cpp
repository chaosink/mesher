#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesher.hpp"
using namespace mesher;
#include "OGL.hpp"
#include "Camera.hpp"
#include "FPS.hpp"

GLFWwindow* InitGLFW(int window_w, int window_H) {
	if(!glfwInit()) exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	GLFWwindow *window = glfwCreateWindow(window_w, window_H, "Mesher", NULL, NULL);
	if(!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed for core profile
	if(glewInit() != GLEW_OK) {
		glfwTerminate();
		fprintf(stderr, "Failed to initialize GLEW\n");
		exit(EXIT_FAILURE);
	}

	glfwSwapInterval(1);
	return window;
}

void TermGLFW(GLFWwindow *window) {
	glfwDestroyWindow(window);
	glfwTerminate();
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		printf("Usage: mesher model_file\n");
		return 0;
	}

	Mesher mesh;
	mesh.LoadOperator(argv[1]);
	mesh.Build();
	vector<glm::vec3> &vertex = mesh.Triangulate();
	vector<glm::vec3> &normal = mesh.triangel_normal();

	int window_w = 1280;
	int window_h = 720;

	GLFWwindow* window = InitGLFW(window_w, window_h);
	OGL ogl;
	ogl.LoadShader("shader/vertex.glsl", "shader/fragment.glsl");

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertex.size(), vertex.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,        // index
		3,        // size
		GL_FLOAT, // type
		GL_FALSE, // normalized
		0,        // stride
		(void*)0  // pointer
	);
	GLuint normal_buffer;
	glGenBuffers(1, &normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normal.size(), normal.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,        // index
		3,        // size
		GL_FLOAT, // type
		GL_FALSE, // normalized
		0,        // stride
		(void*)0  // pointer
	);

	double time = glfwGetTime();
	Camera camera(window, window_w, window_h, time);
	FPS fps(time);
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
		time = glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 m = glm::scale(glm::mat4(), glm::vec3(0.2f));
		glm::mat4 vp = camera.Update(time);
		glm::mat4 mvp = vp * m;
		ogl.MVP(mvp);
		glm::mat4 v = camera.v();
		glm::mat4 mv = v * m;
		ogl.MV(mv);

		glDrawArrays(GL_TRIANGLES, 0, vertex.size());

		glfwSwapBuffers(window);
		glfwPollEvents();
		fps.Update(time);
	}
	fps.Term();

	glDeleteBuffers(1, &vertex_buffer);
	glDeleteVertexArrays(1, &VertexArrayID);

	TermGLFW(window);
	return 0;
}
