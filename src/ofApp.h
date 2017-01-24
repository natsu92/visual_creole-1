#pragma once

#include "ofMain.h"

#include <wchar.h>
#include <direct.h>

#include<string>
#include<iostream> 
#include<fstream> 
#include<sstream>

#include "pxccapture.h"
#include "pxcsensemanager.h"
#include "pxccapturemanager.h"
#include "pxchandcursormodule.h"
#include "pxccursordata.h"
#include "pxchandconfiguration.h"
#include "pxchanddata.h"
#include "pxchandmodule.h"
#include "pxcfacemodule.h"
#include "pxcfaceconfiguration.h"
#include "pxcprojection.h"

#include "ofxColorPicker.h"
#include "ofxImGui.h"


class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	//Visual Creole�S�̂�State�B�N������̃v���W�F�N�g�I���A�^��A�ҏW�̂R��
	enum _VC_State {
		ENTRY, //�N������͂���B
		RECORD,
		EDIT
	};

	_VC_State VC_State;

	//�^���ʂł̃X�e�[�g
	enum R_State {
		WAITING, //
		READY,
		RECORDING,
		RECORDED //�^��I�����B���Ԃ񂷂��ҏW��ʂɑJ�ڂ���
	};

	//�Đ���ʂł̃X�e�[�g�B���̎��ҏW�A�r���[�X�e�[�g�������ɑJ�ڂ���B
	enum P_State {
		STOP,//��~���B
		PLAYING, //�Đ����B
		PREVIEW //������p�[�c�ɑ΂���v���r���[���Av0.1�ł̓p�[�c�̑O��1�b�̌v3�b
	};

	//�ҏW��ʂł̃X�e�[�g�A�ҏW�������G������
	enum E_State {
		EDITING_NONE, //�Ȃɂ��ҏW���ĂȂ��B�J�n����ҏW�̈��OK������ȂǁB
		EDITING_LEFT, //����ҏW��
		EDITING_RIGHT, //�E��ҏW��
		EDITING_BACK, //�w�i�ҏW��
	};

	//Assets�̃r���[��ԁi��ʉE�A���i��PNG�̕ۊǌɂ̏�Ԃ������B
	enum V_State {
		VIEW_ALL,
		VIEW_LEFT, //
		VIEW_RIGHT,
		VIEW_BACK
	};

	typedef struct _part {
		int startframe; //�`�悪�J�n�����t���[���ԍ�
		int endframe; //�`�悪��~�����t���[���ԍ��Av0.1�ł͊J�n30�t���[���i1�b)��
		int offset_x; //�摜��px�P�ʂ̃I�t�Z�b�g�A�����ŕ`�����ꍇ�͂��܂�K�p���Ȃ������A�O����png��ǂ񂾂Ƃ��ɗ��p����B
		int offset_y;
		int size_percent; //�摜�̑傫�������\���B�O������ǂݍ���png���傫�������菬���������Ƃ��ɁB
						  //�Ƃ肠����%���������œ����邩�ǂ�������Ă݂�B�Ƃ�����v0.1�ł͖������ɂȂ邩���B
						  //int size_x; //�ǂݍ��񂾉摜�̂��Ƃ��Ƃ�
						  //int size_y;
		ofPixels picture;
	} part;

	//���I�z��vector�ɂ��錾
	std::vector<part> partsLeft;
	std::vector<part> partsRight;
	std::vector<part> partsBack;

	void initializeLive();
	void initializeCapture();
	void initializePlayer();
	void initializeLoadedValue();

	void updateCamera();

	void drawPoints();
	void drawImages();
	void drawPicCtrl();

	ofFbo fbo_back;
	ofFbo fbo_right;
	ofFbo fbo_left;
	ofFbo fbo_face;

	ofPoint mouseLoc_back;
	ofPoint lastmouseLoc_back;

	ofPoint mouseLoc_right;
	ofPoint lastmouseLoc_right;

	ofPoint mouseLoc_left;
	ofPoint lastmouseLoc_left;

	ofPoint mouseLoc_face;
	ofPoint lastmouseLoc_face;

	ofImage image_back;
	ofPixels pixels_back;

	ofImage image_left;
	ofPixels pixels_left;

	ofImage image_right;
	ofPixels pixels_right;

	ofImage image_face;
	ofPixels pixels_face;


	bool drawing_back;
	bool drawing_right;
	bool drawing_left;
	bool drawing_face;

	ofxColorPicker colorpicker0;
	ofMesh meshGradient;

	ofxImGui::Gui gui;
};
