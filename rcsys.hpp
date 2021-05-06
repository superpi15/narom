#ifndef RCSYS_T
#define RCSYS_T

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <assert.h>

class RcDev_t{
public:
	typedef enum {
		Undef
		, Node
		, Sie
		, Far
		, Hen
		, Vlt
		, Amp
		, Gnd
	} Type_t;
	int id;
	int vid; // nodal vector index
	std::string name;
	std::string rval; // string version attribute
	std::multimap<int,int> conn; // < dest, bridge id>
	Type_t type;
	RcDev_t(int nid):id(nid),type(Undef),vid(-1){}
	const char * typeName(){
		switch(type){
			case Undef: return "Undef";
			case  Node: return "Node";
			case   Sie: return "Conductor";
			case   Far: return "Capacitor";
			case   Hen: return "Inductor";
			case   Vlt: return "V source";
			case   Amp: return "I source";
			case   Gnd: return "Ground";
			default: return "unknown";
		}
	}
	bool isExcitation() const {
		return Vlt == type || Amp == type;
	}
	bool isDevice() const { return Node != type && Gnd != type; }
	bool needCurr() const { return Vlt == type || Hen == type; }
	void print(){
		printf("%12s %12s id= %4d attr= %12.5s (Type = %s)\n", typeName(), name.c_str(), id, rval.c_str(), isDevice()? "device": "excitation");
	}
};

class RcRel_t {
public:
	int nid1, nid2; // nid1 ~> nid2 (indicates flow if mounted with excitation device, )
	int did;        // type id of a device 
	int rid;        // id of this relation 
	int vid;        // nodal vector index 
	RcRel_t( int nrid ):rid(nrid),nid1(-1),nid2(-1),did(-1),vid(-1){}
};

class RcEnt_t {
public:
	RcEnt_t():fdir(0){}
	std::vector<int> addDev, subDev; // index to excitation devices 
	int fdir; // flow direction +/- 1
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
	~RcSys_t(){
		clearPos2Ent();
	}

	int nObj, nRel;
	std::map<std::string, int> name2id;
	std::vector<RcDev_t> vDev;
	std::vector<RcRel_t> vRel;

	std::vector<int> vNodalVolt;
	std::vector<int> vNodalCurr;
	std::vector<RcEnt_t> vExcitation;

	std::string sMatrix, gMatrix, eVector;
	std::map< std::pair<int,int>, RcEnt_t* > pos2sus; // susceptance matrix 
	std::map< std::pair<int,int>, RcEnt_t* > pos2ent; // conductance matrix 
	void entMountDev( int nid1, int nid2, int did, bool pol = false );
	void susMountDev( int nid1, int nid2, int did, bool pol = false );
	void entAddFlow( int nid, int rvid, bool pol = false );
	void clearPos2Ent(){
		std::map< std::pair<int,int>, RcEnt_t* >::iterator itr;
		for( itr = pos2ent.begin(); itr != pos2ent.end(); itr ++ )
			delete itr->second;
		pos2ent.clear();

		for( itr = pos2sus.begin(); itr != pos2sus.end(); itr ++ )
			delete itr->second;
		pos2sus.clear();
	}

	void printEnt( RcEnt_t * pEnt, bool pol = false );
	void writeEnt( std::ostream&, RcEnt_t * pEnt, bool pol = false );
	void printPos2Ent();
	void writePos2Ent( std::ostream& );
	int parse( char * rcfileIn );
	int mna();
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
		if( RcDev_t::Node != vDev[ret].type && RcDev_t::Gnd != vDev[ret].type )
			return -1;
		return ret;
	}

	int getDevice( std::string& name ){
		if( name2id.find(name) == name2id.end() )
			return -1;
		int id = name2id[name];
		if( !vDev[id].isDevice() )
			return -1;
		return id;
	}
	void addRelation( int nid1, int nid2, int did ){
		assert( 0 <= nid1 );
		assert( 0 <= nid2 );
		assert( 0 <=  did );
		assert( nid1 < vDev.size() );
		assert( nid2 < vDev.size() );
		assert(  did < vDev.size() );
		assert( vDev[nid1].type == RcDev_t::Node || vDev[nid1].type == RcDev_t::Gnd );
		assert( vDev[nid2].type == RcDev_t::Node || vDev[nid2].type == RcDev_t::Gnd );
		assert( vDev[ did].type != RcDev_t::Node );
		vRel.push_back( RcRel_t(nRel++) );
		vRel.back().nid1 = nid1;
		vRel.back().nid2 = nid2;
		vRel.back(). did =  did;
		vDev[nid1].conn.insert( std::pair<int,int>( nid2, vRel.back().rid ) );
		vDev[nid2].conn.insert( std::pair<int,int>( nid1, vRel.back().rid ) );
	}
	void printRel( int rid ){
		int nid1, nid2, did;
		nid1 = vRel[rid].nid1;
		nid2 = vRel[rid].nid2;
		did = vRel[rid]. did;
		printf("\t");
		if( vDev[did].isExcitation() ){
			if( RcDev_t::Vlt == vDev[did].type ){
				printf("%s %3s %s %3s %s"
					, vDev[nid1].name.c_str()
					, "(+ "
					, vDev[did].name.c_str()
					, " -)"
					, vDev[nid2].name.c_str() );
			} else {
				printf("%s %3s %s %3s %s"
					, vDev[nid1].name.c_str()
					, "-->"
					, vDev[did].name.c_str()
					, "-->"
					, vDev[nid2].name.c_str() );
			}
			
		} else {
			printf("%s %3s %s %3s %s"
				, vDev[nid1].name.c_str()
				, "---"
				, vDev[did].name.c_str()
				, "---"
				, vDev[nid2].name.c_str() );
		}
		printf("\n");
	}
};

#endif