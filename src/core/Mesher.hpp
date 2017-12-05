#pragma once

#include <vector>

#include <glm/glm.hpp>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 1

#define _QUOTE(S) #S
#define _STR(S) _QUOTE(S)
#define VERSION_STRING _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "." _STR(VERSION_PATCH)

namespace mesher {

enum OperatorEnum : unsigned char {
	Euler_Mvfs,
	Euler_Mve,
	Euler_Mef,
	Euler_KeMr,
	Euler_KfMrh,
	Op_Sweep,
};
struct OperatorBase {
	OperatorEnum op;
	OperatorBase(OperatorEnum op) : op(op) {}
};
struct EulerMvfs : public OperatorBase {
	glm::vec3 p;
	EulerMvfs(float x, float y, float z)
		: OperatorBase(Euler_Mvfs), p(x, y, z) {}
};
struct EulerMve : public OperatorBase {
	glm::vec3 p;
	int v0, f;
	EulerMve(float x, float y, float z, int v0, int f)
		: OperatorBase(Euler_Mve), p(x, y, z), v0(v0), f(f) {}
};
struct EulerMef : public OperatorBase {
	int v0, v1, f0;
	EulerMef(int v0, int v1, int f0)
		: OperatorBase(Euler_Mef), v0(v0), v1(v1), f0(f0) {}
};
struct EulerKeMr : public OperatorBase {
	int e, f;
	EulerKeMr(int e, int f)
		: OperatorBase(Euler_KeMr), e(e), f(f) {}
};
struct EulerKfMrh : public OperatorBase {
	int f0, f1;
	EulerKfMrh(int f0, int f1)
		: OperatorBase(Euler_KfMrh), f0(f0), f1(f1) {}
};
struct OpSweep : public OperatorBase {
	int f;
	glm::vec3 d;
	float t;
	OpSweep(int f, float x, float y, float z, float t)
		: OperatorBase(Op_Sweep), f(f), d(x, y, z), t(t) {}
};

struct Face;
struct Loop;
struct Edge;
struct HalfEdge;
struct Vertex;

struct Solid {
	// Solid *prev, *next;
	int face;
	// Edge *edge;
	// Vertex *vertex;
};

struct Face {
	// Face *prev, *next;
	int solid;

	Loop *loop;

	// glm::vec3 normal;
};

struct Loop {
	int face;

	Loop *prev, *next;
	HalfEdge *half_edge;

	Loop(int face) : face(face) {
		prev = next = this;
	}
};

struct Edge {
	// Edge *prev, *next;
	// Solid *solid;
	HalfEdge *half_edge[2];
};

struct HalfEdge {
	int edge;
	int vertex;

	HalfEdge *prev, *next;
	Loop *loop;

	HalfEdge *twin;

	HalfEdge(int edge, int vertex) : edge(edge), vertex(vertex) {}
};

struct Vertex {
	// Vertex *prev, *next;

	glm::dvec3 position;
	// glm::vec3 normal;

	Vertex(glm::dvec3 position) : position(position) {}
};

class Mesher {
	std::vector<OperatorBase*> operator_;

	std::vector<Solid*> solid_;
	std::vector<Face*> face_;
	std::vector<Edge*> edge_;
	std::vector<Vertex*> vertex_;

	std::vector<glm::vec3> triangel_vertex_;
	std::vector<glm::vec3> triangel_normal_;

	bool InLoop(int v, Loop *l);
	void AddLoop(int f, Loop *l1);
	void SetLoop(HalfEdge *he, Loop *l);
	void Mvfs(glm::vec3 p);
	int Mve(glm::vec3 p, int v0, Loop *l);
	int Mve(glm::vec3 p, int v0, int f);
	void Mef(int v0, int v1, Loop *l0);
	void Mef(int v0, int v1, int f0);
	void KeMr(int e, int f);
	void KfMrh(int f0, int f1);
	void Sweep(int f, glm::dvec3 d, double t);

public:
	void LoadOperator(const char *file);
	void SaveOperator(const char *file);
	void Build();
	std::vector<glm::vec3> &Triangulate();
	std::vector<glm::vec3> TriangulateFace(int f);
	void PrintFace(int f);
	void Print();
	void PrintLoop(Loop *l);
	std::vector<glm::vec3> &triangel_vertex() {
		return triangel_vertex_;
	}
	std::vector<glm::vec3> &triangel_normal() {
		return triangel_normal_;
	}
};

}
