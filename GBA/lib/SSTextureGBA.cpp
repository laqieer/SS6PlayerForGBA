#include "SSTextureGBA.h"

bool SSTextureGBA::tex_free[4] = {true, true, true, true};

int SSTextureGBA::getFreeTexture()
{
    for (int i = 0; i < 4; i++)
    {
        if ( tex_free[i] )
        {
            tex_free[i] = false;
            return i; // Get texture index
        }
    }
    return -1; // Fail to get free texture
}

/* =====================================================================================
	テクスチャファイルの読み込み
===================================================================================== */
int LoadTextureGBA( const char* Filename )
{
	int glyphTexture = SSTextureGBA::getFreeTexture();
    if (glyphTexture == -1)
    {
        return glyphTexture;
    }

    const GBFS_FILE *dat = find_first_gbfs_file((const void *)(&find_first_gbfs_file));

    // load palette and tiles (32 bytes palette + lz77 compressed tiles)
    u32 fileSize = 0;
    const char *fp = (const char *)gbfs_get_obj(dat, Filename, &fileSize);
    const int paletteLength = 32;
    memcpy(pal_obj_bank[glyphTexture], fp, paletteLength);
    const int tiles = 256;
    LZ77UnCompVram(fp + paletteLength, &tile_mem_obj[0][tiles * glyphTexture]);

	return glyphTexture;
}

SSTextureGBA::~SSTextureGBA()
{
    // Free texture
    tex_free[tex] = true;
}

bool SSTextureGBA::Load( const char* fname )
{
	tex = LoadTextureGBA( fname );
	return tex != -1;
}

