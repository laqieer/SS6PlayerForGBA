// 
//  SS6Platform.cpp
//
#include "SS6PlayerPlatform.h"

/**
* 各プラットフォームに合わせて処理を作成してください
* OpenGL+glut用に作成されています。
*/

namespace ss
{
	//きれいな頂点変形に対応する場合は1にする。
	//４ポリゴンで変形します。
	//0の場合はZ型の２ポリゴンで変形します。
	#define USE_TRIANGLE_FIN (1)

	//セルマップの参照するテクスチャ割り当て管理用バッファ
	#define TEXTURE_MAX (512)							//全プレイヤーで使えるのセルマップの枚数
	SSTextureGL* texture[TEXTURE_MAX];				//セルマップの参照するテクスチャ情報の保持
	std::string textureKey[TEXTURE_MAX];			//セルマップの参照するテクスチャキャッシュに登録するキー
	int texture_index = 0;							//セルマップの参照ポインタ

	//レンダリング用ブレンドファンクションを使用するかフラグ
	static bool enableRenderingBlendFunc = false;

	//座標系設定
	int _direction;
	int _window_w;
	int _window_h;


	//アプリケーション初期化時の処理
	void SSPlatformInit(void)
	{
		memset(texture, 0, sizeof(texture));
		int i;
		for (i = 0; i < TEXTURE_MAX; i++)
		{
			textureKey[i] = "";
		}
		texture_index = 0;
		_direction = PLUS_UP;
		_window_w = 1280;
		_window_h = 720;

		enableRenderingBlendFunc = false;
	}
	//アプリケーション終了時の処理
	void SSPlatformRelese(void)
	{
		int i;
		for (i = 0; i < TEXTURE_MAX; i++)
		{
			SSTextureRelese(i);
		}
	}

	/**
	* 上下どちらを正方向にするかとウィンドウサイズを設定します.
	* 上が正の場合はPLUS_UP、下が正の場合はPLUS_DOWN
	*
	* @param  direction      プラス方向
	* @param  window_w       ウィンドウサイズ
	* @param  window_h       ウィンドウサイズ
	*/
	void SSSetPlusDirection(int direction, int window_w, int window_h)
	{
		_direction = direction;
		_window_w = window_w;
		_window_h = window_h;
	}
	void SSGetPlusDirection(int &direction, int &window_w, int &window_h)
	{
		direction = _direction;
		window_w = _window_w;
		window_h = _window_h;
	}

	/**
	* レンダリング用のブレンドファンクションを使用する.
	* レンダリングターゲットとアルファ値がブレンドされてしまうためカラー値のみのレンダリングファンクションにする
	*
	* @param  flg	      通常描画:false、レンダリング描画:true
	*/
	void SSRenderingBlendFuncEnable(int flg)
	{
		enableRenderingBlendFunc = flg;
	}

	/**
	* ファイル読み込み
	*/
	unsigned char* SSFileOpen(const char* pszFileName, const char* pszMode, unsigned long * pSize, const char *pszZipFileName)
	{
		unsigned char * pBuffer = NULL;
		SS_ASSERT2(pszFileName != NULL && pSize != NULL && pszMode != NULL, "Invalid parameters.");
		*pSize = 0;

		if (strcmp(pszZipFileName, "") != 0)
		{
			//Zipファイルの読込み
			//何かしらのZIPファイル読み込み処理
		}
		else
		{
			//直接ファイルを読む
			do
			{
				// read the file from hardware
				FILE *fp = fopen(pszFileName, pszMode);
				SS_BREAK_IF(!fp);

				fseek(fp, 0, SEEK_END);
				*pSize = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				pBuffer = new unsigned char[*pSize];
				*pSize = fread(pBuffer, sizeof(unsigned char), *pSize, fp);
				fclose(fp);
			} while (0);
		}

		if (!pBuffer)
		{

			std::string msg = "Get data from file(";
			msg.append(pszFileName).append(") failed!");

			SSLOG("%s", msg.c_str());

		}
		return pBuffer;
	}

	/**
	* テクスチャの読み込み
	*/
	long SSTextureLoad(const char* pszFileName, int  wrapmode, int filtermode, const char *pszZipFileName)
	{
		/**
		* テクスチャ管理用のユニークな値を返してください。
		* テクスチャの管理はゲーム側で行う形になります。
		* テクスチャにアクセスするハンドルや、テクスチャを割り当てたバッファ番号等になります。
		*
		* プレイヤーはここで返した値とパーツのステータスを引数に描画を行います。
		* ResourceManager::changeTextureを使用する場合はSSTextureLoadから取得したインデックスを設定してください。
		*/
		long rc = 0;

		//空きバッファを検索して使用する
		int start_index = texture_index;	//開始したインデックスを保存する
		bool exit = true;
		bool isLoad = false;
		while (exit)
		{
			if (texture[texture_index] == 0)	//使われていないテクスチャ情報
			{

				if (strcmp(pszZipFileName, "") != 0)
				{
					//何かしらのZIPファイル読み込む処理
				}
				else
				{
					//読み込み処理
					texture[texture_index] = SSTextureGL::create();
					texture[texture_index]->Load(pszFileName);
					if (!texture[texture_index]->tex) {
						DEBUG_PRINTF("テクスチャの読み込み失敗\n");
					}
					else
					{
						isLoad = true;
						rc = texture_index;	//テクスチャハンドルをリソースマネージャに設定する
						textureKey[rc] = pszFileName;	//登録したテクスチャのキーを保存する
					}
				}
				exit = false;	//ループ終わり
			}
			//次のインデックスに移動する
			texture_index++;
			if (texture_index >= TEXTURE_MAX)
			{
				texture_index = 0;
			}
			if (texture_index == start_index)
			{
				//一周したバッファが開いてない
				DEBUG_PRINTF("テクスチャバッファの空きがない\n");
				exit = false;	//ループ終わり
			}
		}

		if (isLoad)
		{
			//SpriteStudioで設定されたテクスチャ設定を反映させるための分岐です。
			int target = GL_TEXTURE_RECTANGLE_ARB;
			if (SsUtTextureisPow2(texture[rc]->getWidth()) &&
				SsUtTextureisPow2(texture[rc]->getHeight()))
			{
				target = GL_TEXTURE_2D;
			}
			glBindTexture(target, texture[rc]->tex);
			//ラップモード
			GLint wrapMode = GL_REPEAT;
			switch (wrapmode)
			{
			case SsTexWrapMode::clamp:	//クランプ
				wrapMode = GL_CLAMP;
				break;
			case SsTexWrapMode::repeat:	//リピート
				wrapMode = GL_REPEAT;	//
				break;
			case SsTexWrapMode::mirror:	//ミラー
	//			wrapMode = GL_MIRRORED_REPEAT;	//
				break;
			}
			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapMode);

			//フィルタモード
			GLint filterMode;
			switch (filtermode)
			{
			case SsTexFilterMode::nearlest:	//ニアレストネイバー
				filterMode = GL_NEAREST;
				break;
			case SsTexFilterMode::linear:	//リニア、バイリニア
				filterMode = GL_LINEAR;
				break;
			}
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filterMode);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filterMode);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		return rc;
	}
	
	/**
	* テクスチャの解放
	*/
	bool SSTextureRelese(long handle)
	{
		/// 解放後も同じ番号で何度も解放処理が呼ばれるので、例外が出ないように作成してください。
		bool rc = true;

		//コメント
		if (texture[handle])
		{
			delete (texture[handle]);
		}
		else
		{
			rc = false;
		}
		//登録情報の削除
		texture[handle] = 0;
		textureKey[handle] = "";	//登録したテクスチャのキーを保存する

		return rc ;
	}

	/**
	* 画像ファイル名から読み込まれているテクスチャバッファのインデックスを取得する
	* keyはResourcesフォルダからの画像ファイルまでのパスになります。
	*
	* 使用されていない場合はfalseになります。
	*/
	bool SSGetTextureIndex(std::string  key, std::vector<int> *indexList)
	{
		bool rc = false;

		indexList->clear();

		int i;
		for (i = 0; i < TEXTURE_MAX; i++)
		{
			if (textureKey[i] == key)
			{
				indexList->push_back(i);
				rc = true;
			}
		}


		return (rc);
	}

	/**
	* テクスチャのサイズを取得
	* テクスチャのUVを設定するのに使用します。
	*/
	bool SSGetTextureSize(long handle, int &w, int &h)
	{
		if (texture[handle])
		{
			w = texture[handle]->getWidth();
			h = texture[handle]->getHeight();
		}
		else
		{
			return false;
		}
		return true;
	}

	/**
	* 描画ステータス
	*/
	struct SSDrawState
	{
		int texture;
		int partType;
		int partBlendfunc;
		int partsColorUse;
		int partsColorFunc;
		int partsColorType;
		int maskInfluence;
		void init(void)
		{
			texture = -1;
			partType = -1;
			partBlendfunc = -1;
			partsColorUse = -1;
			partsColorFunc = -1;
			partsColorType = -1;
			maskInfluence = -1;
		}
	};
	SSDrawState _ssDrawState;

	//各プレイヤーの描画を行う前の初期化処理
	void SSRenderSetup( void )
	{
		glDisableClientState(GL_COLOR_ARRAY);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0);

		glBlendEquation(GL_FUNC_ADD);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

//		glOrtho(0.0, _window_w, _window_h, 0.0, -1.0, 1.0);	//並行投影変換
		glOrtho(0.0, _window_w, 0.0, _window_h , -1.0, 1.0);	//並行投影変換

		_ssDrawState.init();
	}

	void SSRenderEnd(void)
	{
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_ALPHA_TEST);
		//ブレンドモード　減算時の設定を戻す
		glBlendEquation(GL_FUNC_ADD);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	//中間点を求める
	static void	CoordinateGetDiagonalIntersection(SsVector2& out, const SsVector2& LU, const SsVector2& RU, const SsVector2& LD, const SsVector2& RD)
	{
		out = SsVector2(0.f, 0.f);

		/* <<< 係数を求める >>> */
		float c1 = (LD.y - RU.y) * (LD.x - LU.x) - (LD.x - RU.x) * (LD.y - LU.y);
		float c2 = (RD.x - LU.x) * (LD.y - LU.y) - (RD.y - LU.y) * (LD.x - LU.x);
		float c3 = (RD.x - LU.x) * (LD.y - RU.y) - (RD.y - LU.y) * (LD.x - RU.x);


		if (c3 <= 0 && c3 >= 0) return;

		float ca = c1 / c3;
		float cb = c2 / c3;

		/* <<< 交差判定 >>> */
		if (((0.0f <= ca) && (1.0f >= ca)) && ((0.0f <= cb) && (1.0f >= cb)))
		{	/* 交差している */
			out.x = LU.x + ca * (RD.x - LU.x);
			out.y = LU.y + ca * (RD.y - LU.y);
		}
	}

	//パーツカラーがない場合の設定
	/// // RGB=100%テクスチャ、A=テクスチャｘ頂点カラーの設定にする。
	static void __fastcall setupTextureCombinerTo_NoBlendRGB_MultiplyAlpha_()
	{
		// カラーは１００％テクスチャ
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
		// αだけ合成
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	}
	//6.2対応
	//パーツカラー、ミックス、頂点の場合に不透明度を適用させる
	/**
	ブレンドタイプに応じたテクスチャコンバイナの設定を行う (パーツカラー用)

	rateOrAlpha		ミックス時のみ参照される。単色では Rate に、頂点単位では Alpha になる。
	target			ミックス時のみ参照される。

	参考：http://www.opengl.org/sdk/docs/man/xhtml/glTexEnv.xml
	*/
	static void __fastcall setupSimpleTextureCombiner_for_PartsColor_(BlendType type, float rateOrAlpha, VertexFlag target)
	{
		//static const float oneColor[4] = {1.f,1.f,1.f,1.f};
		float constColor[4] = { 0.5f,0.5f,0.5f,rateOrAlpha };
		static const GLuint funcs[] = { GL_INTERPOLATE, GL_MODULATE, GL_ADD, GL_SUBTRACT };
		GLuint func = funcs[(int)type];
		GLuint srcRGB = GL_TEXTURE0;
		GLuint dstRGB = GL_PRIMARY_COLOR;

		// true:  頂点αをブレンドする。
		// false: constColor のαをブレンドする。
		bool combineAlpha = true;

		switch (type)
		{
		case BlendType::BLEND_MIX:
		case BlendType::BLEND_MUL:
		case BlendType::BLEND_ADD:
		case BlendType::BLEND_SUB:
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			// rgb
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, func);

			// mix の場合、特殊
			if (type == SsBlendType::mix)
			{
				if (target == VertexFlag::VERTEX_FLAG_ONE)
				{
					// 全体なら、const 値で補間する
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
					glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
				}
				else
				{
					// 頂点カラーのアルファをテクスチャに対する頂点カラーの割合にする。
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);

					combineAlpha = false;
				}
				// 強度なので 1 に近付くほど頂点カラーが濃くなるよう SOURCE0 を頂点カラーにしておく。
				std::swap(srcRGB, dstRGB);
			}
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, srcRGB);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, dstRGB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
			break;
		case BlendType::BLEND_SCREEN:
		case BlendType::BLEND_EXCLUSION:
		case BlendType::BLEND_INVERT:
		default:
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			break;
		}

		if (combineAlpha)
		{
			// alpha は常に掛け算
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		}
		else
		{
#if 1
			// 浮いた const 値を頂点αの代わりにブレンドする。v6.2.0+ 2018/06/21 endo
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
#else
			// ミックス＋頂点単位の場合αブレンドはできない。
			// αはテクスチャを100%使えれば最高だが、そうはいかない。
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
#endif
		}
	}

	//頂点バッファにパラメータを保存する
	void setClientState(SSV3F_C4B_T2F point, int index, float* uvs, float* colors, float* vertices)
	{
		uvs[0 + (index * 2)] = point.texCoords.u;
		uvs[1 + (index * 2)] = point.texCoords.v;

		colors[0 + (index * 4)] = point.colors.r / 255.0f;
		colors[1 + (index * 4)] = point.colors.g / 255.0f;
		colors[2 + (index * 4)] = point.colors.b / 255.0f;
		colors[3 + (index * 4)] = point.colors.a / 255.0f;

		vertices[0 + (index * 3)] = point.vertices.x;
		vertices[1 + (index * 3)] = point.vertices.y;
		vertices[2 + (index * 3)] = point.vertices.z;
	}

	/**
	* メッシュの表示
	*/
	void SSDrawMesh(CustomSprite *sprite, State state)
	{
		bool ispartColor = (state.flags & PART_FLAG_PARTS_COLOR);

		// 単色で処理する
		float alpha = ((float)state.quad.tl.colors.a / 255.0f) * ((float)state.Calc_opacity / 255.0f);
		float setcol[4];
		setcol[0] = state.quad.tl.colors.r / 255.0f;
		setcol[1] = state.quad.tl.colors.g / 255.0f;
		setcol[2] = state.quad.tl.colors.b / 255.0f;
		setcol[3] = alpha;

		if (
			(_ssDrawState.partsColorFunc != state.partsColorFunc)
			|| (_ssDrawState.partsColorType != state.partsColorType)
			|| (_ssDrawState.partsColorUse != ispartColor)
		   )
		{
			// パーツカラーの指定
			if (ispartColor)
			{

				// パーツカラーがある時だけブレンド計算する
				setupSimpleTextureCombiner_for_PartsColor_((BlendType)state.partsColorFunc, state.rate.oneRate, (VertexFlag)state.partsColorType);
			}
			else
			{
				//パーツカラーなし
				setupTextureCombinerTo_NoBlendRGB_MultiplyAlpha_();
			}
		}


		//メッシュの座標データは親子の計算が済んでいるのでプレイヤーのTRSで変形させる
		float t[16];
		float mat[16];
		IdentityMatrix(mat);
		State pls = sprite->_parentPlayer->getState();

		MultiplyMatrix(pls.mat, mat, mat);

		for (size_t i = 0; i < sprite->_meshVertexSize; i++)
		{
			sprite->_mesh_colors[i * 4 + 0] = setcol[0];
			sprite->_mesh_colors[i * 4 + 1] = setcol[1];
			sprite->_mesh_colors[i * 4 + 2] = setcol[2];
			sprite->_mesh_colors[i * 4 + 3] = setcol[3];

			if (sprite->_meshIsBind == true)
			{
				//プレイヤーのマトリクスをメッシュデータに与える
				TranslationMatrix(t, sprite->_mesh_vertices[i * 3 + 0], sprite->_mesh_vertices[i * 3 + 1], sprite->_mesh_vertices[i * 3 + 2]);
				MultiplyMatrix(t, mat, t);
				sprite->_mesh_vertices[i * 3 + 0] = t[12];
				sprite->_mesh_vertices[i * 3 + 1] = t[13];
				sprite->_mesh_vertices[i * 3 + 2] = 0;
			}
			else
			{
				//バインドされていないメッシュはパーツのマトリクスを与える
				TranslationMatrix(t, sprite->_mesh_vertices[i * 3 + 0], sprite->_mesh_vertices[i * 3 + 1], sprite->_mesh_vertices[i * 3 + 2]);
				MultiplyMatrix(t, state.mat, t);
				MultiplyMatrix(t, mat, t);
				sprite->_mesh_vertices[i * 3 + 0] = t[12];
				sprite->_mesh_vertices[i * 3 + 1] = t[13];
				sprite->_mesh_vertices[i * 3 + 2] = 0;
			}

		}


		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		// UV 配列を指定する
		glTexCoordPointer(2, GL_FLOAT, 0, (GLvoid *)sprite->_mesh_uvs);
		// カラー配列を設定する
		glColorPointer(4, GL_FLOAT, 0, (GLvoid *)sprite->_mesh_colors);
		// 頂点バッファの設定
		glVertexPointer(3, GL_FLOAT, 0, (GLvoid *)sprite->_mesh_vertices);


		glDrawElements(GL_TRIANGLES, sprite->_meshTriangleSize * 3, GL_UNSIGNED_SHORT, sprite->_mesh_indices);
	}

	/**
	* スプライトの表示
	*/
	void SSDrawSprite(CustomSprite *sprite, State *overwrite_state)
	{
		//アプリケーションからPlayer::drawに渡された拡張パラメータ
		void* exParam = sprite->_parentPlayer->getExParamDraw();
		if (exParam)
		{
			//exParamをキャストして使用してください。
		}


		if (sprite->_state.isVisibled == false) return; //非表示なので処理をしない

		//ステータスから情報を取得し、各プラットフォームに合わせて機能を実装してください。
		State state;
		if (overwrite_state)
		{
			//個別に用意したステートを使用する（エフェクトのパーティクル用）
			state = *overwrite_state;
		}
		else
		{
			state = sprite->_state;
		}
		int tex_index = state.texture.handle;
		if (texture[tex_index] == nullptr)
		{
			return;
		}

		execMask(sprite);	//マスク初期化

		/**
		* OpenGLの3D機能を使用してスプライトを表示します。
		* 下方向がプラスになります。
		* 3Dを使用する場合頂点情報を使用して再現すると頂点変形やUV系のアトリビュートを反映させる事ができます。
		*/
		//描画用頂点情報を作成
		SSV3F_C4B_T2F_Quad quad;
		quad = state.quad;

		//原点補正
		float cx = state.size_X * -state.pivotX;
		float cy = state.size_Y * -state.pivotY;

		quad.tl.vertices.x += cx;
		quad.tl.vertices.y += cy;
		quad.tr.vertices.x += cx;
		quad.tr.vertices.y += cy;
		quad.bl.vertices.x += cx;
		quad.bl.vertices.y += cy;
		quad.br.vertices.x += cx;
		quad.br.vertices.y += cy;

		float mat[16];
		IdentityMatrix(mat);
		State pls = sprite->_parentPlayer->getState();	//プレイヤーのTRSを最終座標に加える

		MultiplyMatrix(pls.mat, mat, mat);

		float t[16];
		TranslationMatrix(t, quad.tl.vertices.x, quad.tl.vertices.y, 0.0f);

		MultiplyMatrix(t, state.mat, t);	//SS上のTRS
		MultiplyMatrix(t, mat, t);			//プレイヤーのTRS	
		quad.tl.vertices.x = t[12];
		quad.tl.vertices.y = t[13];
		TranslationMatrix(t, quad.tr.vertices.x, quad.tr.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.tr.vertices.x = t[12];
		quad.tr.vertices.y = t[13];
		TranslationMatrix(t, quad.bl.vertices.x, quad.bl.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.bl.vertices.x = t[12];
		quad.bl.vertices.y = t[13];
		TranslationMatrix(t, quad.br.vertices.x, quad.br.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.br.vertices.x = t[12];
		quad.br.vertices.y = t[13];



		//頂点カラーにアルファを設定
		float alpha = state.Calc_opacity / 255.0f;
		if (state.flags & PART_FLAG_LOCALOPACITY)
		{
			alpha = state.localopacity / 255.0f;	//ローカル不透明度対応
		}

		if (
			 (state.flags & PART_FLAG_PARTS_COLOR) 
		  && ((VertexFlag)state.partsColorType != VertexFlag::VERTEX_FLAG_ONE) 
		  && ((BlendType)state.partsColorFunc == BlendType::BLEND_MIX)
		   )
		{
			//ver6.2 パーツカラー対応
			//パーツカラー、頂点、MIXを選択した場合は不透明度を適用しない
			//ミックスの場合、Rate として扱われるので不透明度を掛けてはいけない
		}
		else
		{
			quad.tl.colors.a = quad.tl.colors.a * alpha;
			quad.tr.colors.a = quad.tr.colors.a * alpha;
			quad.bl.colors.a = quad.bl.colors.a * alpha;
			quad.br.colors.a = quad.br.colors.a * alpha;
		}
		//テクスチャ有効
		int	gl_target = GL_TEXTURE_RECTANGLE_ARB;
		if (texture[tex_index]->texture_is_pow2 == true)
		{
			gl_target = GL_TEXTURE_2D;
		}

		if (_ssDrawState.texture != texture[tex_index]->tex)
		{
			glEnable(gl_target);
			//テクスチャのバインド
			glBindTexture(gl_target, texture[tex_index]->tex);
		}


		//描画モード
		//SS6ではストレートアルファで保持、ブレンドを行います。
		//プリマルチアルファで処理を行う場合はブレンドファンクションが異なります。
		if (_ssDrawState.partBlendfunc != state.blendfunc)
		{
			glBlendEquation(GL_FUNC_ADD);
			if (enableRenderingBlendFunc == false)
			{
				//通常の描画
				switch (state.blendfunc)
				{
				case BLEND_MIX:		///< 0 ブレンド（ミックス）
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					break;
				case BLEND_MUL:		///< 1 乗算
					glBlendFunc(GL_ZERO, GL_SRC_COLOR);
					break;
				case BLEND_ADD:		///< 2 加算
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					break;
				case BLEND_SUB:		///< 3 減算
					glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
					glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_DST_ALPHA);
					break;
				case BLEND_MULALPHA:	///< 4 α乗算
					glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
					break;
				case BLEND_SCREEN:		///< 5 スクリーン
					glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
					break;
				case BLEND_EXCLUSION:	///< 6 除外
					glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
					break;
				case BLEND_INVERT:		///< 7 反転
					glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
					break;

				}
			}
			else
			{
				//レンダリング用の描画
				switch (state.blendfunc)
				{
				case BLEND_MIX:		///< 0 ブレンド（ミックス）
#if OPENGLES20
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_MUL:		///< 1 乗算
#if OPENGLES20
					glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_ZERO, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_ADD:		///< 2 加算
#if OPENGLES20
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_SUB:		///< 3 減算

					glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
#if OPENGLES20
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_MULALPHA:	///< 4 α乗算
#if OPENGLES20
					glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_SCREEN:		///< 5 スクリーン
#if OPENGLES20
					glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_ONE_MINUS_DST_COLOR, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_EXCLUSION:	///< 6 除外
#if OPENGLES20
					glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_INVERT:		///< 7 反転
#if OPENGLES20
					glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ZERO, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_ONE_MINUS_DST_COLOR, GL_ZERO, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				}
			}
		}

		bool ispartColor = (state.flags & PART_FLAG_PARTS_COLOR);

		//メッシュの場合描画
		if (sprite->_partData.type == PARTTYPE_MESH)
		{
			SSDrawMesh(sprite, state);

			_ssDrawState.texture = texture[tex_index]->tex;
			_ssDrawState.partType = sprite->_partData.type;
			_ssDrawState.partBlendfunc = state.blendfunc;
			_ssDrawState.partsColorFunc = state.partsColorFunc;
			_ssDrawState.partsColorType = state.partsColorType;
			_ssDrawState.partsColorUse = (int)ispartColor;
			_ssDrawState.maskInfluence = (int)sprite->_maskInfluence;
			return;
		}

		//パーツカラーの適用
		if (
			   (_ssDrawState.partsColorFunc != state.partsColorFunc)
			|| (_ssDrawState.partsColorType != state.partsColorType)
			|| (_ssDrawState.partsColorUse != ispartColor)
			)
		{
			if (state.flags & PART_FLAG_PARTS_COLOR)
			{
				//パーツカラーの反映
				if ((VertexFlag)state.partsColorType == VertexFlag::VERTEX_FLAG_ONE)
				{
					//単色
					setupSimpleTextureCombiner_for_PartsColor_((BlendType)state.partsColorFunc, state.rate.oneRate, (VertexFlag)state.partsColorType);
				}
				else
				{
					//頂点
					setupSimpleTextureCombiner_for_PartsColor_((BlendType)state.partsColorFunc, alpha, (VertexFlag)state.partsColorType);
				}
			}
			else
			{
				//パーツカラーなし
				setupTextureCombinerTo_NoBlendRGB_MultiplyAlpha_();
			}
		}

		// ssbpLibでは4つの頂点でスプライトの表示を実装しています。
		// SS6では５つの頂点でスプライトの表示を行っており、頂点変形時のゆがみ方が異なります。
		float	uvs[2*5];			// UVバッファ
		float	colors[4 * 5];		// カラーバッファ
		float	vertices[3 * 5];	// 座標バッファ

		memset(uvs, 0, sizeof(uvs));
		memset(colors, 0, sizeof(colors));
		memset(vertices, 0, sizeof(vertices));

#if USE_TRIANGLE_FIN
		setClientState(quad.tl, 0, uvs, colors, vertices);
		setClientState(quad.tr, 1, uvs, colors, vertices);
		setClientState(quad.bl, 2, uvs, colors, vertices);
		setClientState(quad.br, 3, uvs, colors, vertices);

		// UV 配列を指定する
		glEnableClientState(GL_VERTEX_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, (GLvoid *)uvs);
		// カラー配列を指定する
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, (GLvoid *)colors);
		// 頂点バッファの設定
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (GLvoid *)vertices);

		//きれいな頂点変形に対応
		if ((state.flags & PART_FLAG_PARTS_COLOR) || (state.flags & PART_FLAG_VERTEX_TRANSFORM))
		{
			//頂点変形、パーツカラーを使用した場合は中心に頂点を作成し4つのポリゴンに分割して描画を行う。
			//頂点の算出
			SsVector2	vertexCoordinateLU = SsVector2(quad.tl.vertices.x, quad.tl.vertices.y);// : 左上頂点座標（ピクセル座標系）
			SsVector2	vertexCoordinateRU = SsVector2(quad.tr.vertices.x, quad.tr.vertices.y);// : 右上頂点座標（ピクセル座標系）
			SsVector2	vertexCoordinateLD = SsVector2(quad.bl.vertices.x, quad.bl.vertices.y);// : 左下頂点座標（ピクセル座標系）
			SsVector2	vertexCoordinateRD = SsVector2(quad.br.vertices.x, quad.br.vertices.y);// : 右下頂点座標（ピクセル座標系）

			SsVector2 CoordinateLURU = (vertexCoordinateLU + vertexCoordinateRU) * 0.5f;
			SsVector2 CoordinateLULD = (vertexCoordinateLU + vertexCoordinateLD) * 0.5f;
			SsVector2 CoordinateLDRD = (vertexCoordinateLD + vertexCoordinateRD) * 0.5f;
			SsVector2 CoordinateRURD = (vertexCoordinateRU + vertexCoordinateRD) * 0.5f;

			SsVector2 center;
			CoordinateGetDiagonalIntersection(center, CoordinateLURU, CoordinateRURD, CoordinateLULD, CoordinateLDRD);

			SsVector2*	coodinatetable[] = { &vertexCoordinateLU , &vertexCoordinateRU , &vertexCoordinateLD , &vertexCoordinateRD , &center };

			//頂点の設定
			int i;
			vertices[4 * 3 + 0] = center.x;
			vertices[4 * 3 + 1] = center.y;
			vertices[4 * 3 + 2] = 0;
			//UVの設定
			for (i = 0; i < 4; ++i)
			{
				uvs[4 * 2] += uvs[i * 2];
				uvs[4 * 2 + 1] += uvs[i * 2 + 1];
			}
			uvs[4 * 2] /= 4.0f;
			uvs[4 * 2 + 1] /= 4.0f;

			float a, r, g, b;
			a = r = g = b = 0;
			for (int i = 0; i < 4; i++)
			{
				int idx = i * 4;
				r += colors[idx++];
				g += colors[idx++];
				b += colors[idx++];
				a += colors[idx++];
			}
			//カラー値の設定
			int idx = 4 * 4;
			colors[idx++] = r / 4.0f;
			colors[idx++] = g / 4.0f;
			colors[idx++] = b / 4.0f;
			colors[idx++] = a / 4.0f;

			//描画
			static const GLubyte indices[] = { 4, 3, 1, 0, 2, 3 };
			glDrawElements(GL_TRIANGLE_FAN, 6, GL_UNSIGNED_BYTE, indices);
		}
		else
		{
			// 変形しないスプライトはZ型の2ポリゴンで分割表示する
			// 頂点配列を描画
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
#else
		setClientState(quad.tl, 0, uvs, colors, vertices);
		setClientState(quad.tr, 1, uvs, colors, vertices);
		setClientState(quad.bl, 2, uvs, colors, vertices);
		setClientState(quad.br, 3, uvs, colors, vertices);
		
		// UV 配列を指定
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, (GLvoid *)uvs);
		// 頂点カラーの設定
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, (GLvoid *)colors);
		// 頂点バッファの設定
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (GLvoid *)vertices);

		// 頂点配列を描画
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
		//レンダリングステートの保存
		_ssDrawState.texture = texture[tex_index]->tex;
		_ssDrawState.partType = sprite->_partData.type;
		_ssDrawState.partBlendfunc = state.blendfunc;
		_ssDrawState.partsColorFunc = state.partsColorFunc;
		_ssDrawState.partsColorType = state.partsColorType;
		_ssDrawState.partsColorUse = (int)ispartColor;
		_ssDrawState.maskInfluence = (int)sprite->_maskInfluence;
//
/*
		glDisable(gl_target);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_ALPHA_TEST);
		//ブレンドモード　減算時の設定を戻す
		glBlendEquation(GL_FUNC_ADD);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
*/
}


	void clearMask()
	{
		glClear(GL_STENCIL_BUFFER_BIT);
		enableMask(false);
	}

	void enableMask(bool flag)
	{

		if (flag)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else {
			glDisable(GL_STENCIL_TEST);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
		_ssDrawState.maskInfluence = -1;		//マスクを実行する
		_ssDrawState.partType = -1;		//マスクを実行する
	}

	void execMask(CustomSprite *sprite)
	{
		if (
			 (_ssDrawState.partType != sprite->_partData.type)
		  || (_ssDrawState.maskInfluence != (int)sprite->_maskInfluence)
		   )
		{
			glEnable(GL_STENCIL_TEST);
			if (sprite->_partData.type == PARTTYPE_MASK)
			{

				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

				if (!(sprite->_maskInfluence)) { //マスクが有効では無い＝重ね合わせる
					glStencilFunc(GL_ALWAYS, 1, ~0);  //常に通過
					glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
					//描画部分を1へ
				}
				else {
					glStencilFunc(GL_ALWAYS, 1, ~0);  //常に通過
					glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
				}

				glEnable(GL_ALPHA_TEST);

			}
			else {

				if ((sprite->_maskInfluence)) //パーツに対してのマスクが有効か否か
				{
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);  //1と等しい
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				}
				else {
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glDisable(GL_STENCIL_TEST);
				}

				// 常に無効
				glDisable(GL_ALPHA_TEST);
			}
		}
		if (sprite->_partData.type == PARTTYPE_MASK)
		{
			//不透明度からマスク閾値へ変更
			float mask_alpha = (float)(255 - sprite->_state.masklimen) / 255.0f;
			glAlphaFunc(GL_GREATER, mask_alpha);
		}
	}

	/**
	* 文字コード変換
	*/ 

	std::string utf8Togbk(const char *src)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
		unsigned short * wszGBK = new unsigned short[len + 1];
		memset(wszGBK, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, (LPWSTR)wszGBK, len);

		len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
		char *szGBK = new char[len + 1];
		memset(szGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
		std::string strTemp(szGBK);
		if (strTemp.find('?') != std::string::npos)
		{
			strTemp.assign(src);
		}
		delete[]szGBK;
		delete[]wszGBK;
		return strTemp;
	}

	/**
	* windows用パスチェック
	*/ 
	bool isAbsolutePath(const std::string& strPath)
	{

		std::string strPathAscii = utf8Togbk(strPath.c_str());
		if (strPathAscii.length() > 2
			&& ((strPathAscii[0] >= 'a' && strPathAscii[0] <= 'z') || (strPathAscii[0] >= 'A' && strPathAscii[0] <= 'Z'))
			&& strPathAscii[1] == ':')
		{
			return true;
		}
		return false;
	}

};
