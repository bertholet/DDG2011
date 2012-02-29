#pragma once
#include "pardisoMatrix.h"
#include "meshMetaInfo.h"

class DDGMatrices
{
public:
	DDGMatrices(void);
	~DDGMatrices(void);

	static pardisoMatrix d0( meshMetaInfo & aMesh );
	static pardisoMatrix d1( meshMetaInfo & aMesh );
	//static pardisoMatrix d2( meshMetaInfo & aMesh );

	static pardisoMatrix dual_d0( meshMetaInfo & aMesh );
	static pardisoMatrix dual_d1( meshMetaInfo & aMesh );

	//vornoi Area / 1
	static pardisoMatrix star0( meshMetaInfo & aMesh );
	//dual edge / edge
	static pardisoMatrix star1( meshMetaInfo & aMesh );
	//1 / volume
	static pardisoMatrix star2( meshMetaInfo & aMesh );


	static pardisoMatrix delta1( meshMetaInfo & aMesh );
	static pardisoMatrix delta2( meshMetaInfo & aMesh );

	//////////////////////////////////////////////////////////////////////////
	// creates an identity Matrix.
	//////////////////////////////////////////////////////////////////////////
	static pardisoMatrix id0(meshMetaInfo & aMesh);
	static pardisoMatrix id1(meshMetaInfo & aMesh);
	static pardisoMatrix id2(meshMetaInfo & aMesh);

	//////////////////////////////////////////////////////////////////////////
	// special matrices for bordered meshs in an incompressibility setting
	// 
	// matrix takes a oneform and returns the dual values on the
	// primal simplices.
	//////////////////////////////////////////////////////////////////////////
	static pardisoMatrix dualVals1(meshMetaInfo & aMesh);
	//////////////////////////////////////////////////////////////////////////
	// duald1 + duald1border diff will be dual d1 in a setting where there are
	// borders and no compression i.e. the onforms sum up to zeor around 
	// each triangle.
	//////////////////////////////////////////////////////////////////////////
	static pardisoMatrix dual_d1_borderdiff( meshMetaInfo & aMesh );
};
