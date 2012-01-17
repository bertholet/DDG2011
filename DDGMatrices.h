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

	//vornoi Area / 1
	static pardisoMatrix star0( meshMetaInfo & aMesh );
	//dual edge / edge
	static pardisoMatrix star1( meshMetaInfo & aMesh );
	//1 / volume
	static pardisoMatrix star2( meshMetaInfo & aMesh );


	static pardisoMatrix delta1( meshMetaInfo & aMesh );
	static pardisoMatrix delta2( meshMetaInfo & aMesh );
};
