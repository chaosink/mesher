#include "Mesher.hpp"

#include <fstream>
#include <string>
#include <iostream>
using namespace std;

namespace mesher {

void Mesher::LoadOperator(const char *file) {
	std::ifstream ifs(file);
	std::string op;
	float x, y, z, t;
	int n0, n1, n2;

	while(ifs >> op) {
		if(op[0] == '#') {
			ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		} else if(op == "Mvfs") {
			ifs >> x >> y >> z;
			operator_.push_back(new EulerMvfs(x, y, z));
		} else if(op == "Mve") {
			ifs >> x >> y >> z >> n0 >> n1;
			operator_.push_back(new EulerMve(x, y, z, n0, n1));
		} else if(op == "Mef") {
			ifs >> n0 >> n1 >> n2;
			operator_.push_back(new EulerMef(n0, n1, n2));
		} else if(op == "KeMr") {
			ifs >> n0 >> n1;
			operator_.push_back(new EulerKeMr(n0, n1));
		} else if(op == "KfMrh") {

		} else if(op == "Sweep") {
			ifs >> x >> y >> z >> t;
			operator_.push_back(new OpSweep(x, y, z, t));
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
				fprintf(f, "Mvfs %f %f %f\n", o->p.x, o->p.y, o->p.z);
				break;
			}
			case Euler_Mve: {
				auto o = static_cast<EulerMve*>(op);
				fprintf(f, "Mve %f %f %f %d %d\n", o->p.x, o->p.y, o->p.z, o->v0, o->f);
				break;}
			case Euler_Mef: {
				auto o = static_cast<EulerMef*>(op);
				fprintf(f, "Mef %d %d %d\n", o->v0, o->v1, o->f0);
				break;
			}
			case Euler_KeMr: {
				auto o = static_cast<EulerKeMr*>(op);
				fprintf(f, "KeMr %d %d\n", o->e, o->f);
				break;
			}
			case Euler_KfMrh: {
				auto o = static_cast<EulerKfMrh*>(op);
				fprintf(f, "KfMrh %f %f %f\n", o->p.x, o->p.y, o->p.z);
				break;
			}
			case Op_Sweep: {
				auto o = static_cast<OpSweep*>(op);
				fprintf(f, "Sweep %f %f %f %f\n", o->d.x, o->d.y, o->d.z, o->t);
				break;
			}
		}
	}

	fclose(f);
}

bool Mesher::InLoop(Vertex *v, Loop *l) {
	HalfEdge *he = l->half_edge;
	if(!he) return false;
	do {
		if(v == he->vertex) return true;
		he = he->next;
	} while(he != l->half_edge);
	return false;
}

void Mesher::Mvfs(glm::vec3 p) {
	solid_.push_back(new Solid);
	face_.push_back(new Face);
	vertex_.push_back(new Vertex(p));
	Solid *s = solid_.back();
	Face *f = face_.back();

	Loop *l = new Loop(f);

	s->face = f;

	f->solid = s;
	f->loop = l;

	l->prev = l->next = l;
	l->face = f;
	l->half_edge = nullptr;
}

Vertex *Mesher::Mve(glm::vec3 p, Vertex *v0, Loop *l) {
	edge_.push_back(new Edge);
	vertex_.push_back(new Vertex(p));
	Edge *e = edge_.back();
	Vertex *v1 = vertex_.back();

	HalfEdge *he0 = new HalfEdge(e, v0), *he1 = new HalfEdge(e, v1);

	e->half_edge[0] = he0;
	e->half_edge[1] = he1;

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

Vertex *Mesher::Mve(glm::vec3 p, Vertex *v0, Face *f) {
	Loop *l = f->loop;
	do {
		if(InLoop(v0, l)) break;
		l = l->next;
	} while(l != f->loop);
	Mve(p, v0, l);
}

void Mesher::Mef(Vertex *v0, Vertex *v1, Loop *l0) {
	face_.push_back(new Face);
	edge_.push_back(new Edge);
	Face *f1 = face_.back();
	Edge *e = edge_.back();

	Loop *l1 = new Loop(f1);
	HalfEdge *he0 = new HalfEdge(e, v0), *he1 = new HalfEdge(e, v1);

	f1->solid = l0->face->solid;
	f1->loop = l1;

	e->half_edge[0] = he0;
	e->half_edge[1] = he1;

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
}

void Mesher::Mef(Vertex *v0, Vertex *v1, Face *f0) {
	Loop *l0 = f0->loop;
	do {
		if(InLoop(v0, l0)) break;
		l0 = l0->next;
	} while(l0 != f0->loop);
	Mef(v0, v1, l0);
}

void Mesher::KeMr(Edge *&e, Face *f) {
	Loop *l1 = new Loop(f);

	Loop *l0 = f->loop;

	l1->next = l0->next;
	l1->prev = l0;
	l0->next->prev = l1;
	l0->next = l1;

	HalfEdge *he0 = e->half_edge[0], *he1 = e->half_edge[1];

	he0->prev->next = he1->next;
	he1->next->prev = he0->prev;

	he1->prev->next = he0->next;
	he0->next->prev = he1->prev;

	l0->half_edge = he0->prev; // outer loop
	l1->half_edge = he1->prev; // inner loop

	delete e->half_edge[0];
	delete e->half_edge[1];
	delete e; // edge memory deleted, pointer still in edge list (vector)
	// e = nullptr;
}

void Mesher::KfMrh(Face *&f) {
	delete f;
	// f = nullptr;
}

void Mesher::Sweep(Face *f, glm::vec3 d, float t) {
	d = glm::normalize(d) * t;
	Loop *l = f->loop;
	do {
		HalfEdge *he = l->half_edge;
		Vertex *v_init = Mve(he->vertex->position + d, he->vertex, l);
		Vertex *v_prev = v_init;
		he = he->next;
		do {
			Vertex *v_next = Mve(he->vertex->position + d, he->vertex, l);
			Mef(v_prev, v_next,l);
			v_prev = v_next;
			he = he->next;
		} while(he != l->half_edge);
		Mef(v_prev, v_init, l);
		if(l != f->loop) KfMrh(l->face);
		l = l->next;
	} while(l != f->loop);
}

void Mesher::Build() {
	for(unsigned int i = 0; i < operator_.size(); i++) {
		// printf("o%02d\n", i);
		switch(operator_[i]->op) {
			case Euler_Mvfs: {
				auto e = static_cast<EulerMvfs*>(operator_[i]);
				Mvfs(e->p);
				break;
			}
			case Euler_Mve: {
				auto e = static_cast<EulerMve*>(operator_[i]);
				Mve(e->p, vertex_[e->v0], face_[e->f]);
				break;
			}
			case Euler_Mef: {
				auto e = static_cast<EulerMef*>(operator_[i]);
				Mef(vertex_[e->v0], vertex_[e->v1], face_[e->f0]);
				break;
			}
			case Euler_KeMr: {
				auto e = static_cast<EulerKeMr*>(operator_[i]);
				KeMr(edge_[e->e], face_[e->f]);
				break;
			}
			case Euler_KfMrh: {
				// auto e = static_cast<EulerKfMrh*>(operator_[i]);
				// KfMrh(e->p);
				break;
			}
		}
	}
}

void Mesher::PrintFace() {
	Face *face;
	for(unsigned int f_i = 0; f_i < face_.size(); f_i++) {
		if(!face_[f_i]) continue;
		face = face_[f_i];
		Loop *l = face->loop;
		int l_i = 0;
		do {
			HalfEdge *he = l->half_edge;
			int he_i = 0;
			do {
				glm::vec3 &v =  he->vertex->position;
				printf("f%d l%d he%d: %g %g %g\n", f_i, l_i, he_i, v.x, v.y, v.z);
				he = he->next;
				he_i++;
			} while(he != l->half_edge);
			l = l->next;
			l_i++;
		} while(l != face->loop);
	}
}

}
