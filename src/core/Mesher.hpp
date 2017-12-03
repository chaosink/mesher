#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace mesher {

enum EulerOp : unsigned char {
	Euler_Mvfs,
	Euler_Mve,
	Euler_Mef,
	Euler_KeMr,
	Euler_KfMrh,
};
struct EulerBase {
	EulerOp op;
	EulerBase(EulerOp op) : op(op) {}
};
struct EulerMvfs : public EulerBase {
	glm::vec3 p;
	EulerMvfs(float x, float y, float z)
		: EulerBase(Euler_Mvfs), p(x, y, z) {}
};
struct EulerMve : public EulerBase {
	glm::vec3 p;
	int v0, f;
	EulerMve(float x, float y, float z, int v0, int f)
		: EulerBase(Euler_Mve), p(x, y, z), v0(v0), f(f) {}
};
struct EulerMef : public EulerBase {
	int v0, v1, f0;
	EulerMef(int v0, int v1, int f0)
		: EulerBase(Euler_Mef), v0(v0), v1(v1), f0(f0) {}
};
struct EulerKeMr : public EulerBase {
	int e, f;
	EulerKeMr(int e, int f)
		: EulerBase(Euler_KeMr), e(e), f(f) {}
};
struct EulerKfMrh : public EulerBase {
	glm::vec3 p;
	EulerKfMrh()
		: EulerBase(Euler_KfMrh) {}
};

struct Face;
struct Loop;
struct Edge;
struct HalfEdge;
struct Vertex;

struct Solid {
	// Solid *prev, *next;
	Face *face;
	// Edge *edge;
	// Vertex *vertex;
};

struct Face {
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
	// Vertex *prev, *next;

	glm::vec3 position;
	glm::vec3 normal;

	Vertex(glm::vec3 position) : position(position) {}
};

class Mesher {
	std::vector<EulerBase*> euler_;

	std::vector<Solid*> solid_;
	std::vector<Face*> face_;
	std::vector<Edge*> edge_;
	std::vector<Vertex*> vertex_;

	void Mvfs(glm::vec3 p);
	void Mve(glm::vec3 p, Vertex *v0, Face *f);
	void Mef(Vertex *v0, Vertex *v1, Face *f0);
	void KeMr(Edge *e, Face *fm);
	void KfMrh(Face *f);

public:
	void LoadEuler(const char *file);
	void SaveEuler(const char *file);
	void Build();
	void PrintFace();
};

}
