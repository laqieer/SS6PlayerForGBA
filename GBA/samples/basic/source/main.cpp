#include <stdio.h>
#include <iostream>
#include <cstring>

#include <tonc.h>

#include "SS6Player.h"

//画面サイズ
#define WIDTH (SCREEN_WIDTH)
#define HEIGHT (SCREEN_HEIGHT)

//FPS制御用
// int frameCnt;
#define SKIP_FRAME (2)

//glutのコールバック関数
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void idle(void);
void disp(void);

//アプリケーションの制御
void Init();
void update(float dt);
void relese(void);
void draw(void);

void userDataCallback(ss::Player* player, const ss::UserData* data);
void playEndCallback(ss::Player* player);

// SSプレイヤー
ss::Player *ssplayer;
ss::ResourceManager *resman;

//アプリケーションでの入力操作用
bool nextanime = false;			//次のアニメを再生する
bool forwardanime = false;		//前のアニメを再生する
bool pauseanime = false;
int playindex = 0;				//現在再生しているアニメのインデックス
int playerstate = 0;
std::vector<std::string> animename;	//アニメーション名のリスト


//アプリケーションのメイン関数関数
int main(void) 
{
    IRQ_INIT();
    irq_enable(II_VBLANK);

	Init();

//     frameCnt = 1;
    
    while (true)
    {
        VBlankIntrWait();
        
//         ss::DEBUG_PRINTF("frameCnt: %d", frameCnt);

//         if (frameCnt == 0)
        {
            update((float)SKIP_FRAME / 60.0f );		//ゲームの更新
		    draw();									//ゲームの描画
        }

//         frameCnt = (frameCnt + 1) % SKIP_FRAME;
    }

    /// プレイヤー終了処理
	relese( );

	return 0;
}

//描画コールバック
void disp(void)
{
	draw();
}

//アプリケーション初期化処理
void Init()
{
	/**********************************************************************************

	SpriteStudioアニメーション表示のサンプルコード
	Visual Studio Community 2017で動作を確認しています。
	WindowsSDK(デスクトップC++ x86およびx64用のWindows10 SDK)をインストールする必要があります
	プロジェクトのNuGetでglutを検索しnupengl.coreを追加してください。

	ssbpとpngがあれば再生する事ができますが、Resourcesフォルダにsspjも含まれています。

	**********************************************************************************/

    ss::DEBUG_PRINTF("Init Start!");
    
    // Init tte
    tte_init_se(0, BG_CBB(0)|BG_SBB(31), 0, CLR_WHITE, 0, NULL, NULL);
    tte_write("#{P:0,144}https://github.com/laqieer/SS6PlayerForGBA");
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
    
    //Set backdrop color
    pal_bg_mem[0] = 0x5425;
    
    //Hide all OBJs
    for(int i=0; i<128; i++)
    {
    	obj_hide(&obj_mem[i]);
    }

	//プレイヤーを使用する前の初期化処理
	//この処理はアプリケーションの初期化で１度だけ行ってください。
	ss::SSPlatformInit();
	//Y方向の設定とウィンドウサイズ設定を行います
	ss::SSSetPlusDirection(ss::PLUS_DOWN, WIDTH, HEIGHT);
	//リソースマネージャの作成
	resman = ss::ResourceManager::getInstance();
	//プレイヤーを使用する前の初期化処理ここまで


	//プレイヤーの作成
	tte_write("#{P:0,0}Create ssplayer\n");
	
	ssplayer = ss::Player::create();

    ss::DEBUG_PRINTF("ssplayer create complete!");
    
	//アニメデータをリソースに追加
	tte_write("Load model: chara_2head.ssbp\n");
	
	//それぞれのプラットフォームに合わせたパスへ変更してください。
	resman->addData("chara_2head.ssbp");
	
    ss::DEBUG_PRINTF("addData complete!");

	//プレイヤーにリソースを割り当て
	ssplayer->setData("chara_2head");						// ssbpファイル名（拡張子不要）
	
    ss::DEBUG_PRINTF("setData complete!");
    
	//再生するモーションを設定
	tte_write("Play motion: chara_2head/attack1\n");
	
	ssplayer->play("chara_2head/attack1");				// アニメーション名を指定(ssae名/アニメーション)
	
    ss::DEBUG_PRINTF("play attack1 complete!");


	//表示位置を設定
	ssplayer->setPosition(WIDTH / 2, HEIGHT / 2);
	ssplayer->setScale(1.0f, 1.0f);

	//ユーザーデータコールバックを設定
	ssplayer->setUserDataCallback(userDataCallback);

	//アニメーション終了コールバックを設定
	ssplayer->setPlayEndCallback(playEndCallback);

	//ssbpに含まれているアニメーション名のリストを取得する
	animename = resman->getAnimeName(ssplayer->getPlayDataName());
	playindex = 0;				//現在再生しているアニメのインデックス
	
    ss::DEBUG_PRINTF("Init Complete!");
}

//アプリケーション更新
void update(float dt)
{
	//プレイヤーの更新、引数は前回の更新処理から経過した時間
	ssplayer->update(dt);

	if (nextanime == true)
	{
		playindex++;
		if (playindex >= animename.size())
		{
			playindex = 0;
		}
		std::string name = animename.at(playindex);
		ssplayer->play(name);
		nextanime = false;
	}
	if (forwardanime == true)
	{
		playindex--;
		if ( playindex < 0 )
		{
			playindex = animename.size() - 1;
		}
		std::string name = animename.at(playindex);
		ssplayer->play(name);
		forwardanime = false;
	}
	if (pauseanime == true)
	{
		if (playerstate == 0)
		{
			ssplayer->animePause();
			playerstate = 1;
		}
		else
		{
			ssplayer->animeResume();
			playerstate = 0;
		}
		pauseanime = false;
	}
}

//ユーザーデータコールバック
void userDataCallback(ss::Player* player, const ss::UserData* data)
{
	//再生したフレームにユーザーデータが設定されている場合呼び出されます。
	//プレイヤーを判定する場合、ゲーム側で管理しているss::Playerのアドレスと比較して判定してください。
	/*
	//コールバック内でパーツのステータスを取得したい場合は、この時点ではアニメが更新されていないため、
	//getPartState　に　data->frameNo　でフレーム数を指定して取得してください。
	ss::ResluteState result;
	//再生しているモーションに含まれるパーツ名「collision」のステータスを取得します。
	ssplayer->getPartState(result, "collision", data->frameNo);
	*/

}

//アニメーション終了コールバック
void playEndCallback(ss::Player* player)
{
	//再生したアニメーションが終了した段階で呼び出されます。
	//プレイヤーを判定する場合、ゲーム側で管理しているss::Playerのアドレスと比較して判定してください。
	//player->getPlayAnimeName();
	//を使用する事で再生しているアニメーション名を取得する事もできます。

	//ループ回数分再生した後に呼び出される点に注意してください。
	//無限ループで再生している場合はコールバックが発生しません。

}

//アプリケーション描画
void draw(void)
{
	//プレイヤーの描画
	ssplayer->draw();
}

//アプリケーション終了処理
void relese(void)
{
	//SSPlayerの削除
	delete (ssplayer);
	delete (resman);
	ss::SSPlatformRelese( );
}

