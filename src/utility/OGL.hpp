#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

class OGL {
	GLuint shader_;
	GLuint mvp_, mv_;

	GLuint LoadShaderFromString(const char *vertex_string, const char *fragment_string, const char *geometry_string = nullptr);

public:
	OGL();
	~OGL();
	void LoadShader(const char *vertex_file_path, const char *fragment_file_path, const char *geometry_file_path = nullptr);
	void MVP(glm::mat4 mvp);
	void MV(glm::mat4 mv);
	GLuint shader() {
		return shader_;
	}
};
