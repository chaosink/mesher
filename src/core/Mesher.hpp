#pragma once

#include <vector>

#include <glm/glm.hpp>

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
	glm::vec3 p;
	EulerKfMrh()
		: OperatorBase(Euler_KfMrh) {}
};
struct OpSweep : public OperatorBase {
	glm::vec3 d;
	float t;
	OpSweep(float x, float y, float z, float t)
		: OperatorBase(Op_Sweep), d(x, y, z), t(t) {}
};

struct Face;
struct Loop;
struct Edge;
struct HalfEdge;
struct Vertex;

struct Solid {
	int no;
	// Solid *prev, *next;
	Face *face;
	// Edge *edge;
	// Vertex *vertex;
};

struct Face {
	int no;
	// Face *prev, *next;
	Solid *solid;
	Loop *loop;

	// glm::vec3 normal;
};

struct Loop {
	Loop *prev, *next;
	Face *face;
	HalfEdge *half_edge;

	Loop(Face *face) : face(face) {
		prev = next = this;
	}
};

struct Edge {
	int no;
	// Edge *prev, *next;
	// Solid *solid;
	HalfEdge *half_edge[2];
};

struct HalfEdge {
	HalfEdge *prev, *next;
	Loop *loop;
	Edge *edge;
	Vertex *vertex;

	HalfEdge *twin;

	HalfEdge(Edge *edge, Vertex *vertex) : edge(edge), vertex(vertex) {}
};

struct Vertex {
	int no;
	// Vertex *prev, *next;

	glm::vec3 position;
	glm::vec3 normal;

	Vertex(glm::vec3 position) : position(position) {}
};

class Mesher {
	std::vector<OperatorBase*> operator_;

	std::vector<Solid*> solid_;
	std::vector<Face*> face_;
	std::vector<Edge*> edge_;
	std::vector<Vertex*> vertex_;

	bool InLoop(Vertex *v, Loop *l);
	void Mvfs(glm::vec3 p);
	Vertex *Mve(glm::vec3 p, Vertex *v0, Loop *l);
	Vertex *Mve(glm::vec3 p, Vertex *v0, Face *f);
	void Mef(Vertex *v0, Vertex *v1, Loop *l0);
	void Mef(Vertex *v0, Vertex *v1, Face *f0);
	void KeMr(Edge *&e, Face *f);
	void KfMrh(Face *&f);
	void Sweep(Face *f, glm::vec3 d, float t);

public:
	void LoadOperator(const char *file);
	void SaveOperator(const char *file);
	void Build();
	void PrintFace();
};

}
