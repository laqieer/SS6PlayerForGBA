#include <stdio.h>
#include <iostream>
#include <cstring>

#include <tonc.h>

#include "SS6Player.h"

//��ʃT�C�Y
#define WIDTH (SCREEN_WIDTH)
#define HEIGHT (SCREEN_HEIGHT)

//FPS����p
int nowtime = 0;	//�o�ߎ���
int drawtime = 0;	//�O��̎���

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


//�A�v���P�[�V�����̃��C���֐��֐�
int main() 
{
    irq_enable(II_VBLANK);

	Init();

    while (true)
    {
        VBlankIntrWait();
    }

	return 0;
}

//�`��R�[���o�b�N
void disp(void)
{
	draw();
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

	//�v���C���[���g�p����O�̏���������
	//���̏����̓A�v���P�[�V�����̏������łP�x�����s���Ă��������B
	ss::SSPlatformInit();
	//Y�����̐ݒ�ƃE�B���h�E�T�C�Y�ݒ���s���܂�
	ss::SSSetPlusDirection(ss::PLUS_DOWN, WIDTH, HEIGHT);
	//���\�[�X�}�l�[�W���̍쐬
	resman = ss::ResourceManager::getInstance();
	//�v���C���[���g�p����O�̏��������������܂�


	//�v���C���[�̍쐬
	ssplayer = ss::Player::create();

	//�A�j���f�[�^�����\�[�X�ɒǉ�

	//���ꂼ��̃v���b�g�t�H�[���ɍ��킹���p�X�֕ύX���Ă��������B
	resman->addData("Resources/character_template_comipo/character_template1.ssbp");
	//�v���C���[�Ƀ��\�[�X�����蓖��
	ssplayer->setData("character_template1");						// ssbp�t�@�C�����i�g���q�s�v�j
	//�Đ����郂�[�V������ݒ�
	ssplayer->play("character_template_3head/stance");				// �A�j���[�V���������w��(ssae��/�A�j���[�V����)


	//�\���ʒu��ݒ�
	ssplayer->setPosition(WIDTH / 2, HEIGHT / 2);
	ssplayer->setScale(1.0f, 1.0f);

	//���[�U�[�f�[�^�R�[���o�b�N��ݒ�
	ssplayer->setUserDataCallback(userDataCallback);

	//�A�j���[�V�����I���R�[���o�b�N��ݒ�
	ssplayer->setPlayEndCallback(playEndCallback);

	//ssbp�Ɋ܂܂�Ă���A�j���[�V�������̃��X�g���擾����
	animename = resman->getAnimeName(ssplayer->getPlayDataName());
	playindex = 0;				//���ݍĐ����Ă���A�j���̃C���f�b�N�X
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

