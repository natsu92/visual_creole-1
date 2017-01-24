#include "ofApp.h"

PXCSenseManager* senseManager = 0;
PXCHandModule *handModule;
PXCHandData *handData = 0;
PXCHandConfiguration* config;
PXCFaceModule *faceModule;
PXCFaceConfiguration *faceConfig;
PXCFaceData *faceData;
PXCProjection *projection;

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
ofVec2f leftPointerAt;
ofVec2f rightPointerAt;
ofImage leftHandImage, rightHandImage, backImage, headImage;
ofImage leftHandImage_ld, rightHandImage_ld, backImage_ld, headImage_ld;
bool rightHandFound, leftHandFound;
bool leftHandImageflag, rightHandImageflag, backImageflag, headImageflag;
bool leftPointerFound;
bool rightPointerFound;

ofVec2f leftEyeAt, rightEyeAt, mouseAt;
bool faceFound;

int isrightAssign2Tip;
int isleftAssign2Tip;
int flocation = 0;

bool isColorPointsEnable;

int rightHandImageSize;
int leftHandImageSize;
int backImageSize;//追加するイメージの情報をまとめてきれいにする
int headImageSize;
ofVec2f backImageAt;
int backImageOffsetX, backImageOffsetY, rightHandImageOffsetX, rightHandImageOffsetY,
leftHandImageOffsetX, leftHandImageOffsetY, faceImageOffsetX, faceImageOffsetY;
//TODO ここらへんの画像系変数をクラスにまとめ、left,right,back,faceの4いんすたんすで管理出来るようにする。

bool windowcloserflag;

bool captureWindowFlag;
bool startMenuFlag;
bool projectControlFlag; //EDIT 1
bool playbackControlFlag; //EDIT 2
bool assetViewFlag; //EDIT 3
bool assetEditFlag; //EDIT 4

ofFileDialogResult result;

string path;
string dir;
wstring widepath;
const wchar_t* path_wchar_t;

bool Pause;

ImGuiWindowFlags window_flags = 0;


//--------------------------------------------------------------
void ofApp::setup() {

	ofBackground(180);
	ofSetWindowShape(1980, 1080);
	ofSetFrameRate(FPS);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("data\\YuGothM001.TTF", 24.0f, nullptr, nullptr);//フォント、文字サイズ

	gui.setup();

	//描く仕組みを作る
	ofEnableAlphaBlending();
	ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);


	fbo_back.allocate(1280, 960, GL_RGBA);
	drawing_back = false;

	fbo_right.allocate(1280, 960, GL_RGBA);
	drawing_right = false;


	fbo_left.allocate(1280, 960, GL_RGBA);
	drawing_left = false;


	fbo_face.allocate(1280, 960, GL_RGBA);//サイズと色
	drawing_face = false;

	//------------------------------------------------
	colorpicker0.setColorRadius(1.0);
	colorpicker0.setColorAngle(0.5);

	int x = 1380;
	int y = 500;
	int w = 150 * 2;
	int h = 300 * 2;
	//	int g = (int)((ofGetHeight() - h * 2) / 3);
	//	y = g;

	colorpicker0.setSize(x, y, w, h);

	//	y = y + h + g;

	meshGradient.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
	meshGradient.addVertex(ofVec3f(0, 0));
	meshGradient.addVertex(ofVec3f(ofGetWidth(), 0));
	meshGradient.addVertex(ofVec3f(ofGetWidth(), ofGetHeight()));
	meshGradient.addVertex(ofVec3f(0, ofGetHeight()));

	meshGradient.addColor(ofColor::white);
	meshGradient.addColor(ofColor::white);
	meshGradient.addColor(ofColor::white);
	meshGradient.addColor(ofColor::white);
	//------------------------------------------------

	static bool no_titlebar = false;
	static bool no_border = true;
	static bool no_resize = true;
	static bool no_move = true;
	static bool no_scrollbar = false;
	static bool no_collapse = true;
	static bool no_menu = true;
	if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
	if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
	if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
	if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
	if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
	if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;

	camViewport.set(60, 60, 1280, 960);

	windowcloserflag = false;

	VC_State = ENTRY;
	startMenuFlag = true;
	initializeLive();

	rightHandImageSize = 100;
	leftHandImageSize = 100;
	backImageSize = 100;
	headImageSize = 100;

}

void ofApp::initializeLive() {
	senseManager = PXCSenseManager::CreateInstance();
	senseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, COLOR_WIDTH, COLOR_HEIGHT, 0);
	senseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, DEPTH_WIDTH, DEPTH_HEIGHT, 0);

	senseManager->EnableHand();
	senseManager->EnableFace();

	senseManager->Init();

	auto device = senseManager->QueryCaptureManager()->QueryDevice();

	projection = device->CreateProjection();

	handModule = senseManager->QueryHand();
	handData = handModule->CreateOutput();

	faceModule = senseManager->QueryFace();
	faceData = faceModule->CreateOutput();

	config = handModule->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();

	faceConfig = faceModule->CreateActiveConfiguration();
	faceConfig->SetTrackingMode(PXCFaceConfiguration::TrackingModeType::FACE_MODE_COLOR_PLUS_DEPTH);
}

void ofApp::initializeCapture() {

	senseManager = PXCSenseManager::CreateInstance();

	senseManager->QueryCaptureManager()->SetFileName(path_wchar_t, true);
	// load your file at `path`
	senseManager->QueryCaptureManager()->SetRealtime(true);
	senseManager->QueryCaptureManager()->SetPause(Pause);

	senseManager->EnableHand();
	senseManager->EnableFace();

	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_COLOR, COLOR_WIDTH, COLOR_HEIGHT, 30);
	senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_DEPTH, DEPTH_WIDTH, DEPTH_HEIGHT, 30);

	senseManager->Init();

	handModule = senseManager->QueryHand();
	handData = handModule->CreateOutput();

	faceModule = senseManager->QueryFace();
	faceData = faceModule->CreateOutput();

	config = handModule->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();

	faceConfig = faceModule->CreateActiveConfiguration();
	faceConfig->SetTrackingMode(PXCFaceConfiguration::TrackingModeType::FACE_MODE_COLOR_PLUS_DEPTH);

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
	senseManager->EnableFace();

	senseManager->Init();

	senseManager->QueryCaptureManager()->SetRealtime(false);
	senseManager->QueryCaptureManager()->SetPause(Pause);

	handModule = senseManager->QueryHand();
	handData = handModule->CreateOutput();

	faceModule = senseManager->QueryFace();
	faceData = faceModule->CreateOutput();

	config = handModule->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();

	faceConfig = faceModule->CreateActiveConfiguration();
	faceConfig->SetTrackingMode(PXCFaceConfiguration::TrackingModeType::FACE_MODE_COLOR_PLUS_DEPTH);

	framePlayback = 0;
}


void ofApp::initializeLoadedValue() {
	cout << dir << endl;
	ifstream ifs(dir + "offsets.csv");

	if (!ifs) {
		cout << "input err" << endl;
	}
	else {
		//csvファイルを1行ずつ読み込む
		string str;
		string token;
		istringstream stream;
		//istringstream stream;
		//1st, left hand
		getline(ifs, str);
		stream.str("");
		stream.clear(stringstream::goodbit);
		stream.str(str);
		getline(stream, token, ','); leftHandImageflag = stoi(token);
		getline(stream, token, ','); isleftAssign2Tip = stoi(token);
		getline(stream, token, ','); leftHandImageOffsetX = stoi(token);
		getline(stream, token, ','); leftHandImageOffsetY = stoi(token);
		getline(stream, token, ','); leftHandImageSize = stoi(token);
		if (leftHandImageflag) {
			leftHandImage_ld.loadImage(dir + "left.png");
		}
		//cout << stream << endl;
		//2nd line right hand
		getline(ifs, str);
		stream.str("");
		stream.clear(stringstream::goodbit);
		stream.str(str);
		getline(stream, token, ','); rightHandImageflag = stoi(token);
		getline(stream, token, ','); isrightAssign2Tip = stoi(token);
		getline(stream, token, ','); rightHandImageOffsetX = stoi(token);
		getline(stream, token, ','); rightHandImageOffsetY = stoi(token);
		getline(stream, token, ','); rightHandImageSize = stoi(token);
		if (rightHandImageflag) {
			rightHandImage_ld.loadImage(dir + "right.png");
		}
		//3rd line face
		getline(ifs, str);
		stream.str("");
		stream.clear(stringstream::goodbit);
		stream.str(str);
		getline(stream, token, ','); headImageflag = stoi(token);
		getline(stream, token, ','); flocation = stoi(token);
		getline(stream, token, ','); faceImageOffsetX = stoi(token);
		getline(stream, token, ','); faceImageOffsetY = stoi(token);
		getline(stream, token, ','); headImageSize = stoi(token);
		if (headImageflag) {
			headImage_ld.loadImage(dir + "head.png");
		}
		//3rd back
		getline(ifs, str);
		stream.str("");
		stream.clear(stringstream::goodbit);
		stream.str(str);
		getline(stream, token, ','); backImageflag = stoi(token);
		getline(stream, token, ',');
		getline(stream, token, ','); backImageOffsetX = stoi(token);
		getline(stream, token, ','); backImageOffsetY = stoi(token);
		getline(stream, token, ','); backImageSize = stoi(token);
		if (backImageflag) {
			backImage_ld.loadImage(dir + "back.png");
		}
	}
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
	fbo_back.begin();

	ofColor colorTop = colorpicker0.getColor();
	meshGradient.setColor(0, colorTop);
	meshGradient.setColor(1, colorTop);

	if (drawing_back)
	{
		ofSetColor(colorTop);
		ofDrawCircle(mouseLoc_back.x, mouseLoc_back.y, 5);
		ofSetLineWidth(6);
		ofDrawLine(lastmouseLoc_back.x, lastmouseLoc_back.y, mouseLoc_back.x, mouseLoc_back.y);
	}
	fbo_back.end();

	ofPixels pix_back;
	fbo_back.readToPixels(pix_back);
	image_back.setFromPixels(pix_back);

	fbo_left.begin();
	ofColor color_left = colorpicker0.getColor();
	meshGradient.setColor(0, color_left);
	meshGradient.setColor(1, color_left);
	if (drawing_left)
	{
		ofSetColor(color_left);
		ofDrawCircle(mouseLoc_left.x, mouseLoc_left.y, 2);
		ofSetLineWidth(2);
		ofDrawLine(lastmouseLoc_left.x, lastmouseLoc_left.y, mouseLoc_left.x, mouseLoc_left.y);
	}
	fbo_left.end();

	ofPixels pix_left;
	fbo_left.readToPixels(pix_left);
	image_left.setFromPixels(pix_left);

	ofColor color_right = colorpicker0.getColor();
	fbo_right.begin();
	meshGradient.setColor(0, color_right);
	meshGradient.setColor(1, color_right);
	if (drawing_right)
	{
		ofSetColor(color_right);
		ofDrawCircle(mouseLoc_right.x, mouseLoc_right.y, 5);
		ofSetLineWidth(5);
		ofDrawLine(lastmouseLoc_right.x, lastmouseLoc_right.y, mouseLoc_right.x, mouseLoc_right.y);
	}
	fbo_right.end();
	ofPixels pix_right;
	fbo_right.readToPixels(pix_right);
	image_right.setFromPixels(pix_right);

	ofColor color_face = colorpicker0.getColor();
	fbo_face.begin();
	meshGradient.setColor(0, color_face);
	meshGradient.setColor(1, color_face);
	if (drawing_face)
	{
		ofSetColor(color_face);
		ofDrawCircle(mouseLoc_face.x, mouseLoc_face.y, 5);
		//線幅変えたい・・・・GUI作る
		ofSetLineWidth(5);
		ofDrawLine(lastmouseLoc_face.x, lastmouseLoc_face.y, mouseLoc_face.x, mouseLoc_face.y);
	}
	fbo_face.end();
	ofPixels pix_face;
	fbo_face.readToPixels(pix_face);
	image_face.setFromPixels(pix_face);
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
		//hand
		handData->Update();
		pxcUID handId;
		PXCHandData::IHand* hand;
		PXCHandData::JointData jointData;
		PXCPointF32 colorPoint;
		//left hand at first
		if (handData->QueryHandId(PXCHandData::ACCESS_ORDER_LEFT_HANDS, 0, handId) == PXC_STATUS_NO_ERROR) {
			handData->QueryHandDataById(handId, hand);
			//auto center_l = hand->QueryMassCenterImage();
			///leftHandAt.set(center_l.x, center_l.y);

			hand->QueryTrackedJoint((PXCHandData::JointType)1, jointData);
			colorPoint = { 0 };
			auto depthPoint = jointData.positionImage;
			depthPoint.z = jointData.positionWorld.z * 1000;
			projection->MapDepthToColor(1, &depthPoint, &colorPoint);
			leftHandAt.set(colorPoint.x, colorPoint.y);

			hand->QueryTrackedJoint((PXCHandData::JointType)9, jointData);
			colorPoint = { 0 };
			depthPoint = jointData.positionImage;
			depthPoint.z = jointData.positionWorld.z * 1000;
			projection->MapDepthToColor(1, &depthPoint, &colorPoint);
			leftPointerAt.set(colorPoint.x, colorPoint.y);

			if (leftHandAt.x != -1.0f && leftHandAt.y != -1.0f && leftPointerAt.x != -1.0f && leftPointerAt.y != -1.0f) {
				leftHandFound = true;
				leftPointerFound = true;
				printf("left, %f\n", leftHandAt.x);
			}
			else
			{
				leftHandFound = false;
				leftPointerFound = false;
			}
		}
		else {
			leftHandFound = false;
			leftPointerFound = false;
		}

		if (handData->QueryHandId(PXCHandData::ACCESS_ORDER_RIGHT_HANDS, 0, handId) == PXC_STATUS_NO_ERROR) {
			handData->QueryHandDataById(handId, hand);
			/*
			auto center_r = hand->QueryMassCenterImage();
			rightHandAt.set(center_r.x, center_r.y);
			rightHandFound = true;
			rightPointerFound = true;
			hand->QueryTrackedJoint((PXCHandData::JointType)9, jointData);
			rightPointerAt.set(jointData.positionImage.x, jointData.positionImage.y);
			*/
			hand->QueryTrackedJoint((PXCHandData::JointType)1, jointData);
			colorPoint = { 0 };
			auto depthPoint = jointData.positionImage;
			depthPoint.z = jointData.positionWorld.z * 1000;
			projection->MapDepthToColor(1, &depthPoint, &colorPoint);
			rightHandAt.set(colorPoint.x, colorPoint.y);

			hand->QueryTrackedJoint((PXCHandData::JointType)9, jointData);
			colorPoint = { 0 };
			depthPoint = jointData.positionImage;
			depthPoint.z = jointData.positionWorld.z * 1000;
			projection->MapDepthToColor(1, &depthPoint, &colorPoint);
			rightPointerAt.set(colorPoint.x, colorPoint.y);

			if (rightHandAt.x != -1.0f && rightHandAt.y != -1.0f && rightPointerAt.x != -1.0f && rightPointerAt.y != -1.0f) {
				rightHandFound = true;
				rightPointerFound = true;
				printf("right, %f\n", rightHandAt.x);
			}
			else
			{
				rightHandFound = false;
				rightPointerFound = false;
			}
		}
		else {
			rightHandFound = false;
			rightPointerFound = false;
		}

		//FACE
		faceData->Update();
		pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
		//		for (pxcI32 i = 0; i < nfaces; i++) {
		PXCFaceData::Face *face = faceData->QueryFaceByIndex(0);
		if (face != nullptr) {
			PXCFaceData::LandmarksData *ldata = face->QueryLandmarks();
			if (ldata != nullptr) {
				pxcI32 npoints = ldata->QueryNumPoints();
				PXCFaceData::LandmarkPoint *points = new PXCFaceData::LandmarkPoint[npoints];
				ldata->QueryPointsByGroup(
					PXCFaceData::LandmarksGroupType::LANDMARK_GROUP_LEFT_EYE, points);
				leftEyeAt.set(points[6].image.x, points[6].image.y);
				ldata->QueryPointsByGroup(
					PXCFaceData::LandmarksGroupType::LANDMARK_GROUP_RIGHT_EYE, points);
				rightEyeAt.set(points[6].image.x, points[6].image.y);
				ldata->QueryPointsByGroup(
					PXCFaceData::LandmarksGroupType::LANDMARK_GROUP_MOUTH, points);
				//printf("lefteye:%f,%f\n", points[0].image.x, points[0].image.y);
				mouseAt.set(points[3].image.x, points[3].image.y);
				faceFound = true;
			}
			else faceFound = false;
		}
		else faceFound = false;
		//		}
		senseManager->ReleaseFrame();
	}
	else {
		printf("Aquireframe error\n");
		Pause = true;
		framePlayback--;
		senseManager->QueryCaptureManager()->SetPause(Pause);
	}
	texture.loadData(imagePixels.getPixels(), 640, 480, GL_BGRA);
}

//--------------------------------------------------------------
void ofApp::draw() {
	gui.begin();
	texture.draw(camViewport);

	colorpicker0.draw();
	ofDrawBitmapString(ofToString((int)ofGetFrameRate()), 10, 20);

	fbo_back.draw(0, 0);

	switch (VC_State) {

	case ENTRY:
		drawPoints();
		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always); //GUIのカラムサイズ
		ImGui::SetNextWindowPos(ofVec2f(1380, 50), ImGuiSetCond_Always);
		ImGui::Begin("Start Menu", 0, window_flags);

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
				dir = path + "\\";
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
				dir = path;
				dir.erase(dir.length() - 12);
				//printf("%s\n",dir);
				cout << dir << endl;
				for (int i = 0; i < path.length(); ++i)
					widepath += wchar_t(path[i]);
				path_wchar_t = (wchar_t*)widepath.c_str();
				Pause = true;
				initializePlayer();
				VC_State = EDIT;
				initializeLoadedValue();
			}
			else {
				initializeLive();
			}
		}
		ImGui::End();
		break;

	case RECORD:
		if (Pause == false) frameNum++;

		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ofVec2f(1380, 50), ImGuiSetCond_Always);
		ImGui::Begin("Record Manager", 0, window_flags);

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
		ImGui::Checkbox("PointsEnable", &isColorPointsEnable);
		if (isColorPointsEnable) {
			drawPoints();
		}
		ImGui::End();
		drawPicCtrl();
		drawImages();
		break;

	case EDIT:
		//drawPoints();
		if (Pause == false) frameNum++;

		ImGui::SetNextWindowSize(ofVec2f(500, 200), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ofVec2f(1380, 50), ImGuiSetCond_Always);
		ImGui::Begin("Playback Manager", 0, window_flags);

		if (ImGui::Button("Start"))
		{
			Pause = false;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
		{
			Pause = true;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			framePlayback = 0;
			Pause = true;
			senseManager->QueryCaptureManager()->SetPause(Pause);
		}
		ImGui::SameLine();

		if (ImGui::Button("Save Project"))
		{
			leftHandImage_ld.saveImage(dir + "left.png", OF_IMAGE_QUALITY_HIGH);
			rightHandImage_ld.saveImage(dir + "right.png", OF_IMAGE_QUALITY_HIGH);
			headImage_ld.saveImage(dir + "head.png", OF_IMAGE_QUALITY_HIGH);
			backImage_ld.saveImage(dir + "back.png", OF_IMAGE_QUALITY_HIGH);
			ofstream ofs(dir + "offsets.csv");
			ofs << (int)leftHandImageflag << ',' << isleftAssign2Tip << ',' << leftHandImageOffsetX << ',' << leftHandImageOffsetY << ',' << leftHandImageSize << ',' << endl;
			ofs << (int)rightHandImageflag << ',' << isrightAssign2Tip << ',' << rightHandImageOffsetX << ',' << rightHandImageOffsetY << ',' << rightHandImageSize << ',' << endl;
			ofs << (int)headImageflag << ',' << flocation << ',' << faceImageOffsetX << ',' << faceImageOffsetY << ',' << headImageSize << ',' << endl;
			ofs << (int)backImageflag << ",," << backImageOffsetX << ',' << backImageOffsetY << ',' << backImageSize << ',' << endl;

		}

		if (ImGui::Button("-3s"))
		{
			framePlayback -= 90;
		}
		ImGui::SameLine();
		if (ImGui::Button("-1s"))
		{
			framePlayback -= 30;
		}
		ImGui::SameLine();
		if (ImGui::Button("+1s"))
		{
			framePlayback += 30;
		}
		ImGui::SameLine();
		if (ImGui::Button("+3s"))
		{
			framePlayback += 90;
		}

		ImGui::Text("Playback Frame Count : %d", framePlayback);
		ImGui::Text("Playback Time Duration : %d", framePlayback / 30);
		ImGui::Checkbox("PointsEnable", &isColorPointsEnable);
		if (isColorPointsEnable) {
			drawPoints();
		}
		ImGui::End();
		drawPicCtrl();
		drawImages();
		break;
	}
	gui.end();
}


void ofApp::drawPicCtrl() {

	ImGui::SetNextWindowSize(ofVec2f(500, 300), ImGuiSetCond_Always);
	ImGui::SetNextWindowPos(ofVec2f(1380, 280), ImGuiSetCond_Always);
	ImGui::Begin("Picture Control", &startMenuFlag);
	//ImGui::RadioButton("")
	static int e = 0;
	ImGui::RadioButton("Left Hand", &e, 0); ImGui::SameLine();
	ImGui::RadioButton("Right Hand", &e, 1); ImGui::SameLine();
	ImGui::RadioButton("Head", &e, 2); ImGui::SameLine();
	ImGui::RadioButton("Back", &e, 3);
	ImGui::NewLine();

	switch (e) {
	case 0:
		if (ImGui::Button("Assign Picture"))
		{
			result = ofSystemLoadDialog("Load Lefthand Image");//ファイルのパス名UIを組み立てつつ
			if (result.bSuccess) {
				path = result.getPath();
				leftHandImage_ld.loadImage(path);
				leftHandImageflag = true;
			}
		}

		ImGui::SameLine();
		ImGui::Checkbox("Enable", &leftHandImageflag);
		ImGui::SameLine();
		if (ImGui::Button("default"))
		{
			isleftAssign2Tip = 0;
			leftHandImageSize = 100;
			leftHandImageOffsetX = 0;
			leftHandImageOffsetY = 0;
		}
		//ImGui::Checkbox("at Tip", &isleftAssign2Tip);
		ImGui::RadioButton("at Palm", &isleftAssign2Tip, 0); ImGui::SameLine();
		ImGui::RadioButton("at Tip", &isleftAssign2Tip, 1);
		ImGui::SliderInt("Size", &leftHandImageSize, 10, 500, nullptr);
		//下のものはドラックアンドドロップにて修正できるようにする
		ImGui::SliderInt("X offset", &leftHandImageOffsetX, -640, 640, nullptr);
		ImGui::SliderInt("Y offset", &leftHandImageOffsetY, -480, 480, nullptr);

		break;
	case 1:
		if (ImGui::Button("Assign Picture"))
		{
			result = ofSystemLoadDialog("Load Righthand Image");
			if (result.bSuccess) {
				path = result.getPath();
				rightHandImage_ld.loadImage(path);
				rightHandImageflag = true;
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Enable", &rightHandImageflag);
		ImGui::SameLine();
		if (ImGui::Button("default"))
		{
			isrightAssign2Tip = 0;
			rightHandImageSize = 100;
			rightHandImageOffsetX = 0;
			rightHandImageOffsetY = 0;
		}
		//ImGui::Checkbox("at Tip", &isrightAssign2Tip);
		ImGui::RadioButton("at Palm", &isrightAssign2Tip, 0); ImGui::SameLine();
		ImGui::RadioButton("at Tip", &isrightAssign2Tip, 1);
		ImGui::SliderInt("Size", &rightHandImageSize, 10, 500, nullptr);

		//ドラックアンドドロップでつくりたい
		ImGui::SliderInt("X offset", &rightHandImageOffsetX, -640, 640, nullptr);
		ImGui::SliderInt("Y offset", &rightHandImageOffsetY, -480, 480, nullptr);

		break;
	case 2:
		if (ImGui::Button("Assign Picture"))
		{
			result = ofSystemLoadDialog("Load Head Image");
			if (result.bSuccess) {
				path = result.getPath();
				headImage_ld.loadImage(path);
				//rightHandImage.resize(rightHandImage.getWidth() / 2, rightHandImage.getHeight() / 2);
				headImageflag = true;
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Enable", &headImageflag);
		ImGui::SameLine();
		if (ImGui::Button("default"))
		{
			flocation = 0;
			headImageSize = 100;
			faceImageOffsetX = 0;
			faceImageOffsetY = 0;
		}

		ImGui::RadioButton("mouse", &flocation, 0); ImGui::SameLine();
		ImGui::RadioButton("lefteye", &flocation, 1); ImGui::SameLine();
		ImGui::RadioButton("righteye", &flocation, 2);
		ImGui::SliderInt("Size", &headImageSize, 10, 500, nullptr);
		ImGui::SliderInt("X offset", &faceImageOffsetX, -640, 640, nullptr);
		ImGui::SliderInt("Y offset", &faceImageOffsetY, -480, 480, nullptr);
		break;
	case 3:
		if (ImGui::Button("Assign Picture"))
		{
			result = ofSystemLoadDialog("Load BackGround Image");
			if (result.bSuccess) {
				path = result.getPath();
				backImage_ld.loadImage(path);
				//backImage.resize(backImage.getWidth() / 2, backImage.getHeight() / 2);
				backImageflag = true;
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Enable", &backImageflag);
		ImGui::SameLine();
		if (ImGui::Button("default"))
		{
			backImageSize = 100;
			backImageOffsetX = 0;
			backImageOffsetY = 0;
		}
		ImGui::NewLine();
		ImGui::SliderInt("Size", &backImageSize, 10, 500, nullptr);
		ImGui::SliderInt("X offset", &backImageOffsetX, -640, 640, nullptr);
		ImGui::SliderInt("Y offset", &backImageOffsetY, -480, 480, nullptr);
		backImageAt.set(backImageOffsetX, backImageOffsetY);
	}
	ImGui::End();
}

void ofApp::drawPoints() {
	ofPushStyle();
	if (leftHandFound) {
		ofSetColor(255, 0, 0);
		ofFill();
		ofRect(leftHandAt.x * 2 + camViewport.x, leftHandAt.y * 2 + camViewport.y, 30, 30);
		ofSetColor(255, 100, 100);
		ofFill();
		ofRect(leftPointerAt.x * 2 + camViewport.x, leftPointerAt.y * 2 + camViewport.y, 10, 10);
		fbo_left.draw(leftHandAt.x * 2 + camViewport.x - 200, leftHandAt.y * 2 + camViewport.y - 300);
	}

	if (rightHandFound) {
		ofSetColor(0, 0, 255);
		ofFill();
		ofRect(rightHandAt.x * 2 + camViewport.x, rightHandAt.y * 2 + camViewport.y, 30, 30);
		ofSetColor(100, 100, 255);
		ofFill();
		ofRect(rightPointerAt.x * 2 + camViewport.x, rightPointerAt.y * 2 + camViewport.y, 10, 10);
		fbo_right.draw(rightHandAt.x * 2 + camViewport.x - 200, rightHandAt.y * 2 + camViewport.y - 300);
	}
	if (faceFound) {
		ofSetColor(0, 255, 0);
		ofFill();
		ofRect(rightEyeAt.x * 2 + camViewport.x, rightEyeAt.y * 2 + camViewport.y, 10, 10);
		ofRect(leftEyeAt.x * 2 + camViewport.x, leftEyeAt.y * 2 + camViewport.y, 10, 10);
		ofRect(mouseAt.x * 2 + camViewport.x, mouseAt.y * 2 + camViewport.y, 10, 10);
		fbo_face.draw(mouseAt.x * 2 + camViewport.x - 400, mouseAt.y * 2 + camViewport.y - 500);
	}

	ofPopStyle();
}

void ofApp::drawImages() {
	if (leftHandFound && leftHandImageflag) {
		leftHandImage.clone(leftHandImage_ld);
		leftHandImage.resize(leftHandImageSize, leftHandImage_ld.getHeight()*leftHandImageSize / leftHandImage_ld.getWidth());
		if (isleftAssign2Tip) {
			leftHandImage.draw(leftPointerAt.x * 2 + 40 - leftHandImage.getWidth() / 2 + leftHandImageOffsetX, leftPointerAt.y * 2 + 60 - leftHandImage.getHeight() / 2 + leftHandImageOffsetY);
		}
		else {
			leftHandImage.draw(leftHandAt.x * 2 + 40 - leftHandImage.getWidth() / 2 + leftHandImageOffsetX, leftHandAt.y * 2 + 60 - leftHandImage.getHeight() / 2 + leftHandImageOffsetY);
		}
	}
	if (rightHandFound && rightHandImageflag) {
		rightHandImage.clone(rightHandImage_ld);
		rightHandImage.resize(rightHandImageSize, rightHandImage_ld.getHeight()*rightHandImageSize / rightHandImage_ld.getWidth());
		if (isrightAssign2Tip) {

			//pushmarix
			rightHandImage.draw(rightPointerAt.x * 2 + 60 - rightHandImage.getWidth() / 2 + rightHandImageOffsetX, rightPointerAt.y * 2 + 60 - rightHandImage.getHeight() / 2 + rightHandImageOffsetY);
			//popmatrix

		}
		else {
			rightHandImage.draw(rightHandAt.x * 2 + 60 - rightHandImage.getWidth() / 2 + rightHandImageOffsetX, rightHandAt.y * 2 + 60 - rightHandImage.getHeight() / 2 + rightHandImageOffsetY);
		}
	}
	if (backImageflag) {
		backImage.clone(backImage_ld);
		backImage.resize(backImageSize, backImage_ld.getHeight()*backImageSize / backImage_ld.getWidth());
		backImage.draw(backImageAt.x * 2 + 60 - backImage.getWidth() / 2 + backImageOffsetX, backImageAt.y * 2 + 60 - backImage.getHeight() / 2 + backImageOffsetY);
	}

	if (headImageflag) {
		headImage.clone(headImage_ld);
		headImage.resize(headImageSize, headImage_ld.getHeight()*headImageSize / headImage_ld.getWidth());
		if (flocation == 0) {
			headImage.draw(mouseAt.x * 2 + 60 - headImage.getWidth() / 2 + faceImageOffsetX, mouseAt.y * 2 + 60 - headImage.getHeight() / 2, faceImageOffsetY);
		}
		else if (flocation == 1) {
			headImage.draw(leftEyeAt.x * 2 + 60 - headImage.getWidth() / 2 + faceImageOffsetX, leftEyeAt.y * 2 + 60 - headImage.getHeight() / 2 + faceImageOffsetY);
		}
		else if (flocation == 2) {
			headImage.draw(rightEyeAt.x * 2 + 60 - headImage.getWidth() / 2 + faceImageOffsetX, rightEyeAt.y * 2 + 60 - headImage.getHeight() / 2 + faceImageOffsetY);
		}
	}
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	Pause = !Pause;
	//senseManager->QueryCaptureManager()->SetPause(Pause);

	if (key == 's') {
		image_back.save("my file_" + ofGetTimestampString() + ".png", OF_IMAGE_QUALITY_BEST);
	}

	switch (key) {
	case 'f':
		ofToggleFullscreen();
		break;
	}
	switch (key) {
	case 'q':

		drawing_back = false;
		fbo_back.begin();
		ofClear(255, 255, 255, 0);
		fbo_back.end();

		break;

	case 'w':

		drawing_right = false;
		fbo_right.begin();
		ofClear(255, 255, 255, 0);
		fbo_right.end();

		break;

	case 'e':

		drawing_left = false;
		fbo_left.begin();
		ofClear(255, 255, 255, 0);
		fbo_left.end();

		break;

	case 'r':

		drawing_face = false;
		fbo_face.begin();
		ofClear(255, 255, 255, 0);
		fbo_face.end();

		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	drawing_back = false;
	drawing_left = false;
	drawing_right = false;
	drawing_face = false;

	mouseLoc_back.x = x;
	mouseLoc_back.y = y;

	mouseLoc_right.x = x - (rightHandAt.x * 2 + camViewport.x) - 200;
	mouseLoc_right.y = y - (rightHandAt.y * 2 + camViewport.y) - 300;

	mouseLoc_left.x = x - (leftHandAt.x * 2 + camViewport.x - 200);
	mouseLoc_left.y = y - (leftHandAt.y * 2 + camViewport.y - 300);

	mouseLoc_face.x = x - (mouseAt.x * 2 + camViewport.x - 400);
	mouseLoc_face.y = y - (mouseAt.y * 2 + camViewport.y - 500);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	drawing_back = true;
	drawing_left = true;
	drawing_right = true;
	drawing_face = true;

	lastmouseLoc_back = mouseLoc_back;
	mouseLoc_back.set(x, y);

	lastmouseLoc_right = mouseLoc_right;
	mouseLoc_right.set(x - (rightHandAt.x * 2 + camViewport.x - 200), y - (rightHandAt.y * 2 + camViewport.y - 300));

	lastmouseLoc_left = mouseLoc_left;
	mouseLoc_left.set(x - (leftHandAt.x * 2 + camViewport.x - 200), y - (leftHandAt.y * 2 + camViewport.y - 300));

	lastmouseLoc_face = mouseLoc_face;
	mouseLoc_face.set(x - (mouseAt.x * 2 + camViewport.x - 400), y - (mouseAt.y * 2 + camViewport.y - 500));
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	drawing_back = false;
	drawing_left = false;
	drawing_right = false;
	drawing_face = false;
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
