#pragma once

class Model {
	int n_vertex_ = 0;
	float *vertex_;
	float *normal_;
	float *uv_;
public:
	Model(const char *file_name);
	~Model() {
		delete[] vertex_;
		delete[] normal_;
	}
	int n_vertex() {
		return n_vertex_;
	}
	float* vertex() {
		return vertex_;
	}
	float* normal() {
		return normal_;
	}
	float* uv() {
		return uv_;
	}
};
