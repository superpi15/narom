#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
/*

r 1o      // define a device 
r 2o

n0 n3 2o
n2 n1 1f
n1 n4 r 
n3 n8 r
n6 n7 r2

 */

class RcDev_t{
public:
	typedef enum {
		Undef
		, Node
		, Ohm
		, Far
		, Vlt
		, Amp
	} Type_t;
	int id;
	std::string name;
	float attr;
	Type_t type;
	RcDev_t(int nid):id(id),type(Undef),attr(0){}
	const char * typeName(){
		switch(type){
			case Undef: return "Undef";
			case  Node: return "Node";
			case   Ohm: return "Resistor";
			case   Far: return "Capacitor";
			case   Vlt: return "V source";
			case   Amp: return "I source";
		}
	}
	void print(){
		printf("%12s %12s %12.5f\n", typeName(), name.c_str(), attr);
	}
};

int main( int argc, char ** argv ){
	if( argc < 2 ){
		printf("./narom <rc in> [<rc out>]\n");
		return 0;
	}

	char * rcfileIn = argv[1];

	int nObj = 0;
	printf("read from %s ... \n", rcfileIn);
	std::ifstream rcin( rcfileIn );
	std::string line, w[4];
	std::map<std::string, int> name2id;
	std::vector<RcDev_t> vDev;

	while( std::getline( rcin, line ) ){
		if( line.empty() ) continue;

		std::istringstream istr(line);
		int nTerms = 0;
		for(; nTerms < 4 && (istr>>w[nTerms]); nTerms ++);

		if( nTerms > 3 || nTerms < 2 ){
			printf("syntax error: \'%s\'\n", line.c_str());
			continue;
		}

		if( nTerms == 2 ){
			if( name2id.end() != name2id.find(w[0]) ){
				printf("redefine symbol: \'%s\'\n", line.c_str());
				return 0;
			}
			if( w[1].size() < 2 || (w[1].back() != 'o' && w[1].back() != 'f') ){
				printf("syntax error: \'%s\'\n", line.c_str());
				return 0;
			}

			vDev.push_back( RcDev_t(nObj++) );
			name2id[w[0]] = vDev.back().id;
			vDev.back().name = w[0];
			vDev.back().type = w[1].back()=='o'? RcDev_t::Ohm: RcDev_t::Far;
			w[1].back() = ' ';
			vDev.back().attr = atof(w[1].c_str());
			vDev.back().print();
		}
	}

	rcin.close();
	return 1;
}