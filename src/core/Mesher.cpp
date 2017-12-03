#include "Mesher.hpp"

#include <fstream>
#include <string>
#include <iostream>

namespace mesher {

void Mesher::LoadEuler(const char *file) {
	std::ifstream ifs(file);
	std::string euler;
	float x, y, z;
	int n0, n1, n2;

	while(ifs >> euler) {
		if(euler[0] == '#')
			ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if(euler == "Mvfs") {
			ifs >> x >> y >> z;
			euler_.push_back(new EulerMvfs(x, y, z));
		} else if(euler == "Mve") {
			ifs >> x >> y >> z;
			ifs >> n0 >> n1;
			euler_.push_back(new EulerMve(x, y, z, n0, n1));
		} else if(euler == "Mef") {
			ifs >> n0 >> n1 >> n2;
			euler_.push_back(new EulerMef(n0, n1, n2));
		} else if(euler == "KeMr") {
			ifs >> n0 >> n1;
			euler_.push_back(new EulerKeMr(n0, n1));
		} else if(euler == "KfMrh") {

		}
	}

	ifs.close();
}

void Mesher::SaveEuler(const char *file) {
	FILE *f = fopen(file, "w");

	for(auto euler: euler_) {
		switch(euler->name) {
			case Euler_Mvfs: {
				auto e = static_cast<EulerMvfs*>(euler);
				fprintf(f, "Mvfs %f %f %f\n", e->p.x, e->p.y, e->p.z);
				break;
			}
			case Euler_Mve: {
				auto e = static_cast<EulerMve*>(euler);
				fprintf(f, "Mve %f %f %f %d %d\n", e->p.x, e->p.y, e->p.z, e->v0, e->f);
				break;}
			case Euler_Mef: {
				auto e = static_cast<EulerMef*>(euler);
				fprintf(f, "Mef %d %d %d\n", e->v0, e->v1, e->f0);
				break;
			}
			case Euler_KeMr: {
				auto e = static_cast<EulerKeMr*>(euler);
				fprintf(f, "KeMr %d %d\n", e->e, e->f);
				break;
			}
			case Euler_KfMrh: {
				auto e = static_cast<EulerKfMrh*>(euler);
				fprintf(f, "KfMrh %f %f %f\n", e->p.x, e->p.y, e->p.z);
				break;
			}
		}
	}

	fclose(f);
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

void Mesher::Mve(glm::vec3 p, Vertex *v0, Face *f) {
	edge_.push_back(new Edge);
	vertex_.push_back(new Vertex(p));
	Edge *e = edge_.back();
	Vertex *v1 = vertex_.back();

	HalfEdge *he0 = new HalfEdge(e, v0), *he1 = new HalfEdge(e, v1);

	Loop *l = f->loop;

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
}

void Mesher::Mef(Vertex *v0, Vertex *v1, Face *f0) {
	face_.push_back(new Face);
	edge_.push_back(new Edge);
	Face *f1 = face_.back();
	Edge *e = edge_.back();

	Loop *l1 = new Loop(f1);
	HalfEdge *he0 = new HalfEdge(e, v0), *he1 = new HalfEdge(e, v1);

	Loop *l0 = f0->loop;

	f1->solid = f0->solid;
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

void Mesher::KeMr(Edge *e, Face *f) {
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

	delete e; // edge memory deleted, pointer still in edge list
}

void Mesher::KfMrh(Face *f) {

}

void Mesher::Build() {
	for(auto euler: euler_) {
		switch(euler->name) {
			case Euler_Mvfs: {
				auto e = static_cast<EulerMvfs*>(euler);
				Mvfs(e->p);
				break;
			}
			case Euler_Mve: {
				auto e = static_cast<EulerMve*>(euler);
				Mve(e->p, vertex_[e->v0], face_[e->f]);
				break;}
			case Euler_Mef: {
				auto e = static_cast<EulerMef*>(euler);
				Mef(vertex_[e->v0], vertex_[e->v1], face_[e->f0]);
				break;
			}
			case Euler_KeMr: {
				auto e = static_cast<EulerKeMr*>(euler);
				KeMr(edge_[e->e], face_[e->f]);
				break;
			}
			case Euler_KfMrh: {
				// auto e = static_cast<EulerKfMrh*>(euler);
				// KfMrh(e->p);
				break;
			}
		}
	}
}

void Mesher::SaveFace() {
	for(auto face: face_) {
		Loop *l = face->loop;
		do {
			HalfEdge *he = l->half_edge;
			do {
				glm::vec3 &v =  he->vertex->position;
				std::cout << v.x << " " << v.y << " " << v.z << std::endl;
				he = he->next;
			} while(he != l->half_edge);
			l = l->next;
		} while(l != face->loop);
	}
}

}
