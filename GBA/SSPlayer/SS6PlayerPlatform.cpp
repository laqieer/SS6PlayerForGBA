﻿// 
//  SS6Platform.cpp
//
#include "SS6PlayerPlatform.h"

/**
* 各プラットフォームに合わせて処理を作成してください
* GBA用に作成されています。
*/

namespace ss
{
	//きれいな頂点変形に対応する場合は1にする。
	//４ポリゴンで変形します。
	//0の場合はZ型の２ポリゴンで変形します。
	#define USE_TRIANGLE_FIN (1)

	//セルマップの参照するテクスチャ割り当て管理用バッファ
	#define TEXTURE_MAX (4)							//全プレイヤーで使えるのセルマップの枚数
	SSTextureGBA* texture[TEXTURE_MAX];				//セルマップの参照するテクスチャ情報の保持
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
		_direction = PLUS_DOWN;
		_window_w = 240;
		_window_h = 160;

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
				// read the file from gbfs
                const GBFS_FILE *dat = find_first_gbfs_file(find_first_gbfs_file);
                const void *fp = gbfs_get_obj(dat, pszFileName, pSize);
                pBuffer = new unsigned char[*pSize];
                memcpy(pBuffer, fp, pSize);
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
					texture[texture_index] = SSTextureGBA::create();
					texture[texture_index]->Load(pszFileName);
					if (texture[texture_index]->tex == -1) {
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
        REG_DISPCNT |= DCNT_OBJ;
        RegisterRamReset(RESET_OAM);

		_ssDrawState.init();
	}

	void SSRenderEnd(void)
	{
        RegisterRamReset(RESET_OAM);
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
		* GBAのOBJ機能を使用してスプライトを表示します。
		* 下方向がプラスになります。
        */

        if (state.cellIndex > 31)
        {
            return;
        }

        OBJ_ATTR obj;

        switch ((int)state.size_Y + 100 * (int)state.size_X)
        {
            case 808:
                obj.attr1 = ATTR1_SIZE_8x8;
                obj.attr0 = ATTR0_SQUARE;
                break;
            case 1616:
                obj.attr1 = ATTR1_SIZE_16x16;
                obj.attr0 = ATTR0_SQUARE;
                break;
            case 3232:
                obj.attr1 = ATTR1_SIZE_32x32;
                obj.attr0 = ATTR0_SQUARE;
                break;
            case 6464:
                obj.attr1 = ATTR1_SIZE_64x64;
                obj.attr0 = ATTR0_SQUARE;
                break;
            case 816:
                obj.attr1 = ATTR1_SIZE_8x16;
                obj.attr0 = ATTR0_TALL;
                break;
            case 832:
                obj.attr1 = ATTR1_SIZE_8x32;
                obj.attr0 = ATTR0_TALL;
                break;
            case 1632:
                obj.attr1 = ATTR1_SIZE_16x32;
                obj.attr0 = ATTR0_TALL;
                break;
            case 3264:
                obj.attr1 = ATTR1_SIZE_32x64;
                obj.attr0 = ATTR0_TALL;
                break;
            case 168:
                obj.attr1 = ATTR1_SIZE_16x8;
                obj.attr0 = ATTR0_WIDE;
                break;
            case 328:
                obj.attr1 = ATTR1_SIZE_32x8;
                obj.attr0 = ATTR0_WIDE;
                break;
            case 3216:
                obj.attr1 = ATTR1_SIZE_32x16;
                obj.attr0 = ATTR0_WIDE;
                break;
            case 6432:
                obj.attr1 = ATTR1_SIZE_64x32;
                obj.attr0 = ATTR0_WIDE;
                break;
            default:
                return;
        }

        obj.attr2 = ATTR2_BUILD(256 * state.texture + 32 * (int)state.rect.origin.y / 8 + (int)state.rect.origin.x / 8, state.texture, 0);

        float x = state.mat[12];	/// 表示座標はマトリクスから取得します。
		float y = state.mat[13];	/// 表示座標はマトリクスから取得します。
        obj_set_pos(&obj, (int)x, (int)y);

        float rotationZ = RadianToDegree(state.Calc_rotationZ);		/// 回転値
		float scaleX = state.Calc_scaleX;							/// 拡大率
		float scaleY = state.Calc_scaleY;							/// 拡大率
        if (rotationZ == 0.0f && scaleX == 1.0f && scaleY == 1.0f)
        {
            obj.attr0 |= ATTR0_REG;
            obj.attr1 |= state.flipX ? ATTR1_HFLIP: 0;
            obj.attr1 |= state.flipY ? ATTR1_VFLIP: 0;
        }
        else
        {
            obj.attr0 |= ATTR0_AFF_DBL;
            // https://wiki.nycresistor.com/wiki/GB101:Affine_Sprites
            obj.attr1 |= ATTR1_AFF_ID(state.cellIndex);
            obj_aff_rotscale(&obj_aff_mem[state.cellIndex], (FIXED)(scaleX * 256), (FIXED)(scaleY * 256), (u16)(rotationZ * 0xffff / 360));
        }

        oam_copy(&obj_mem[state.cellIndex], &obj, 1);
    }


	void clearMask()
	{
		enableMask(false);
	}

	void enableMask(bool flag)
	{
	}

	void execMask(CustomSprite *sprite)
	{
	}

	/**
	* windows用パスチェック
	*/ 
	bool isAbsolutePath(const std::string& strPath)
	{
		return false;
	}

};
