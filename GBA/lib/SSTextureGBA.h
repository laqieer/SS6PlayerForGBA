#ifndef __TKTEXTURE__
#define __TKTEXTURE__

#include <stdio.h>
#include <cstring>

#include "tonc.h"
#include "gbfs.h"

class SSTextureGBA
{
public:
	int tex;
	const int tex_width = 256;
	const int tex_height = 64;
    static bool tex_free[4];

public:
	SSTextureGBA() : tex(0){}
	virtual ~SSTextureGBA();
	bool Load( const char* fname );	

	int	getWidth() { return tex_width; }
	int	getHeight() { return tex_height; }
	static SSTextureGBA* create(){ return new SSTextureGBA(); }

    static int getFreeTexture();
};

#endif
