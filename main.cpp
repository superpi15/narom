#include "rcsys.hpp"

/*

r 1o      // define a device 
r 2o

n0 n3 2o
n2 n1 1f
n1 n4 r 
n3 n8 r
n6 n7 r2

 */

int main( int argc, char ** argv ){
	if( argc < 2 ){
		printf("./narom <rc in> [<rc out>]\n");
		return 0;
	}

	char * rcfileIn = argv[1];
	RcSys_t rcsys;

	if( !rcsys.parse( rcfileIn ) ){
		return 0;
	}
	rcsys.mna();
	rcsys.printPos2Ent();

	if( argc >= 3 ){
		char * rcfileOut = argv[2];
		std::ofstream ostr( rcfileOut );
		rcsys.writePos2Ent(ostr);
		ostr.close();
	}
	return 1;
}