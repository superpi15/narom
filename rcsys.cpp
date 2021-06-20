#include "rcsys.hpp"

int RcSys_t::parse( char * rcfileIn ){
	int nErrors = 0, nLines = 0;
	printf("read from \'%s\' ... \n", rcfileIn);
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

			// matrix naming 
			if( w[0].size() && '.' == w[0][0] )
			{
				if( ".susceptance" == w[0] ){
					sMatrix = w[1];
				} else
				if( ".conductance" == w[0] ){
					gMatrix = w[1];
				} else
				if( ".excitation" == w[0] ){
					eVector = w[1];
				} else {
					nErrors ++ ;
					printf("syntax error at line %d: \'%s\'\n", nLines, line.c_str());
					return 0;
				}
				continue;
			}

			if( w[1].size() < 2 || (w[1].back() != 's' && w[1].back() != 'f' && w[1].back() != 'v' && w[1].back() != 'a' && w[1].back() != 'h') ){
				nErrors ++ ;
				printf("syntax error at line %d: \'%s\'\n", nLines, line.c_str());
				return 0;
			}

			vDev.push_back( RcDev_t(nObj++) );
			name2id[w[0]] = vDev.back().id;
			vDev.back().name = w[0];
			switch( w[1].back() ){
				case 's': vDev.back().type = RcDev_t::Sie; break;
				case 'f': vDev.back().type = RcDev_t::Far; break;
				case 'v': vDev.back().type = RcDev_t::Vlt; break;
				case 'a': vDev.back().type = RcDev_t::Amp; break;
				case 'h': vDev.back().type = RcDev_t::Hen; break;
			}
			w[1].back() = ' ';
			vDev.back().rval = w[1];
			//vDev.back().attr = atof(w[1].c_str());
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
			if( nid0 == nid1 ){
				printf("invalid at line %d: \'%s\'\n", nLines, line.c_str());
				return 0;
			}
			addRelation( nid0, nid1, did );
		}
	}
	rcin.close();
	printf("netlist ... \n");
	for(int i = 0; i < vRel.size(); i ++)
		printRel(i);
	return 1;
}


int RcSys_t::mna(){
	printf("building mna ... \n");
	const std::string ndPrefix = "nd"; // node name prefix 
	int nNodalIdx = 0;
	vNodalVolt.clear();
	vNodalCurr.clear();
	vExcitation.clear();
	clearPos2Ent();

	for(int i = 0; i < vDev.size(); i ++){
		if( RcDev_t::Node == vDev[i].type ){
			vNodalVolt.push_back( vDev[i].id );
			printf("%3d: %s\n", nNodalIdx, vDev[i].name.c_str());
			vDev[i].vid = nNodalIdx ++ ;
		} else 
			vDev[i].vid = -1;
	}

	for(int i = 0; i < vRel.size(); i ++){
		if( -1 != vRel[i].did && vDev[vRel[i].did].needCurr() ){
			vNodalCurr.push_back( vRel[i].rid );
			printf("%3d: ", nNodalIdx);
			printRel(vRel[i].rid);
			vRel[i].vid = nNodalIdx ++ ;
		} else
			vRel[i].vid = -1;
	}
	vExcitation.resize( nNodalIdx );

	if( nNodalIdx ){
		ndNames.clear();
		ndNames.resize( nNodalIdx );
		for(int i = 0; i < vDev.size(); i ++){
			if( -1 == vDev[i].vid )
				continue;
			char buff[256];
			sprintf(buff, "%s%d", ndPrefix.c_str(), vDev[i].vid );
			ndNames[ vDev[i].vid ] = buff;
		}
	}

	for(int i = 0; i < vRel.size(); i ++){
		int nid1, nid2, did;
		nid1 = vRel[i].nid1;
		nid2 = vRel[i].nid2;
		std::pair<int,int> pos;
		did = vRel[i].did;

		// conductor 
		if( RcDev_t::Sie == vDev[did].type ){
			if( 0 < nid1 )
				entMountDev(nid1, nid1, did);
			if( 0 < nid2 )
				entMountDev(nid2, nid2, did);
			if( 0 < nid1 && 0 < nid2 )
				entMountDev(nid1, nid2, did, 1);
		} else
		if( RcDev_t::Vlt == vDev[did].type ){
			if( 0 < nid1 )
				entAddFlow(nid1, vRel[i].vid, 1); // flow out is the inversed flow in 
			if( 0 < nid2 )
				entAddFlow(nid2, vRel[i].vid, 0);
			vExcitation[ vRel[i].vid ].addDev.push_back(did);
		}
		else
		if( RcDev_t::Far == vDev[did].type ){ // capacitor 
			if( 0 < nid1 )
				susMountDev(nid1, nid1, did);
			if( 0 < nid2 )
				susMountDev(nid2, nid2, did);
			if( 0 < nid1 && 0 < nid2 )
				susMountDev(nid1, nid2, did, 1);
		} else
		if( RcDev_t::Amp == vDev[did].type ){
			if( 0 < nid1 )
				vExcitation[ vDev[nid1].vid ].addDev.push_back(did);
			if( 0 < nid2 )
				vExcitation[ vDev[nid2].vid ].subDev.push_back(did);
		} else
		if( RcDev_t::Hen == vDev[did].type ){
			if( 0 < nid1 )
				entAddFlow(nid1, vRel[i].vid, 1); // flow out is the inversed flow in 
			if( 0 < nid2 )
				entAddFlow(nid2, vRel[i].vid, 0);
			susMountInductor(vRel[i].vid, did);
		}
	}
}

void RcSys_t::writeEnt( std::ostream& ostr, RcEnt_t * pEnt, bool pol ){
	std::vector<int>& addDev = pol? pEnt->subDev: pEnt->addDev;
	std::vector<int>& subDev = pol? pEnt->addDev: pEnt->subDev;
	int fdir = pEnt->fdir;
	int nPrinted = 0;

	if( fdir ){
		ostr << (pol? -fdir: fdir);
		nPrinted ++ ;
	}

	for( int i = 0; i < addDev.size(); i ++, nPrinted ++ )
		ostr << (nPrinted? "+": " ") << vDev[addDev[i]].name;

	for( int i = 0; i < subDev.size(); i ++, nPrinted ++ )
		ostr << "-" << vDev[subDev[i]].name;

	if( 0 == nPrinted )
		ostr << "0";
}

void RcSys_t::printEnt( RcEnt_t * pEnt, bool pol ){
	std::ostringstream ostr;
	writeEnt( ostr, pEnt, pol );
	printf(" %10s", ostr.str().c_str());
}

void RcSys_t::printPos2Ent(){
	int nNodalIdx = vNodalVolt.size() + vNodalCurr.size();

	printf("susceptance matrix: \n");
	for(int row = 0; row < nNodalIdx; row ++ ){
		for(int col = 0; col < nNodalIdx; col ++ ){
			std::pair<int,int> pos(row, col);
			if( row > col ) std::swap(pos.first, pos.second);
			if( pos2sus.end() != pos2sus.find(pos) )
				printEnt( pos2sus[pos], pos.second >= vNodalVolt.size() && row > col );
			else
				printf(" %10s", "0");
		}
		printf("\n");
	}

	printf("conductance matrix: \n");
	for(int row = 0; row < nNodalIdx; row ++ ){
		for(int col = 0; col < nNodalIdx; col ++ ){
			std::pair<int,int> pos(row, col);
			if( row > col ) std::swap(pos.first, pos.second);
			if( pos2ent.end() != pos2ent.find(pos) )
				printEnt( pos2ent[pos], pos.second >= vNodalVolt.size() && row > col );
			else
				printf(" %10s", "0");
		}
		printf("\n");
	}
	printf("excitation vector: \n");
	for(int i=0; i < vExcitation.size(); i ++){
		printEnt( &vExcitation[i] );
	}
	printf("\n");
}

void RcSys_t::writePos2Ent( std::ostream& ostr, std::string& rcfileOut ){
	ostr << "% This matlab code is generated by \'Narom - auto MNA\' \n\n";
	int ipad = 1; // index pad for 1-based indexing
	int nNodalIdx = vNodalVolt.size() + vNodalCurr.size();

	std::string funcName = rcfileOut;
	for(int i = 0; i < funcName.size(); i ++)
		if( '.' == funcName[i] ){
			funcName.resize(i);
			break;
		}

	ostr << "function ";
	if( sMatrix.empty() ) sMatrix = "smatrix";
	if( gMatrix.empty() ) gMatrix = "gmatrix";
	if( eVector.empty() ) eVector = "evector";
	if( ndNameVector.empty() ) ndNameVector = "ndName";


	ostr << "[" << sMatrix << "," << gMatrix << "," << eVector << "," << ndNameVector << "]";
	ostr << " = " << funcName << "()" << "\n";

	ostr << sMatrix << "=zeros("<< nNodalIdx << "," << nNodalIdx << ");\n";
	ostr << gMatrix << "=zeros("<< nNodalIdx << "," << nNodalIdx << ");\n";
	ostr << eVector << "=zeros("<< nNodalIdx << "," << 1 << ");\n";

	ostr << ndNameVector << " = [ ";
	for( int i = 0; i < ndNames.size(); i ++)
		ostr << "\"" << ndNames[i] <<"\" ";
	ostr << "];\n";

	for(int i = 0; i < vDev.size(); i ++){
		if( !vDev[i].isDevice() ) continue;
		ostr << vDev[i].name << "= " << vDev[i].rval << ";\n";
	}

	for(int row = 0; row < nNodalIdx; row ++ ){
		for(int col = 0; col < nNodalIdx; col ++ ){
			std::pair<int,int> pos(row, col);
			if( row > col ) std::swap(pos.first, pos.second);
			if( pos2sus.end() == pos2sus.find(pos) )
				continue;
			std::ostringstream buff;
			writeEnt( buff, pos2sus[pos], pos.second >= vNodalVolt.size() && row > col );
			ostr << sMatrix << "("<< row + ipad << "," << col + ipad << ")= " << buff.str() <<";\n";
		}
	}

	for(int row = 0; row < nNodalIdx; row ++ ){
		for(int col = 0; col < nNodalIdx; col ++ ){
			std::pair<int,int> pos(row, col);
			if( row > col ) std::swap(pos.first, pos.second);
			if( pos2ent.end() == pos2ent.find(pos) )
				continue;
			std::ostringstream buff;
			writeEnt( buff, pos2ent[pos], pos.second >= vNodalVolt.size() && row > col );
			ostr << gMatrix << "("<< row + ipad << "," << col + ipad << ")= " << buff.str() <<";\n";
		}
	}

	for(int i = 0; i < vExcitation.size(); i ++){
		std::ostringstream buff;
		writeEnt( buff, &vExcitation[i] );
		if( "0" == buff.str() )
			continue;
		ostr << eVector << "("<< i + ipad << ")= " << buff.str() <<";\n";
	}

	ostr << "end" << "\n"; // end of function 
}

void RcSys_t::entAddFlow( int nid, int rvid, bool pol ){
	int vid1, vid2;
	vid1 = vDev[nid].vid;
	vid2 = rvid;
	assert( vid1 < vid2 );
	assert( vNodalVolt.size() <= vid2 );
	std::pair<int,int> pos(vid1,vid2);
	assert( NULL == pos2ent[pos] );
	pos2ent[pos] = new RcEnt_t;
	pos2ent[pos]->fdir = pol? -1: 1;
}

void RcSys_t::susMountDev( int nid1, int nid2, int did, bool pol ){
	int row, col;
	row = vDev[nid1].vid;
	col = vDev[nid2].vid;
	if( row > col ) std::swap(row, col);
	std::pair<int,int> pos(row,col);
	if( NULL == pos2sus[pos] )
		pos2sus[pos] = new RcEnt_t;
	if( !pol )
		pos2sus[pos]->addDev.push_back(did);
	else
		pos2sus[pos]->subDev.push_back(did);
}

void RcSys_t::susMountInductor( int rvid, int did ){
	int vid = rvid;
	std::pair<int,int> pos(vid,vid);
	if( NULL == pos2sus[pos] )
		pos2sus[pos] = new RcEnt_t;
	pos2sus[pos]->addDev.push_back(did);
}

void RcSys_t::entMountDev( int nid1, int nid2, int did, bool pol ){
	int row, col;
	row = vDev[nid1].vid;
	col = vDev[nid2].vid;
	if( row > col ) std::swap(row, col);
	std::pair<int,int> pos(row,col);
	if( NULL == pos2ent[pos] )
		pos2ent[pos] = new RcEnt_t;
	if( !pol )
		pos2ent[pos]->addDev.push_back(did);
	else
		pos2ent[pos]->subDev.push_back(did);
}
