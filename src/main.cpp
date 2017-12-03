#include <iostream>
#include <typeinfo>
using namespace std;

#include "Mesher.hpp"
using namespace mesher;

int main(int argc, char *argv[]) {
	if(argc < 2) {
		printf("Usage: mesher model_file\n");
		return 0;
	}

	Mesher mesh;
	mesh.LoadEuler(argv[1]);
	mesh.SaveEuler("save.euler");
	mesh.Build();
	mesh.SaveFace();

	return 0;
}
