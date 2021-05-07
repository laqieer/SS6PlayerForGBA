#include <stdio.h>
#include <iostream>
#include <cstring>

#include <tonc.h>

#include "SS6Player.h"

//��ʃT�C�Y
#define WIDTH (SCREEN_WIDTH)
#define HEIGHT (SCREEN_HEIGHT)

//FPS����p
// int frameCnt;
#define SKIP_FRAME (2)

//glut�̃R�[���o�b�N�֐�
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void idle(void);
void disp(void);

//�A�v���P�[�V�����̐���
void Init();
void update(float dt);
void relese(void);
void draw(void);

void userDataCallback(ss::Player* player, const ss::UserData* data);
void playEndCallback(ss::Player* player);

// SS�v���C���[
ss::Player *ssplayer;
ss::ResourceManager *resman;

//�A�v���P�[�V�����ł̓��͑���p
bool nextanime = false;			//���̃A�j�����Đ�����
bool forwardanime = false;		//�O�̃A�j�����Đ�����
bool pauseanime = false;
int playindex = 0;				//���ݍĐ����Ă���A�j���̃C���f�b�N�X
int playerstate = 0;
std::vector<std::string> animename;	//�A�j���[�V�������̃��X�g

// .ssae files
const char * models[] = {"character_template_2head",  "character_template_3head"};
const char * motions[] = {"wait",  "attack", "walk"};

static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

void handleInput()
{
	key_poll();
	
	// Press A/R/��/�� button to play next animation
	if(key_hit(KEY_A) || key_hit(KEY_R) || key_hit(KEY_RIGHT) || key_hit(KEY_DOWN))
	{
		nextanime = true;
	}
	
	// Press B/L/��/�� button to play previous animation
	if(key_hit(KEY_B) || key_hit(KEY_L) || key_hit(KEY_LEFT) || key_hit(KEY_UP))
	{
		forwardanime = true;
	}
	
	// Press Select/Start button to pause/resume animation
	if(key_hit(KEY_SELECT) || key_hit(KEY_START))
	{
		pauseanime = true;
	}
}

//�A�v���P�[�V�����̃��C���֐��֐�
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

		handleInput();

//         if (frameCnt == 0)
        {
            update((float)SKIP_FRAME / 60.0f );		//�Q�[���̍X�V
		    draw();									//�Q�[���̕`��
        }

//         frameCnt = (frameCnt + 1) % SKIP_FRAME;
    }

    /// �v���C���[�I������
	relese( );

	return 0;
}

//�`��R�[���o�b�N
void disp(void)
{
	draw();
}

void hideAllSprites()
{
    //Hide all OBJs
    for(int i=0; i<128; i++)
    {
        obj_hide(&obj_mem[i]);
    }
}

//�A�v���P�[�V��������������
void Init()
{
	/**********************************************************************************

	SpriteStudio�A�j���[�V�����\���̃T���v���R�[�h
	Visual Studio Community 2017�œ�����m�F���Ă��܂��B
	WindowsSDK(�f�X�N�g�b�vC++ x86�����x64�p��Windows10 SDK)���C���X�g�[������K�v������܂�
	�v���W�F�N�g��NuGet��glut��������nupengl.core��ǉ����Ă��������B

	ssbp��png������΍Đ����鎖���ł��܂����AResources�t�H���_��sspj���܂܂�Ă��܂��B

	**********************************************************************************/

    ss::DEBUG_PRINTF("Init Start!");
    
    // Init tte
    tte_init_se(0, BG_CBB(0)|BG_SBB(31), 0, CLR_WHITE, 0, NULL, NULL);
    tte_write("#{P:0,144}https://github.com/laqieer/SS6PlayerForGBA");
    tte_write("#{P:120,24}A/R/Right/Down#{P:120,32}Next#{P:120,40}B/L/Left/Up#{P:120,48}Previous#{P:120,56}Select/Start#{P:120,64}Pause/Resume");
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
    
    //Set backdrop color
    pal_bg_mem[0] = 0x5425;
    
    hideAllSprites();

	//�v���C���[���g�p����O�̏���������
	//���̏����̓A�v���P�[�V�����̏������łP�x�����s���Ă��������B
	ss::SSPlatformInit();
	//Y�����̐ݒ�ƃE�B���h�E�T�C�Y�ݒ���s���܂�
	ss::SSSetPlusDirection(ss::PLUS_DOWN, WIDTH, HEIGHT);
	//���\�[�X�}�l�[�W���̍쐬
	resman = ss::ResourceManager::getInstance();
	//�v���C���[���g�p����O�̏��������������܂�


	//�v���C���[�̍쐬
// 	tte_write("#{P:0,0}Create ssplayer\n");
	
	ssplayer = ss::Player::create();

    ss::DEBUG_PRINTF("ssplayer create complete!");
    
	//�A�j���f�[�^�����\�[�X�ɒǉ�
	tte_write("#{P:0,0}character_sample1.ssbp\n");
	
	//���ꂼ��̃v���b�g�t�H�[���ɍ��킹���p�X�֕ύX���Ă��������B
	resman->addData("character_sample1.ssbp");
	
    ss::DEBUG_PRINTF("addData complete!");

	//�v���C���[�Ƀ��\�[�X�����蓖��
	ssplayer->setData("character_sample1");						// ssbp�t�@�C�����i�g���q�s�v�j
	
    ss::DEBUG_PRINTF("setData complete!");
    
	//�Đ����郂�[�V������ݒ�
	tte_write("character_template_2head/attack");
	
	ssplayer->play("character_template_2head/attack");				// �A�j���[�V���������w��(ssae��/�A�j���[�V����)
	
    ss::DEBUG_PRINTF("play attack complete!");


	//�\���ʒu��ݒ�
	ssplayer->setPosition(WIDTH / 2, HEIGHT / 2);
	ssplayer->setScale(1.0f, 1.0f);

	//���[�U�[�f�[�^�R�[���o�b�N��ݒ�
	ssplayer->setUserDataCallback(userDataCallback);

	//�A�j���[�V�����I���R�[���o�b�N��ݒ�
	ssplayer->setPlayEndCallback(playEndCallback);

	//ssbp�Ɋ܂܂�Ă���A�j���[�V�������̃��X�g���擾����
	std::vector<std::string>  animenames = resman->getAnimeName(ssplayer->getPlayDataName());
	std::copy_if (animenames.begin(), animenames.end(), std::back_inserter(animename), [](std::string name){return !endsWith(name, "/Setup");} );
	playindex = 0;				//���ݍĐ����Ă���A�j���̃C���f�b�N�X
	
    ss::DEBUG_PRINTF("Init Complete!");
}

void refreshAnimeName(std::string name)
{
	tte_erase_rect (0, 8, 240, 24);
	tte_write_ex (0, 8, name.c_str(), NULL);
}

//�A�v���P�[�V�����X�V
void update(float dt)
{        
	//�v���C���[�̍X�V�A�����͑O��̍X�V��������o�߂�������
	ssplayer->update(dt);

	if (nextanime == true)
	{
		playindex++;
		if (playindex >= animename.size())
		{
			playindex = 0;
		}
		std::string name = animename.at(playindex);
		hideAllSprites();
		ssplayer->play(name);
		refreshAnimeName(name);
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
		hideAllSprites();
		ssplayer->play(name);
		tte_erase_line();
		refreshAnimeName(name);
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

//���[�U�[�f�[�^�R�[���o�b�N
void userDataCallback(ss::Player* player, const ss::UserData* data)
{
	//�Đ������t���[���Ƀ��[�U�[�f�[�^���ݒ肳��Ă���ꍇ�Ăяo����܂��B
	//�v���C���[�𔻒肷��ꍇ�A�Q�[�����ŊǗ����Ă���ss::Player�̃A�h���X�Ɣ�r���Ĕ��肵�Ă��������B
	/*
	//�R�[���o�b�N���Ńp�[�c�̃X�e�[�^�X���擾�������ꍇ�́A���̎��_�ł̓A�j�����X�V����Ă��Ȃ����߁A
	//getPartState�@�Ɂ@data->frameNo�@�Ńt���[�������w�肵�Ď擾���Ă��������B
	ss::ResluteState result;
	//�Đ����Ă��郂�[�V�����Ɋ܂܂��p�[�c���ucollision�v�̃X�e�[�^�X���擾���܂��B
	ssplayer->getPartState(result, "collision", data->frameNo);
	*/

}

//�A�j���[�V�����I���R�[���o�b�N
void playEndCallback(ss::Player* player)
{
	//�Đ������A�j���[�V�������I�������i�K�ŌĂяo����܂��B
	//�v���C���[�𔻒肷��ꍇ�A�Q�[�����ŊǗ����Ă���ss::Player�̃A�h���X�Ɣ�r���Ĕ��肵�Ă��������B
	//player->getPlayAnimeName();
	//���g�p���鎖�ōĐ����Ă���A�j���[�V���������擾���鎖���ł��܂��B

	//���[�v�񐔕��Đ�������ɌĂяo�����_�ɒ��ӂ��Ă��������B
	//�������[�v�ōĐ����Ă���ꍇ�̓R�[���o�b�N���������܂���B

}

//�A�v���P�[�V�����`��
void draw(void)
{
	//�v���C���[�̕`��
	ssplayer->draw();
}

//�A�v���P�[�V�����I������
void relese(void)
{
	//SSPlayer�̍폜
	delete (ssplayer);
	delete (resman);
	ss::SSPlatformRelese( );
}

