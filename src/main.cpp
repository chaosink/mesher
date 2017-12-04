#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Mesher.hpp"
using namespace mesher;
#include "Model.hpp"
#include "Camera.hpp"
#include "FPS.hpp"

GLFWwindow* InitGLFW(int window_w, int window_H) {
	if(!glfwInit()) exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	GLFWwindow *window = glfwCreateWindow(window_w, window_H, "CUPix", NULL, NULL);
	if(!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
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
	// mesh.SaveOperator("save.op");
	mesh.Build();
	// mesh.Print();
	vector<glm::vec3> &vertex = mesh.Triangulate();

	int window_w = 1280;
	int window_h = 720;

	GLFWwindow* window = InitGLFW(window_w, window_h);












	return 0;
}
