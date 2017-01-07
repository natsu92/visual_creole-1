#pragma once

#include "ofMain.h"

#include <wchar.h>
#include <direct.h>

#include "pxccapture.h"
#include "pxcsensemanager.h"
#include "pxccapturemanager.h"
#include "pxchandcursormodule.h"
#include "pxccursordata.h"
#include "pxchandconfiguration.h"
#include "pxchanddata.h"
#include "pxchandmodule.h"

#include "ofxImGui.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		//Visual Creole全体のState。起動直後のプロジェクト選択、録画、編集の３つ
		enum _VC_State {
			ENTRY, //起動直後はこれ。
			ENTRY2RECORD,
			RECORD,
			ENTRY2EDIT,
			EDIT
		} ;

		_VC_State VC_State;

		//録画画面でのステート
		enum R_State {
			WAITING, //
			READY,
			RECORDING,
			RECORDED //録画終了時。たぶんすぐ編集画面に遷移する
		} ;

		//再生画面でのステート。この時編集、ビューステートも同時に遷移する。
		enum P_State {
			STOP,//停止中。
			PLAYING, //再生中。
			PREVIEW //作ったパーツに対するプレビュー中、v0.1ではパーツの前後1秒の計3秒
		} ;

		//編集画面でのステート、編集中≒お絵かき中
		enum E_State {
			EDITING_NONE, //なにも編集してない。開始時や編集領域のOK押下後など。
			EDITING_LEFT, //左手編集中
			EDITING_RIGHT, //右手編集中
			EDITING_BACK, //背景編集中
		} ;

		//Assetsのビュー状態（画面右、中段のPNGの保管庫の状態を示す。
		enum V_State {
			VIEW_ALL,
			VIEW_LEFT, //
			VIEW_RIGHT,
			VIEW_BACK
		} ;

		typedef struct _part {
			int startframe; //描画が開始されるフレーム番号
			int endframe; //描画が停止されるフレーム番号、v0.1では開始30フレーム（1秒)後
			int offset_x; //画像のpx単位のオフセット、自分で描いた場合はあまり適用しないかも、外部のpngを読んだときに利用する。
			int offset_y;
			int size_percent; //画像の大きさを％表示。外部から読み込んだpngが大きかったり小さかったときに。
			//とりあえず%処理だけで動けるかどうかやってみる。というかv0.1では未実装になるかも。
			//int size_x; //読み込んだ画像のもともとの
			//int size_y;
			ofPixels picture;
		} part ;

		//動的配列vectorによる宣言
		std::vector<part> partsLeft;
		std::vector<part> partsRight;
		std::vector<part> partsBack;

		void initializeLive();
		void initializeCapture();
		void initializePlayer();

		void updateCamera();

		void drawPoints();

		ofxImGui::Gui gui;
};
