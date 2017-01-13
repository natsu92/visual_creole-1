#include "ofApp.h"



PXCSenseManager* senseManager = 0;
PXCHandModule *handModule;
PXCHandData *handData = 0;
PXCHandConfiguration* config;

const int COLOR_WIDTH = 640;
const int COLOR_HEIGHT = 480;
const int DEPTH_WIDTH = 640;
const int DEPTH_HEIGHT = 480;
const int FPS = 30;
const int capCalibrateX = 50;

ofRectangle camViewport;
ofPixels imagePixels;
ofTexture texture;

int running;

uint32_t frameNum;
uint32_t framePlayback;
ofVec2f rightHandAt;
ofVec2f leftHandAt;
ofVec2f fingerAt;


ofVec2f leftPointerThumbAt;//親指
ofVec2f rightPointerThumbAt;

ofVec2f leftPointerIndexAt;//人差し指
ofVec2f rightPointerIndexAt;

ofVec2f leftPointerMiddleAt;//中指
ofVec2f rightPointerMiddleAt;

ofVec2f leftPointerRingAt;//薬指
ofVec2f rightPointerRingAt;

ofVec2f leftPointerPinkyAt;//小指
ofVec2f rightPointerPinkyAt;

ofImage leftHandImage;
ofImage rightHandImage;


bool rightHandFound;
bool leftHandFound;

bool rightPointerThumbFound;//親指
bool leftPointerThumbFound;

bool rightPointerIndexFound;//人差し指
bool leftPointerIndexFound;

bool rightPointerMiddleFound;//中指
bool leftPointerMiddleFound;

bool rightPointerRingFound;//薬指
bool leftPointerRingFound;

bool rightPointerPinkyFound;//小指
bool leftPointerPinkyFound;

bool leftHandImageFrag;
bool rightHandImageFrag;

bool captureWindowFlag;
bool startMenuFlag;
bool projectControlFlag; //EDIT 1
bool playbackControlFlag; //EDIT 2
bool assetViewFlag; //EDIT 3
bool assetEditFlag; //EDIT 4


ofFileDialogResult result;

string path;
wstring widepath;
const wchar_t* path_wchar_t;

bool Pause;

//--------------------------------------------------------------
void ofApp::setup() {

	ofBackground(255);
	ofSetWindowShape(1980, 1080);
	ofSetFrameRate(FPS);

	gui.setup();

	camViewport.set(60, 60, 1280, 960);

	//描く仕組みを作る
	ofEnableAlphaBlending();
	ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);

	//Aをいれると透過する
	fbo.allocate(1280, 960,GL_RGBA);
	drawing = false;
	haconiwa = false;
	object = false;

	fbo1.allocate(60, 60, GL_RGB);

	//--------------------------------

	colorpicker0.setColorRadius(1.0);
	colorpicker0.setColorAngle(0.5);

	int x = 1380;
	int y = 0;
	int w = 150;
	int h = 300;
	int g = (int)((ofGetHeight() - h * 2) / 3);
	y = g;

	colorpicker0.setSize(x, y, w, h);

	y = y + h + g;


	meshGradient.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
	meshGradient.addVertex(ofVec3f(0, 0));
	meshGradient.addVertex(ofVec3f(ofGetWidth(), 0));
	meshGradient.addVertex(ofVec3f(ofGetWidth(), ofGetHeight()));
	meshGradient.addVertex(ofVec3f(0, ofGetHeight()));

	meshGradient.addColor(ofColor::white);
	meshGradient.addColor(ofColor::white);
	meshGradient.addColor(ofColor::white);
	meshGradient.addColor(ofColor::white);



	//--------------------------------


	VC_State = ENTRY;
	startMenuFlag = true;
	initializeLive();


}

void ofApp::initializeLive() {
	senseManager = PXCSenseManager::CreateInstance();
	senseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, COLOR_WIDTH, COLOR_HEIGHT, 0);
	senseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, DEPTH_WIDTH, DEPTH_HEIGHT, 0);

	senseManager->EnableHand();
	senseManager->Init();

	handModule = senseManager->QueryHand();
	handData = handModule->CreateOutput();

	config = handModule->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();
}

void ofApp::initializeCapture() {

	senseManager = PXCSenseManager::CreateInstance();

	senseManager->QueryCaptureManager()->SetFileName(path_wchar_t, true);
	// load your file at `path`
	senseManager->QueryCaptureManager()->SetRealtime(true);
	senseManager->QueryCaptureManager()->SetPause(Pause);

	senseManager->EnableHand();

	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_COLOR, COLOR_WIDTH, COLOR_HEIGHT, 30);
	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_DEPTH, DEPTH_WIDTH, DEPTH_HEIGHT, 30);

	senseManager->Init();

}

void ofApp::initializePlayer() {
	senseManager = PXCSenseManager::CreateInstance();
	if (senseManager == 0) {
		throw std::runtime_error("SenseManagerの生成に失敗しました");
	}
	wprintf(L"path:%s", path_wchar_t);
	senseManager->QueryCaptureManager()->SetFileName(path_wchar_t, false);
	// load your file at `path`


	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_COLOR, 640, 480, 30);
	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_DEPTH, 640, 480, 30);

	senseManager->EnableHand();

	senseManager->Init();

	senseManager->QueryCaptureManager()->SetRealtime(false);
	senseManager->QueryCaptureManager()->SetPause(Pause);

	handModule = senseManager->QueryHand();
	handData = handModule->CreateOutput();

	config = handModule->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();

	framePlayback = 0;
}


//--------------------------------------------------------------
void ofApp::update() {

	//colorUI
	colorpicker0.update();
	


	ofSetWindowTitle("Visual Creole   Framerate at " + ofToString(ofGetFrameRate(), 2));

	switch (VC_State) {
	case ENTRY:
		updateCamera();
		break;
	case RECORD:
		updateCamera();
		break;
	case EDIT:
		senseManager->QueryCaptureManager()->SetFrameByIndex(framePlayback);
		senseManager->FlushFrame();
		if (!Pause) framePlayback++;
		updateCamera();

		break;
	}



	
	//drawing
	fbo.begin();

	ofColor colorTop = colorpicker0.getColor();
	meshGradient.setColor(0, colorTop);
	meshGradient.setColor(1, colorTop);

	if (drawing)
	{
		ofSetColor(colorTop);
		ofDrawCircle(mouseLoc.x, mouseLoc.y, 5);
		ofDrawLine(lastmouseLoc.x, lastmouseLoc.y, mouseLoc.x, mouseLoc.y);
	}

	fbo.end();


	ofPixels pix;
	fbo.readToPixels(pix);
	image.setFromPixels(pix);




}

void ofApp::updateCamera() { //Liveに必要なもののみ。他に必要なものはほかでやろう
							 // This function blocks until a color sample is ready
	if (senseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR) {
		// Retrieve the sample
		PXCCapture::Sample *sample;
		//color
		sample = senseManager->QuerySample();
		PXCImage* sampleImage;
		sampleImage = sample->color;
		PXCImage::ImageData sampleData;
		if (sampleImage->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &sampleData) >= PXC_STATUS_NO_ERROR) {
			uint8_t* cBuffer = sampleData.planes[0];
			imagePixels.setFromPixels(cBuffer, 640, 480, 4);
			//printf("%s", sampleData.planes[0]);
			sampleImage->ReleaseAccess(&sampleData);
		}
		if (VC_State != RECORD) {
			//hand
			handData->Update();
			pxcUID handId;
			PXCHandData::IHand* hand;
			PXCHandData::JointData jointData;

			//left hand at first
			if (handData->QueryHandId(PXCHandData::ACCESS_ORDER_LEFT_HANDS, 0, handId) == PXC_STATUS_NO_ERROR) {
				handData->QueryHandDataById(handId, hand);
				auto center_l = hand->QueryMassCenterImage();
				leftHandAt.set(center_l.x, center_l.y);
				leftHandFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)5, jointData);
				leftPointerThumbAt.set(jointData.positionImage.x, jointData.positionImage.y);
				leftPointerThumbFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)9, jointData);
				leftPointerIndexAt.set(jointData.positionImage.x, jointData.positionImage.y);
				leftPointerIndexFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)13, jointData);
				leftPointerMiddleAt.set(jointData.positionImage.x, jointData.positionImage.y);
				leftPointerMiddleFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)17, jointData);
				leftPointerRingAt.set(jointData.positionImage.x, jointData.positionImage.y);
				leftPointerRingFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)21, jointData);
				leftPointerPinkyAt.set(jointData.positionImage.x, jointData.positionImage.y);
				leftPointerPinkyFound = true;
			}
			else {
				leftHandFound = false;
				leftPointerThumbFound = false;
				leftPointerIndexFound = false;
				leftPointerMiddleFound = false;
				leftPointerRingFound = false;
				leftPointerPinkyFound = false;
			}

			if (handData->QueryHandId(PXCHandData::ACCESS_ORDER_RIGHT_HANDS, 0, handId) == PXC_STATUS_NO_ERROR) {
				handData->QueryHandDataById(handId, hand);
				auto center_r = hand->QueryMassCenterImage();
				rightHandAt.set(center_r.x, center_r.y);
				rightHandFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)9, jointData);
				rightPointerIndexAt.set(jointData.positionImage.x, jointData.positionImage.y);
				rightPointerIndexFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)5, jointData);
				rightPointerThumbAt.set(jointData.positionImage.x, jointData.positionImage.y);
				rightPointerThumbFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)13, jointData);
				rightPointerMiddleAt.set(jointData.positionImage.x, jointData.positionImage.y);
				rightPointerMiddleFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)17, jointData);
				rightPointerRingAt.set(jointData.positionImage.x, jointData.positionImage.y);
				rightPointerRingFound = true;

				hand->QueryTrackedJoint((PXCHandData::JointType)21, jointData);
				rightPointerPinkyAt.set(jointData.positionImage.x, jointData.positionImage.y);
				rightPointerPinkyFound = true;

			}
			else {
				rightHandFound = false;
				rightPointerIndexFound = false;
				rightPointerThumbFound = false;
				rightPointerMiddleFound = false;
				rightPointerRingFound = false;
				rightPointerPinkyFound = false;
			}




		}
		senseManager->ReleaseFrame();
	}
	else { printf("Aquireframe error\n"); }
	texture.loadData(imagePixels.getPixels(), 640, 480, GL_BGRA);




}

//--------------------------------------------------------------
void ofApp::draw() {


	gui.begin();
	texture.draw(camViewport);


	fbo.draw(0, 0);
	colorpicker0.draw();
	ofDrawBitmapString(ofToString((int)ofGetFrameRate()), 10, 20);





	switch (VC_State) {

	case ENTRY:
		drawPoints();
		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowPos(ofVec2f(1400, 400), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Start Menu", &startMenuFlag);

		//todo 新規プロジェクト生成時とロード時で、フォルダ名を指定するのとファイルを指定するので扱いが異なってしまう。
		if (ImGui::Button("Create New Project"))
		{
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			result = ofSystemSaveDialog("Project", "Input Projectname");
			if (result.bSuccess) {
				path = result.getPath();
				Pause = true;
				int len = path.length();
				char* dname = new char[len + 1];
				memcpy(dname, path.c_str(), len + 1);
				_mkdir(dname);
				path.append("\\record.rssdk");
				cout << path << endl;
				for (int i = 0; i < path.length(); ++i) {
					widepath += wchar_t(path[i]);
				}
				path_wchar_t = widepath.c_str();
				wprintf(L"path:%s", path_wchar_t);
				initializeCapture();

				VC_State = RECORD;
			}
			else {
				initializeLive();
			}
		}
		if (ImGui::Button("Load and Play/Edit Project"))
		{
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			result = ofSystemLoadDialog("Select Project.rssdk");
			if (result.bSuccess) {

				path = result.getPath();
				for (int i = 0; i < path.length(); ++i)
					widepath += wchar_t(path[i]);
				path_wchar_t = (wchar_t*)widepath.c_str();
				Pause = true;
				initializePlayer();
				VC_State = EDIT;
			}
			else {
				initializeLive();
			}
		}
		ImGui::End();
		break;

	case RECORD:
		drawPoints();
		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowPos(ofVec2f(1400, 400), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Record Manager", &startMenuFlag);

		if (Pause == false) frameNum++;

		//todo 新規プロジェクト生成時とロード時で、フォルダ名を指定するのとファイルを指定するので扱いが異なってしまう。
		if (ImGui::Button("Start Recording"))
		{
			Pause = false;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		if (ImGui::Button("Stop Recording"))
		{
			Pause = true;
			//senseManager->QueryCaptureManager()->SetPause(Pause);
			VC_State = EDIT;
			senseManager->QueryCaptureManager()->CloseStreams();
			senseManager->Close();
			senseManager->Release();
			initializePlayer();
		}
		ImGui::Text("Recording Frame Count : %d", frameNum);
		ImGui::Text("Recording Time Duration : %d", frameNum / 30);

		ImGui::End();
		break;

	case EDIT:
		//drawPoints();
		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowPos(ofVec2f(1400, 400), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Project Control", &startMenuFlag);

		if (Pause == false) frameNum++;

		//todo 新規プロジェクト生成時とロード時で、フォルダ名を指定するのとファイルを指定するので扱いが異なってしまう。
		if (ImGui::Button("Assign Left Hand"))
		{
			result = ofSystemLoadDialog("Load Lefthand Image");
			if (result.bSuccess) {
				path = result.getPath();
				leftHandImage.loadImage(path);
				leftHandImage.resize(leftHandImage.getWidth() / 2, leftHandImage.getHeight() / 2);
				leftHandImageFrag = true;
			}
		}
		if (ImGui::Button("Assign Right Hand"))
		{
			result = ofSystemLoadDialog("Load Righthand Image");
			if (result.bSuccess) {
				path = result.getPath();
				rightHandImage.loadImage(path);
				rightHandImage.resize(rightHandImage.getWidth() / 2, rightHandImage.getHeight() / 2);
				rightHandImageFrag = true;
			}
		}

		ImGui::End();
		if (leftHandFound && leftHandImageFrag) {
			leftHandImage.draw(leftHandAt.x * 2 + 40 - leftHandImage.getWidth() / 2, leftHandAt.y * 2 + 60 - leftHandImage.getHeight() / 2);
		}
		if (rightHandFound && rightHandImageFrag) {
			rightHandImage.draw(rightHandAt.x * 2 + 60 - rightHandImage.getWidth() / 2, rightHandAt.y * 2 + 60 - rightHandImage.getHeight() / 2);

		}
		break;

		}

		gui.end();

	
}


void ofApp::drawPoints() {




	ofPushStyle();
	if (leftHandFound) {
		ofSetColor(255, 0, 0);leftHandAt.x * 2 + camViewport.x + capCalibrateX, 
		ofFill();
		ofRect(leftHandAt.x * 2 + camViewport.x + capCalibrateX, leftHandAt.y * 2 + camViewport.y, 30, 30);
		

		//drawing
		fbo1.begin();

		ofColor colorTop = colorpicker0.getColor();
		meshGradient.setColor(0, colorTop);
		meshGradient.setColor(1, colorTop);

		if (drawing)
		{
			ofSetColor(colorTop);
			ofDrawCircle(mouseLoc.x, mouseLoc.y, 5);
			ofDrawLine(lastmouseLoc.x, lastmouseLoc.y, mouseLoc.x, mouseLoc.y);
		}

		fbo1.end();



		ofPixels pix;
		fbo1.readToPixels(pix);
		image.setFromPixels(pix);

		fbo1.draw(0, 0);






	}
	if (rightHandFound) {
		ofSetColor(0, 0, 255);
		ofFill();
		ofRect(rightHandAt.x * 2 + camViewport.x + capCalibrateX, rightHandAt.y * 2 + camViewport.y, 30, 30);

	}

	if (leftPointerThumbFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(leftPointerThumbAt.x * 2 + camViewport.x + capCalibrateX, leftPointerThumbAt.y * 2 + camViewport.y, 10, 10);

	}

	if (rightPointerThumbFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(rightPointerThumbAt.x * 2 + camViewport.x + capCalibrateX, rightPointerThumbAt.y * 2 + camViewport.y, 10, 10);

	}

	if (leftPointerIndexFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(leftPointerIndexAt.x * 2 + camViewport.x + capCalibrateX, leftPointerIndexAt.y * 2 + camViewport.y, 10, 10);

	}

	if (rightPointerIndexFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(rightPointerIndexAt.x * 2 + camViewport.x + capCalibrateX, rightPointerIndexAt.y * 2 + camViewport.y, 10, 10);

	}

	if (leftPointerIndexFound && rightPointerIndexFound) {
		ofSetColor(255, 255, 0);
		ofDrawLine(leftPointerIndexAt.x * 2 + camViewport.x + capCalibrateX, leftPointerIndexAt.y * 2 + camViewport.y, rightPointerIndexAt.x * 2 + camViewport.x + capCalibrateX, rightPointerIndexAt.y * 2 + camViewport.y);

	}

	if (leftPointerMiddleFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(leftPointerMiddleAt.x * 2 + camViewport.x + capCalibrateX, leftPointerMiddleAt.y * 2 + camViewport.y, 10, 10);

	}

	if (rightPointerMiddleFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(rightPointerMiddleAt.x * 2 + camViewport.x + capCalibrateX, rightPointerMiddleAt.y * 2 + camViewport.y, 10, 10);

	}

	if (leftPointerRingFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(leftPointerRingAt.x * 2 + camViewport.x + capCalibrateX, leftPointerRingAt.y * 2 + camViewport.y, 10, 10);

	}

	if (rightPointerRingFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(rightPointerRingAt.x * 2 + camViewport.x + capCalibrateX, rightPointerRingAt.y * 2 + camViewport.y, 10, 10);

	}

	if (leftPointerPinkyFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(leftPointerPinkyAt.x * 2 + camViewport.x + capCalibrateX, leftPointerPinkyAt.y * 2 + camViewport.y, 10, 10);

	}

	if (rightPointerPinkyFound) {
		ofSetColor(0, 255, 255);
		ofFill();
		ofRect(rightPointerPinkyAt.x * 2 + camViewport.x + capCalibrateX, rightPointerPinkyAt.y * 2 + camViewport.y, 10, 10);

	}

	ofPopStyle();
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	Pause = !Pause;
	//senseManager->QueryCaptureManager()->SetPause(Pause);

	if (key == 's') {
		image.save("my file_" + ofGetTimestampString() + ".png", OF_IMAGE_QUALITY_BEST);
	}


	switch (key) {
	case 'f':
		ofToggleFullscreen();
		break;
	case 'c':


		
		break;

	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	
	drawing = false;
	mouseLoc.x = x;
	mouseLoc.y = y;
	



}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	drawing = true;

	lastmouseLoc = mouseLoc;
	mouseLoc.set(x, y);

	}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	drawing = false;

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
