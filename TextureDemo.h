#pragma once
#include "mesh.h"
#include "squareTexture.h"

class TextureDemo
{
public:
	static mesh * bunny;
	squareTexture * tex;
	TextureDemo(void);
	~TextureDemo(void);

	void run(mesh &m);
};

