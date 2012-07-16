#pragma once
#include "pardisomatcreator.h"
#include "meshMetaInfo.h"
#include "meshOperation.h"
#include <algorithm>

//matrix needed to adapt harmonic matrix for natural boundary constraint
// squarematrix with nrVertices*2 rows.
class rot90Creator: public pardisoMatCreator
{

	meshMetaInfo * mesh;
	int nrVerts;

public:
	rot90Creator(meshMetaInfo & aMesh){
		mesh = & aMesh;
		nrVerts = mesh->getBasicMesh().getVertices().size();
	}

	float val(int i , int j){
		// i is the row, j the vertex
		if(i%nrVerts==j%nrVerts){
			return 0.f;
		}
		
		int fc1, fc2;
		tuple2i edge(i%nrVerts,j%nrVerts);
		meshOperation::getNbrFaces(edge,&fc1,&fc2,mesh->getBasicMesh().getNeighborFaces());
		if(!(fc1>-1 && fc2 ==-1)){
			return 0.f;
		}

		if(!meshOperation::isOnBorder(i%nrVerts, mesh->getBasicMesh())){
			return 0.f;
		}

		if(!meshOperation::isOnBorder(j%nrVerts, mesh->getBasicMesh())){
			return 0.f;
		}
		if((i<nrVerts && j<nrVerts) || (i>=nrVerts && j>=nrVerts)){
			return 0.f;
		}

		vector<int> & nbrfcs = mesh->getBasicMesh().getNeighborFaces()[i%nrVerts];
		vector<tuple3i> & fcs = mesh->getBasicMesh().getFaces();
		//itsthe last u_j
		if(fcs[nbrfcs.front()].orientation(edge) ==1 /*==-1/*contains(j%nrVerts)*/){
			//constraint for x, weight for y
			if(i<nrVerts){
				return -1.f;
			}
			else return +1.f;
		}
		//its the first u_k
		else if(fcs[nbrfcs.back()].orientation(edge) ==-1/*==1*/){
			if(i<nrVerts){
				return 1.f;
			}
			else return -1.f;
		}

	//	assert(false);
		return 0.f;
	}

	// row: its the vertex number; 
	void indices(int row, std::vector<int> & target){
		
		assert(row < 2* nrVerts);

		target.clear();
		if(!meshOperation::isOnBorder(row%nrVerts, mesh->getBasicMesh())){
			target.push_back(row);
			return;
		}

		vector<int> & nbrs = mesh->getBasicMesh().getNeighbors()[row%nrVerts];
		tuple2i edge;
		int fc1, fc2;
		for(int i = 0; i <nbrs.size(); i++){
			edge.set(row%nrVerts,nbrs[i]);

			meshOperation::getNbrFaces(edge,&fc1,&fc2,mesh->getBasicMesh().getNeighborFaces());
			if(fc2 < 0){
				target.push_back(row < nrVerts?
					nbrs[i]+nrVerts:nbrs[i]);
			}
			/*if(meshOperation::isOnBorder(nbrs[i], mesh->getBasicMesh())){
				if(row < nrVerts){
					target.push_back(nbrs[i] + nrVerts);
				}
				else{
					target.push_back(nbrs[i]);
				}
			}*/
		}
		sort( target.begin(), target.end() ); 

		//dbg
		if(target.size() != 2){
			int a = 2;
		}
		else{
			edge.set(row%nrVerts, target[0]%nrVerts);
			vector<int> & nbrfcs = mesh->getBasicMesh().getNeighborFaces()[row%nrVerts];
			vector<tuple3i> & fcs = mesh->getBasicMesh().getFaces();
			//itsthe last u_k
			int a = fcs[nbrfcs.front()].orientation(edge);
			int b = fcs[nbrfcs.back()].orientation(edge);
			
			edge.set(row%nrVerts, target[1]%nrVerts);
			int c = fcs[nbrfcs.front()].orientation(edge);
			int d = fcs[nbrfcs.back()].orientation(edge);
			/*if(!(a!=0 || b!=0))
				assert(false);
			if(!(c!=0 || d!=0))
				assert(false);*/
			assert(a==1 || b== -1);
			assert(c==1 || d== -1);
			assert(a+b == -c-d);

		}

	}
};
