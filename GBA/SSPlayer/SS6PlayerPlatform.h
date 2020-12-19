/**
*  SS5PlayerPlatform.h
*/

#ifndef SS5PlayerPlatform_h
#define SS5PlayerPlatform_h
#include "SS6Player.h"
#include "common/loader/sstypes.h"
#include <stdio.h>
#include <string>
#include <Windows.h>
#include <stddef.h>

#include <GL/glew.h>
#include "../lib/SSTextureGL.h"

namespace ss
{
	struct State;
	struct UserData;
	class Player;
	class CustomSprite;

	extern void SSPlatformInit(void);
	extern void SSPlatformRelese(void);
	extern void SSSetPlusDirection(int direction, int window_w, int window_h);
	extern void SSGetPlusDirection(int &direction, int &window_w, int &window_h);
	extern void SSRenderingBlendFuncEnable(int flg);
	extern unsigned char* SSFileOpen(const char* pszFileName, const char* pszMode, unsigned long * pSize, const char * pszZipFileName);
	extern long SSTextureLoad(const char* pszFileName, int  wrapmode, int filtermode, const char * pszZipFileName);
	extern bool SSTextureRelese(long handle);
	extern bool SSGetTextureIndex(std::string  key, std::vector<int> *indexList);
	extern bool isAbsolutePath(const std::string& strPath);
	extern void SSRenderSetup(void);
	extern void SSRenderEnd(void);
	extern void SSDrawSprite(CustomSprite *sprite, State *overwrite_state = NULL);
	extern bool SSGetTextureSize(long handle, int &w, int &h);
	extern void clearMask();
	extern void enableMask(bool flag);
	extern void execMask(CustomSprite *sprite);



};	// namespace ss

#endif
