#include "Mesher.hpp"

#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#define CALLBACK __stdcall
#else
#define CALLBACK
#endif

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

namespace mesher {

template <typename T>
std::vector<T> operator+(const std::vector<T> &v0, const std::vector<T> &v1) {
	std::vector<T> v;
	v.reserve(v0.size() + v1.size());
	v.insert(v.end(), v0.begin(), v0.end());
	v.insert(v.end(), v1.begin(), v1.end());
	return v;
}

template <typename T>
std::vector<T> &operator+=(std::vector<T> &v0, const std::vector<T> &v1) {
	v0.reserve(v0.size() + v1.size());
	v0.insert(v0.end(), v1.begin(), v1.end());
	return v0;
}

void Mesher::LoadOperator(const char *file) {
	std::ifstream ifs(file);
	std::string op;
	float x, y, z, t;
	int n0, n1, n2;
	char c;

	while(ifs >> op) {
		if(op[0] == '#') {
			ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		} else if(op == "Mvfs") {
			ifs.ignore(std::numeric_limits<std::streamsize>::max(), '(');
			ifs >> x >> y >> z;
			operator_.push_back(new EulerMvfs(x, y, z));
			ifs.ignore(std::numeric_limits<std::streamsize>::max(), ')');
		} else if(op == "Mve") {
			ifs.ignore(std::numeric_limits<std::streamsize>::max(), '(');
			ifs >> x >> y >> z;
			ifs.ignore(std::numeric_limits<std::streamsize>::max(), ')');
			ifs >> c >> n0 >> c >> n1;
			operator_.push_back(new EulerMve(x, y, z, n0, n1));
		} else if(op == "Mef") {
			ifs >> c >> n0 >> c >> n1 >> c >> n2;
			operator_.push_back(new EulerMef(n0, n1, n2));
		} else if(op == "KeMr") {
			ifs >> c >> n0 >> c >> n1;
			operator_.push_back(new EulerKeMr(n0, n1));
		} else if(op == "KfMrh") {

		} else if(op == "Sweep") {
			ifs >> c >> n0;
			ifs.ignore(std::numeric_limits<std::streamsize>::max(), '(');
			ifs >> x >> y >> z;
			ifs.ignore(std::numeric_limits<std::streamsize>::max(), ')');
			ifs >> t;
			operator_.push_back(new OpSweep(n0, x, y, z, t));
		}
	}
	ifs.close();
}

void Mesher::SaveOperator(const char *file) {
	FILE *f = fopen(file, "w");
	for(auto op: operator_) {
		switch(op->op) {
			case Euler_Mvfs: {
				auto o = static_cast<EulerMvfs*>(op);
				fprintf(f, "Mvfs (%f %f %f)\n", o->p.x, o->p.y, o->p.z);
				break;
			}
			case Euler_Mve: {
				auto o = static_cast<EulerMve*>(op);
				fprintf(f, "Mve (%f %f %f) v%d f%d\n", o->p.x, o->p.y, o->p.z, o->v0, o->f);
				break;}
			case Euler_Mef: {
				auto o = static_cast<EulerMef*>(op);
				fprintf(f, "Mef v%d v%d f%d\n", o->v0, o->v1, o->f0);
				break;
			}
			case Euler_KeMr: {
				auto o = static_cast<EulerKeMr*>(op);
				fprintf(f, "KeMr e%d f%d\n", o->e, o->f);
				break;
			}
			case Euler_KfMrh: {
				auto o = static_cast<EulerKfMrh*>(op);
				fprintf(f, "KfMrh f%d f%d\n", o->f0, o->f1);
				break;
			}
			case Op_Sweep: {
				auto o = static_cast<OpSweep*>(op);
				fprintf(f, "Sweep f%d (%f %f %f) %f\n", o->f, o->d.x, o->d.y, o->d.z, o->t);
				break;
			}
		}
	}
	fclose(f);
}

bool Mesher::InLoop(int v, Loop *l) {
	HalfEdge *he = l->half_edge;
	if(!he) return false;
	do {
		if(v == he->vertex) return true;
		he = he->next;
	} while(he != l->half_edge);
	return false;
}

void Mesher::AddLoop(int f, Loop *l1) {
	Loop *l0 = face_[f]->loop;

	l1->next = l0->next;
	l1->prev = l0;
	l0->next->prev = l1;
	l0->next = l1;
}

void Mesher::SetLoop(HalfEdge *he, Loop *l) {
	do {
		he->loop = l;
		he = he->next;
	} while(he != l->half_edge);
}

void Mesher::Mvfs(glm::vec3 p) {
	solid_.push_back(new Solid);
	face_.push_back(new Face);
	vertex_.push_back(new Vertex(p));
	Solid *s_p = solid_.back();
	int s = solid_.size() - 1;
	Face *f_p = face_.back();
	int f = face_.size() - 1;

	Loop *l = new Loop(f);

	s_p->face = f;

	f_p->solid = s;
	f_p->loop = l;

	l->prev = l->next = l;
	l->face = f;
	l->half_edge = nullptr;
}

int Mesher::Mve(glm::vec3 p, int v0, Loop *l) {
	edge_.push_back(new Edge);
	vertex_.push_back(new Vertex(p));
	Edge *e_p = edge_.back();
	int e = edge_.size() - 1;
	int v1 = vertex_.size() - 1;

	HalfEdge *he0 = new HalfEdge(e, v0), *he1 = new HalfEdge(e, v1);

	e_p->half_edge[0] = he0;
	e_p->half_edge[1] = he1;

	he0->loop = he1->loop = l;
	he0->twin = he1;
	he1->twin = he0;

	he0->next = he1;
	he1->prev = he0;
	if(l->half_edge == nullptr) {
		he1->next = he0;
		he0->prev = he1;
		l->half_edge = he0;
	} else {
		HalfEdge *he = l->half_edge;
		while(he->next->vertex != v0) he = he->next;
		he1->next = he->next;
		he->next->prev = he1;
		he0->prev = he;
		he->next = he0;
	}

	return v1;
}

int Mesher::Mve(glm::vec3 p, int v0, int f) {
	Loop *l = face_[f]->loop;
	do {
		if(InLoop(v0, l)) break;
		l = l->next;
	} while(l != face_[f]->loop);
	return Mve(p, v0, l);
}

void Mesher::Mef(int v0, int v1, Loop *l0) {
	face_.push_back(new Face);
	edge_.push_back(new Edge);
	Face *f1_p = face_.back();
	int f1 = face_.size() - 1;
	Edge *e_p = edge_.back();
	int e = edge_.size() - 1;

	Loop *l1 = new Loop(f1);
	HalfEdge *he0 = new HalfEdge(e, v0), *he1 = new HalfEdge(e, v1);

	f1_p->solid = face_[l0->face]->solid;
	f1_p->loop = l1;

	e_p->half_edge[0] = he0;
	e_p->half_edge[1] = he1;

	he0->loop = l0;
	he1->loop = l1;
	he0->twin = he1;
	he1->twin = he0;

	HalfEdge *he;

	he = l0->half_edge;
	while(he->next->vertex != v0) he = he->next;
	he1->next = he->next;
	he->next->prev = he1;
	he0->prev = he;
	he->next = he0;

	he = he1->next;
	while(he->next->vertex != v1) he = he->next;
	he0->next = he->next;
	he->next->prev = he0;
	he1->prev = he;
	he->next = he1;

	l0->half_edge = he0;
	l1->half_edge = he1;

	SetLoop(he1, l1);
}

void Mesher::Mef(int v0, int v1, int f0) {
	Loop *l0 = face_[f0]->loop;
	do {
		if(InLoop(v0, l0)) break;
		l0 = l0->next;
	} while(l0 != face_[f0]->loop);
	Mef(v0, v1, l0);
}

void Mesher::KeMr(int e, int f) {
	Loop *l1 = new Loop(f);

	Loop *l0 = face_[f]->loop;

	AddLoop(f, l1);

	HalfEdge *he0 = edge_[e]->half_edge[0], *he1 = edge_[e]->half_edge[1];

	he0->prev->next = he1->next;
	he1->next->prev = he0->prev;

	he1->prev->next = he0->next;
	he0->next->prev = he1->prev;

	l0->half_edge = he0->prev; // outer loop
	l1->half_edge = he1->prev; // inner loop

	SetLoop(he1->prev, l1);

	delete edge_[e]->half_edge[0];
	delete edge_[e]->half_edge[1];
	delete edge_[e]; // edge memory deleted, pointer still in edge list (vector)
	edge_[e] = nullptr;
}

void Mesher::KfMrh(int f0, int f1) {
	AddLoop(f0, face_[f1]->loop);
	delete face_[f1];
	face_[f1] = nullptr;
}

void Mesher::Sweep(int f, glm::dvec3 d, double t) {
	d = glm::normalize(d) * t;
	Loop *l = face_[f]->loop;
	int f_outer = l->half_edge->twin->loop->face;
	do {
		HalfEdge *he = l->half_edge;
		Loop *l_twin = he->twin->loop;
		int v_init = Mve(vertex_[he->vertex]->position + d, he->vertex, l_twin);
		int v_prev = v_init;
		he = he->next;
		do {
			int v_next = Mve(vertex_[he->vertex]->position + d, he->vertex, l_twin);
			Mef(v_next, v_prev, l_twin);
			v_prev = v_next;
			he = he->next;
		} while(he != l->half_edge);
		Mef(v_init, v_prev, l_twin);

		if(l != face_[f]->loop) KfMrh(f_outer, l_twin->face);
		l = l->next;
	} while(l != face_[f]->loop);
}

void Mesher::Build() {
	for(unsigned int i = 0; i < operator_.size(); i++) {
		switch(operator_[i]->op) {
			case Euler_Mvfs: {
				auto o = static_cast<EulerMvfs*>(operator_[i]);
				Mvfs(o->p);
				break;
			}
			case Euler_Mve: {
				auto o = static_cast<EulerMve*>(operator_[i]);
				Mve(o->p, o->v0, o->f);
				break;
			}
			case Euler_Mef: {
				auto o = static_cast<EulerMef*>(operator_[i]);
				Mef(o->v0, o->v1, o->f0);
				break;
			}
			case Euler_KeMr: {
				auto o = static_cast<EulerKeMr*>(operator_[i]);
				KeMr(o->e, o->f);
				break;
			}
			case Euler_KfMrh: {
				auto o = static_cast<EulerKfMrh*>(operator_[i]);
				KfMrh(o->f0, o->f1);
				break;
			}
			case Op_Sweep: {
				auto o = static_cast<OpSweep*>(operator_[i]);
				Sweep(o->f, o->d, o->t);
				break;
			}
		}
	}
}

GLenum primitive_type;
void TessBeginCallback(GLenum type) {
	primitive_type = type;
}

std::vector<glm::vec3> vertex_temp;
void TessVertexCallback(void *data) {
	double *vertex = (GLdouble*)data;
	vertex_temp.push_back(glm::vec3(vertex[0], vertex[1], vertex[2]));
}

std::vector<glm::vec3> triangle_vertex;
void TessEndCallback() {
	if(primitive_type == GL_TRIANGLE_FAN) {
		for(unsigned int i = 1; i < vertex_temp.size() - 1; i++) {
			triangle_vertex.push_back(vertex_temp[0]);
			triangle_vertex.push_back(vertex_temp[i]);
			triangle_vertex.push_back(vertex_temp[i + 1]);
		}
	} else if(primitive_type == GL_TRIANGLE_STRIP) {
		triangle_vertex.push_back(vertex_temp[0]);
		triangle_vertex.push_back(vertex_temp[1]);
		triangle_vertex.push_back(vertex_temp[2]);
		glm::vec3 v = glm::cross(
			vertex_temp[2] - vertex_temp[1],
			vertex_temp[1] - vertex_temp[0]);
		for(unsigned int i = 1; i < vertex_temp.size() - 2; i++)
			if(glm::dot(v, glm::cross(
				vertex_temp[i + 2] - vertex_temp[i + 1],
				vertex_temp[i + 1] - vertex_temp[i + 0])) > 0) {
				triangle_vertex.push_back(vertex_temp[i + 0]);
				triangle_vertex.push_back(vertex_temp[i + 1]);
				triangle_vertex.push_back(vertex_temp[i + 2]);
			} else {
				triangle_vertex.push_back(vertex_temp[i + 0]);
				triangle_vertex.push_back(vertex_temp[i + 2]);
				triangle_vertex.push_back(vertex_temp[i + 1]);
			}
	} else {
		triangle_vertex += vertex_temp;
	}
	vertex_temp.clear();
}

std::vector<glm::vec3> Mesher::TriangulateFace(int f) {
	triangle_vertex.clear();
	if(!face_[f]) return std::move(triangle_vertex);

	GLUtesselator *tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN, (void(CALLBACK*)())TessBeginCallback);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void(CALLBACK*)())TessVertexCallback);
	gluTessCallback(tess, GLU_TESS_END, (void(CALLBACK*)())TessEndCallback);
	gluTessBeginPolygon(tess, 0);
	Loop *l = face_[f]->loop;
	do {
		gluTessBeginContour(tess);
		HalfEdge *he = l->half_edge;
		do {
			double *p = (double*)&vertex_[he->vertex]->position;
			gluTessVertex(tess, p, p);
			he = he->next;
		} while(he != l->half_edge);
		l = l->next;
		gluTessEndContour(tess);
	} while(l != face_[f]->loop);
	gluTessEndPolygon(tess);
	gluDeleteTess(tess);

	return std::move(triangle_vertex);
}

std::vector<glm::vec3> &Mesher::Triangulate() {
	triangel_vertex_.clear();
	for(unsigned int i = 0; i < face_.size(); i++)
		triangel_vertex_ += TriangulateFace(i);

	triangel_normal_.resize(triangel_vertex_.size());
	for(unsigned int i = 0; i < triangel_normal_.size(); i += 3)
		triangel_normal_[i + 0] =
		triangel_normal_[i + 1] =
		triangel_normal_[i + 2] = glm::normalize(glm::cross(
			triangel_vertex_[i + 1] - triangel_vertex_[i + 0],
			triangel_vertex_[i + 2] - triangel_vertex_[i + 1]));

	return triangel_vertex_;
}

void Mesher::PrintFace(int f_i) {
	if(!face_[f_i]) return;
	Face *f_p = face_[f_i];
	Loop *l = f_p->loop;
	int l_i = 0;
	do {
		HalfEdge *he = l->half_edge;
		int he_i = 0;
		do {
			int v_i = he->vertex;
			glm::dvec3 &v = vertex_[v_i]->position;
			printf("f%-2d l%-2d he%-2d v%-2d: %g %g %g\n", f_i, l_i, he_i, v_i, v.x, v.y, v.z);
			he = he->next;
			he_i++;
		} while(he != l->half_edge);
		l = l->next;
		l_i++;
	} while(l != f_p->loop);
}

void Mesher::Print() {
	for(unsigned int f_i = 0; f_i < face_.size(); f_i++)
		PrintFace(f_i);
}

void Mesher::PrintLoop(Loop *l) {
	HalfEdge *he = l->half_edge;
	int he_i = 0;
	do {
		int v_i = he->vertex;
		glm::dvec3 &v = vertex_[v_i]->position;
		printf("he%-2d v%-2d: %g %g %g\n", he_i, v_i, v.x, v.y, v.z);
		he = he->next;
		he_i++;
	} while(he != l->half_edge);
	l = l->next;
}

}
