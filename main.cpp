#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <assert.h>
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
		, Gnd
	} Type_t;
	int id;
	std::string name;
	std::multimap<int,int> conn; // < dest, bridge id>
	float attr;
	Type_t type;
	RcDev_t(int nid):id(nid),type(Undef),attr(0){}
	const char * typeName(){
		switch(type){
			case Undef: return "Undef";
			case  Node: return "Node";
			case   Ohm: return "Resistor";
			case   Far: return "Capacitor";
			case   Vlt: return "V source";
			case   Amp: return "I source";
			case   Gnd: return "Ground";
			default: return "unknown";
		}
	}
	void print(){
		printf("%12s %12s id= %4d attr= %12.5f\n", typeName(), name.c_str(), id, attr);
	}
};

class RcRel_t {
public:
	int nid1, nid2; // nid1 < nid2
	int did;        // type id of a device 
	int rid;        // id of this relation 
	RcRel_t( int nrid ):rid(nrid),nid1(-1),nid2(-1),did(-1){}
};

class RcSys_t {
public:
	RcSys_t():nObj(0), nRel(0){
		vDev.push_back( RcDev_t(nObj++) );
		name2id["gnd"] = vDev.back().id;
		vDev.back().name = "gnd";
		vDev.back().type = RcDev_t::Gnd;
		vDev.back().print();
	}

	int nObj, nRel;
	std::map<std::string, int> name2id;
	std::vector<RcDev_t> vDev;
	std::vector<RcRel_t> vRel;

	int parse( char * rcfileIn );
	int getNode( std::string& name ){
		if( name2id.find(name) == name2id.end() ){
			vDev.push_back( RcDev_t(nObj++) );
			name2id[name] = vDev.back().id;
			vDev.back().name = name;
			vDev.back().type = RcDev_t::Node;
			vDev.back().print();
			return vDev.back().id;
		}
		int id = name2id[name];
		assert(name == vDev[id].name);
		int ret = vDev[id].id;
		if( RcDev_t::Node != vDev[ret].type )
			return -1;
		return ret;
	}
	int addRelation( int nid1, int nid2, int did ){
		if( nid1 > nid2 ) std::swap(nid1, nid2);
		assert( 0 <= nid1 );
		assert( 0 <= nid2 );
		assert( 0 <=  did );
		assert( nid1 < vDev.size() );
		assert( nid2 < vDev.size() );
		assert(  did < vDev.size() );
		assert( vDev[nid1].type == RcDev_t::Node );
		assert( vDev[nid2].type == RcDev_t::Node );
		assert( vDev[ did].type != RcDev_t::Node );
		vRel.push_back( RcRel_t(nRel++) );
		vRel.back().nid1 = nid1;
		vRel.back().nid2 = nid2;
		vRel.back(). did =  did;
		vDev[nid1].conn.insert( std::pair<int,int>( nid2, vRel.back().rid ) );
		vDev[nid2].conn.insert( std::pair<int,int>( nid1, vRel.back().rid ) );
	}
};

int RcSys_t::parse( char * rcfileIn ){
	int nErrors = 0, nLines = 0;
	printf("read from %s ... \n", rcfileIn);
	std::ifstream rcin( rcfileIn );
	std::string line, w[4];

	while( std::getline( rcin, line ) ){
		nLines ++ ;
		if( line.empty() ) continue;

		std::istringstream istr(line);
		int nTerms = 0;
		for(; nTerms < 4 && (istr>>w[nTerms]); nTerms ++);

		if( nTerms > 3 || nTerms < 2 ){
			nErrors ++ ;
			printf("syntax error: \'%s\'\n", line.c_str());
			continue;
		}

		if( nTerms == 2 ){ // declare scope 
			if( name2id.end() != name2id.find(w[0]) ){
				nErrors ++ ;
				printf("redefine symbol: \'%s\'\n", line.c_str());
				return 0;
			}
			if( w[1].size() < 2 || (w[1].back() != 'o' && w[1].back() != 'f' && w[1].back() != 'v' && w[1].back() != 'a') ){
				nErrors ++ ;
				printf("syntax error: \'%s\'\n", line.c_str());
				return 0;
			}

			vDev.push_back( RcDev_t(nObj++) );
			name2id[w[0]] = vDev.back().id;
			vDev.back().name = w[0];
			switch( w[1].back() ){
				case 'o': vDev.back().type = RcDev_t::Ohm; break;
				case 'f': vDev.back().type = RcDev_t::Far; break;
				case 'v': vDev.back().type = RcDev_t::Vlt; break;
				case 'a': vDev.back().type = RcDev_t::Amp; break;
			}
			w[1].back() = ' ';
			vDev.back().attr = atof(w[1].c_str());
			vDev.back().print();
		} else
		if( nTerms == 3 ){ // nodal relation 
			int nid0, nid1;

			if( -1 == (nid0 = getNode(w[0])) ){
				printf("redefine symbol at line %d: \'%s\'\n", nLines, w[0].c_str());
				vDev[name2id[w[0]]].print();
				return 0;
			}

			if( -1 == (nid1 = getNode(w[1])) ){
				printf("redefine symbol at line %d: \'%s\'\n", nLines, w[1].c_str());
				vDev[name2id[w[1]]].print();
				return 0;
			}

		}
	}

	rcin.close();
	return 1;
}

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
	return 1;
}