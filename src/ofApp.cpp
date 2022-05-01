#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	/*--------------------------------------------------------------


	 BASE


	 --------------------------------------------------------------*/
	ofSetLogLevel(OF_LOG_VERBOSE);
	//ofSetVerticalSync(true);
	//ofBackground(0, 0, 0);
	//ofBackground(128);
	//ofSetBackgroundAuto(false);
	//ofSetBackgroundAuto(true);
	//ofEnableAlphaBlending();

	ofSetFrameRate(0);
	ofSetBackgroundColor(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	/*--------------------------------------------------------------


	 ofGui


	 --------------------------------------------------------------*/
	float guiWidth = int(ofGetWidth() / 8);
	gui.setDefaultWidth(guiWidth);
	gui.setup();
	gui.setPosition(0, 0);

	gui.add(debugFlag.setup("Debug", false));
	gui.add(rotationFlag.setup("Rotataion", true));
	gui.add(lightPosition.set("Light Pos",
		ofVec3f(-100.0, 50.0, 50.0),
		ofVec3f(-500, -500, -500),
		ofVec3f(500, 500, 500))
	); // this will create a slider group for your vec3 in the gui.

	gui.add(camNear.setup("Camera Near Clip", 100.0, 1.0, 100.0));
	gui.add(camFar.setup("Camera Far Clip", 3000.0, 100.0, 3000.0));
	gui.add(boxScale.setup("Box Scale", 15.0, 1.0, 20.0));
	gui.add(boxAngle.setup("Box Angle", 45.0, 0.0, 360.0));


	// ----------------------------- debug
	// projection
	std::cout << "Fov : " << easyCam.getFov() << endl;
	std::cout << "AspectRatio : " << easyCam.getAspectRatio() << endl;
	std::cout << "NearClip : " << easyCam.getNearClip() << endl;
	std::cout << "FarClip : " << easyCam.getFarClip() << endl;
	// view
	std::cout << "Position: " << easyCam.getPosition() << endl;
	std::cout << "LookAt : " << easyCam.getLookAtDir() << endl;
	std::cout << "Up : " << easyCam.getUpDir() << endl;

	camNear.addListener(this, &ofApp::changeNearClip);
	camFar.addListener(this, &ofApp::changeFarClip);

	// camera setting
	// I dont know why but if setupPerspective comes first, Depthmap works wrong!!
	//    easyCam.setupPerspective();
	easyCam.setNearClip(camNear);
	easyCam.setFarClip(camFar);
	//    easyCam.setForceAspectRatio(1.0);
	//This only operates with perspective cameras, and will have no effect with cameras in orthographic mode.
	easyCam.setFov(60);

	pos = ofVec3f(0.0, 0.0, 0.0);

	/*--------------------------------------------------------------


	 Shader


	 --------------------------------------------------------------*/
	shader.load("shaders/shader");

	/*--------------------------------------------------------------


	 Depth


	 --------------------------------------------------------------*/
	shaderDepth.load("shaders/depth");

	/*--------------------------------------------------------------


	 depth


	 --------------------------------------------------------------*/
	ofFbo::Settings setting;
	depthMapRes = 1024;
	setting.width = depthMapRes;
	setting.height = 1024 * (float(ofGetHeight()) / ofGetWidth());
	setting.textureTarget = GL_TEXTURE_2D;
	setting.internalformat = GL_R8;
	setting.useDepth = true;
	setting.depthStencilAsTexture = true;
	setting.useStencil = true;
	setting.minFilter = GL_LINEAR;
	setting.maxFilter = GL_LINEAR;
	setting.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
	setting.wrapModeVertical = GL_CLAMP_TO_EDGE;
	//    setting.numSamples = 8;
	depthMap.allocate(setting);

	std::cout << "Created depthMap" << endl;

	/*--------------------------------------------------------------


	 Fog


	 --------------------------------------------------------------*/
	fog = new Fog();
	fog->setup(&easyCam, &depthMap, &lightPosition);
}

//--------------------------------------------------------------
void ofApp::update(){
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
	if (rotationFlag) time = ofGetElapsedTimef();

	if (flagCameraClipChanged) {
		// Camera Reset
		easyCam.setNearClip(camNear);
		easyCam.setFarClip(camFar);
		//        easyCam.reset();

		// ----------------------------- debug
		// projection
		std::cout << "------------------" << endl;
		std::cout << "Fov : " << easyCam.getFov() << endl;
		std::cout << "AspectRatio : " << easyCam.getAspectRatio() << endl;
		std::cout << "NearClip : " << easyCam.getNearClip() << endl;
		std::cout << "FarClip : " << easyCam.getFarClip() << endl;
		//    // view
		std::cout << "Position: " << easyCam.getPosition() << endl;
		std::cout << "LookAt : " << easyCam.getLookAtDir() << endl;
		std::cout << "Up : " << easyCam.getUpDir() << endl;

		flagCameraClipChanged = false;
	}

	fog->update();
}

//--------------------------------------------------------------
void ofApp::changeNearClip(float &camNear) {
	flagCameraClipChanged = !flagCameraClipChanged;
}

//--------------------------------------------------------------
void ofApp::changeFarClip(float &camFar) {
	flagCameraClipChanged = !flagCameraClipChanged;
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofEnableDepthTest();
	ofPushStyle();

	/*--------------------------------------------------------------


	 depth starts


	 --------------------------------------------------------------*/
	depthMap.begin();
	//Necessary to clear depth every time
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	easyCam.begin();
	shaderDepth.begin();

	renderScene(0);

	shaderDepth.end();
	easyCam.end();
	depthMap.end();



	easyCam.begin();
	/*--------------------------------------------------------------


	 shader


	 --------------------------------------------------------------*/
	shader.begin();

	renderScene(1);

	shader.end();

	/*--------------------------------------------------------------


	 debug


	--------------------------------------------------------------*/
	//draw axis (x, y, z)
	//ofSetLineWidth(0.1);
	//ofSetColor(255, 0, 0);
	//ofDrawLine(-500, 0, 0, 500, 0, 0);
	//ofSetColor(0, 255, 0);
	//ofDrawLine(0, -400, 0, 0, 400, 0);
	//ofSetColor(0, 0, 255);
	//ofDrawLine(0, 0, -400, 0, 0, 400);

	/*--------------------------------------------------------------


	 fog


	--------------------------------------------------------------*/
	fog->fogBegin();
	fog->fogEnd();

	/* -----------------------------------------------------------*/


	easyCam.end();

	ofPopStyle();
	ofDisableDepthTest();


	if (debugFlag) {
		/*--------------------------------------------------------------


		 gui


		 --------------------------------------------------------------*/
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(255);
		gui.draw();
		ofPopStyle();

		/*--------------------------------------------------------------


		 debug


		 --------------------------------------------------------------*/
		ofPushStyle();
		ofSetColor(255);

		ofEnableBlendMode(OF_BLENDMODE_ALPHA);

		float guiWidth = int(ofGetWidth() / 8);
		ofDrawBitmapStringHighlight("depth", guiWidth + 5, 10);
		depthMap.getDepthTexture().draw(
			guiWidth + 5,
			10,
			depthMap.getWidth()*0.2,
			depthMap.getHeight()*0.2
		);

		fog->drawDepthFboTex(
			guiWidth + 5,
			10 + depthMap.getHeight()*0.2 + 5,
			depthMap.getWidth()*0.2,
			depthMap.getHeight()*0.2
		);

		ofDrawBitmapStringHighlight("noise", guiWidth + 5 + depthMap.getWidth()*0.2 + 5, 10);
		fog->drawNoiseFboTex(
			guiWidth + 5 + depthMap.getWidth()*0.2,
			10,
			fog->getNoiseFboWidth() * 0.5,
			fog->getNoiseFboWidth() * 0.5
		);

		ofPopStyle();

		/*--------------------------------------------------------------


		 fog


		 --------------------------------------------------------------*/
		fog->drawGui();
	}

}

//--------------------------------------------------------------
void ofApp::renderScene(int mode) {

	for (int i = 0; i < MODEL_NUM; i++) {
		// model Matrix
		ofMatrix4x4 modelMatrix;
		if (i == 0) modelMatrix.translate(-100, 0, 0);
		if (i == 1) modelMatrix.translate(0, 0, 0);
		if (i == 2) modelMatrix.translate(100, 0, 0);
		if (i == 3) modelMatrix.translate(0, -100, 0);
		if (i == 4) modelMatrix.translate(0, 100, 0);
		if (i == 5) modelMatrix.translate(0, 0, -100);
		if (i == 6) modelMatrix.translate(0, 0, 100);
		modelMatrix.rotate(time * 10, 1.0, 0.5, 0.0);
		//pos * modelMatrix;
		sphere.setPosition(pos);
		sphere.setRadius(50);

		// view Matrix
		ofMatrix4x4 viewMatrix;
		viewMatrix = ofGetCurrentViewMatrix();

		// projection Matrix
		ofMatrix4x4 projectionMatrix;
		projectionMatrix = easyCam.getProjectionMatrix();

		// mvp Matrix
		ofMatrix4x4 mvpMatrix;
		mvpMatrix = modelMatrix * viewMatrix * projectionMatrix;

		// inv Matrix
		ofMatrix4x4 invMatrix;
		invMatrix = modelMatrix.getInverse();

		if (mode == 0) {
			shaderDepth.setUniformMatrix4f("mvp", mvpMatrix);
		}
		else if (mode == 1) {
			shader.setUniformMatrix4f("model", modelMatrix);
			shader.setUniformMatrix4f("modelView", easyCam.getModelViewMatrix());
			shader.setUniformMatrix4f("mvp", mvpMatrix);
			shader.setUniformMatrix4f("inv", invMatrix);
			shader.setUniform3f("eyeDirection", ofVec3f(easyCam.getPosition()));

			shader.setUniform3f("lightDirection", ofVec3f(
				lightPosition->x,
				lightPosition->y,
				lightPosition->z
			));
			shader.setUniform4f("ambientColor", ofVec4f(0.1, 0.1, 0.1, 1.0));
		}

		vboMesh = sphere.getMesh();
		for (int j = 0; j < vboMesh.getVertices().size(); j++) {
			vboMesh.addColor(ofFloatColor(1.0, 1.0, 1.0, 1.0));
		}
		vboMesh.setMode(OF_PRIMITIVE_TRIANGLES);
		vboMesh.draw();
	}


}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if (key == 'g') {
		debugFlag = !debugFlag;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
