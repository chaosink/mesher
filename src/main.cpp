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
	mesh.LoadOperator(argv[1]);
	mesh.SaveOperator("save.op");
	mesh.Build();
	mesh.PrintFace();

	return 0;
}
