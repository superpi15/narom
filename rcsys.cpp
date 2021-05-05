#include "rcsys.hpp"

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
			int nid0, nid1, did;

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

			if( -1 == (did = getDevice(w[2])) ){
				printf("redefine symbol at line %d: \'%s\'\n", nLines, w[2].c_str());
				vDev[name2id[w[2]]].print();
				return 0;
			}
			addRelation( nid0, nid1, did );
		}
	}
	rcin.close();

	for(int i = 0; i < vRel.size(); i ++)
		printRel(i);
	return 1;
}
