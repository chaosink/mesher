#include "OGL.hpp"

#include <cstdio>
#include <vector>
#include <string>
#include <fstream>

OGL::OGL() {
	glClearColor(0.08f, 0.16f, 0.24f, 1.f);
	// glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}

OGL::~OGL() {
	glDeleteProgram(shader_);
}

GLuint OGL::LoadShaderFromString(const char *vertex_string, const char *fragment_string, const char *geometry_string) {
	// Create the shaders
	GLuint VertexShaderID   = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader: %s\n", "vertex");
	char const * VertexSourcePointer = vertex_string;
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);
	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if(InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader: %s\n", "fragment");
	char const * FragmentSourcePointer = fragment_string;
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);
	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

if(geometry_string) {
	// Compile Geometry Shader
	printf("Compiling shader: %s\n", "geometry");
	char const * GeometrySourcePointer = geometry_string;
	glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , NULL);
	glCompileShader(GeometryShaderID);
	// Check Geometry Shader
	glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> GeometryShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
		printf("%s\n", &GeometryShaderErrorMessage[0]);
	}
}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
if(geometry_string)
	glAttachShader(ProgramID, GeometryShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	glDeleteShader(GeometryShaderID);

	return ProgramID;
}

void OGL::LoadShader(const char *vertex_file_path, const char *fragment_file_path, const char *geometry_file_path) {
	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()) {
		std::string Line = "";
		while(getline(VertexShaderStream, Line)) VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	} else {
		printf("Impossible to open %s. Are you in the right directory?\n", vertex_file_path);
		return;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	} else {
		printf("Impossible to open %s. Are you in the right directory?\n", fragment_file_path);
		return;
	}

	// Read the Geometry Shader code from the file
	std::string GeometryShaderCode;
	if(geometry_file_path) {
		std::ifstream GeometryShaderStream(geometry_file_path, std::ios::in);
		if(GeometryShaderStream.is_open()){
			std::string Line = "";
			while(getline(GeometryShaderStream, Line))
				GeometryShaderCode += "\n" + Line;
			GeometryShaderStream.close();
		} else {
			printf("Impossible to open %s. Are you in the right directory?\n", geometry_file_path);
			return;
		}
	}

	shader_ = LoadShaderFromString(VertexShaderCode.c_str(), FragmentShaderCode.c_str(), geometry_file_path ? GeometryShaderCode.c_str() : NULL);
	glUseProgram(shader_);
	mvp_ = glGetUniformLocation(shader_, "mvp");
	mv_ = glGetUniformLocation(shader_, "mv");
}

void OGL::MVP(glm::mat4 mvp) {
	glUniformMatrix4fv(mvp_, 1, GL_FALSE, &mvp[0][0]);
}

void OGL::MV(glm::mat4 mv) {
	glUniformMatrix4fv(mv_, 1, GL_FALSE, &mv[0][0]);
}
