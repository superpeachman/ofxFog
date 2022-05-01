#include "Fog.h"

Fog::Fog() {
}

void Fog::setup(ofEasyCam* _mainCam, ofFbo* _depthFbo, ofParameter<ofVec3f>* _lightPosition) {
	// initial setting
	ofSetFrameRate(60);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// shader setings
	shaderFog.load("shaders/fog");

	/*--------------------------------------------------------------


	 ofGui


	 --------------------------------------------------------------*/
	float guiWidth = int(ofGetWidth() / 8);
	gui.setDefaultWidth(guiWidth);
	gui.setup();
	gui.setPosition(0, 0);

	gui.add(octave.setup("Noise octave", 5, 1, 10));
	gui.add(offset.setup("Noise offset", 3, 1, 6));
	gui.add(persistence.setup("Noise persistence", 0.4, 0.1, 1.0));
	gui.add(distLength.setup("Apply Alpha Dist", 0.5, 0.1, 0.5));
	gui.add(fogHighlightX.setup("Fog Highlight X", 0.5, 0.0, 1.0));
	gui.add(fogHighlightY.setup("Fog Highlight Y", 0.5, 0.0, 1.0));
	gui.add(fogPosZ.setup("Fog Pos Z", -200.0, -500.0, -10.0));
	gui.add(fogTexS.setup("Fog Tex Speed", 1.0, 0.0, 5.0));
	gui.add(fogScale.setup("Fog Scale", 1024, 256, 2048));
	fogPosZ.addListener(this, &Fog::changeFogSetting);
	fogTexS.addListener(this, &Fog::changeFogSetting2);

	lightPosition = _lightPosition;

	/*--------------------------------------------------------------


	 Main Camera


	 --------------------------------------------------------------*/
	// ----------------------------- debug
	// projection
	//std::cout << "Fov : " << easyCam->getFov() << endl;
	//std::cout << "AspectRatio : " << easyCam->getAspectRatio() << endl;
	//std::cout << "NearClip : " << easyCam->getNearClip() << endl;
	//std::cout << "FarClip : " << easyCam->getFarClip() << endl;
	//// view
	//std::cout << "Position: " << easyCam->getPosition() << endl;
	//std::cout << "LookAt : " << easyCam->getLookAtDir() << endl;
	//std::cout << "Up : " << easyCam->getUpDir() << endl;

	// camera setting
	// I dont know why but if setupPerspective comes first, Depthmap works wrong!!
//    easyCam.setupPerspective();

	easyCam = _mainCam;
	//This only operates with perspective cameras, and will have no effect with cameras in orthographic mode.
	//easyCam->setFov(60);

	/*--------------------------------------------------------------


	 noise buffer


	 --------------------------------------------------------------*/
	noise.allocate(
		pow(2, octave + offset),
		pow(2, octave + offset), 1, GL_RGBA32F, octave, offset, persistence);
	createNoise();

	/*--------------------------------------------------------------


	 Fog


	 --------------------------------------------------------------*/
	offsetPositionX = new float[fogBufferCount];
	offsetPositionZ = new float[fogBufferCount];
	offsetPositionS = new float[fogBufferCount];
	offsetTexCoordS = new float[fogBufferCount];
	offsetTexCoordT = new float[fogBufferCount];

	updateFogSetting();

	std::cout << "Created noiseBufferCount" << endl;

	meshFog.setMode(OF_PRIMITIVE_TRIANGLES);
	meshFog.enableIndices();

	meshFog.addVertex(ofVec3f(-1.0, 1.0, 0.0));
	meshFog.addVertex(ofVec3f(1.0, 1.0, 0.0));
	meshFog.addVertex(ofVec3f(-1.0, -1.0, 0.0));
	meshFog.addVertex(ofVec3f(1.0, -1.0, 0.0));

	meshFog.addTexCoord(ofVec2f(0.0, 0.0));
	meshFog.addTexCoord(ofVec2f(1.0, 0.0));
	meshFog.addTexCoord(ofVec2f(0.0, 1.0));
	meshFog.addTexCoord(ofVec2f(1.0, 1.0));

	meshFog.addColor(ofFloatColor(1.0, 1.0, 1.0, 1.0));
	meshFog.addColor(ofFloatColor(1.0, 1.0, 1.0, 1.0));
	meshFog.addColor(ofFloatColor(1.0, 1.0, 1.0, 1.0));
	meshFog.addColor(ofFloatColor(1.0, 1.0, 1.0, 1.0));

	meshFog.addTriangle(0, 2, 1);
	meshFog.addTriangle(1, 2, 3);

	setNormals(meshFog);

	std::cout << "Created meshFog" << endl;

	/*--------------------------------------------------------------


	 depth


	 --------------------------------------------------------------*/
	depthMap = _depthFbo;
}

//--------------------------------------------------------------
void Fog::update() {
	if (fogChangeFlag) updateFogSetting();
}

//--------------------------------------------------------------
void Fog::updateFogSetting() {
	for (int i = 0; i < fogBufferCount; i++) {
		offsetPositionX[i] = ofRandomf() * 100.0;
		offsetPositionZ[i] = fogPosZ + i * float(abs(fogPosZ) * 2.0) / (fogBufferCount);
		offsetPositionS[i] = (ofRandom(0.3) + fogTexS) * 0.01;
		offsetTexCoordS[i] = offsetTexCoordT[i] = ofRandom(1.0);
	}
	fogChangeFlag = false;
}

//--------------------------------------------------------------
void Fog::fogBegin() {
	shaderFog.begin();

	for (int i = 0; i < fogBufferCount; i++) {
		offsetTexCoordS[i] += offsetPositionS[i];
		offsetTexCoordT[i] += offsetPositionS[i];

		if (offsetTexCoordS[i] > 1.0 || offsetTexCoordT[i] > 1.0) {
			offsetTexCoordS[i] = 0.0;
			offsetTexCoordT[i] = 0.0;
		}

		ofMatrix4x4 modelMatrix;
		modelMatrix.scale(fogScale, fogScale, 1.0);
		modelMatrix.translate(
			offsetPositionX[i],
			0.0,
			offsetPositionZ[i]
		);

		// inv Matrix
		ofMatrix4x4 invMatrix;
		invMatrix = modelMatrix.getInverse();

		// view Matrix
		ofMatrix4x4 viewMatrix;
		viewMatrix = ofGetCurrentViewMatrix();

		// projection Matrix
		ofMatrix4x4 projectionMatrix;
		projectionMatrix = easyCam->getProjectionMatrix();

		// mvp Matrix
		ofMatrix4x4 mvpMatrix;
		mvpMatrix = modelMatrix * viewMatrix * projectionMatrix;

		shaderFog.setUniformMatrix4f("model", modelMatrix);
		shaderFog.setUniformMatrix4f("mvp", mvpMatrix);
		shaderFog.setUniform2f("offset", ofVec2f(offsetTexCoordS[i], offsetTexCoordT[i]));
		shaderFog.setUniform1f("distLength", distLength * 1.0);
		shaderFog.setUniform1f("fogHighlightX", fogHighlightX);
		shaderFog.setUniform1f("fogHighlightY", fogHighlightY);
		shaderFog.setUniformMatrix4f("inv", invMatrix);
		
		shaderFog.setUniform3f("eyeDirection", easyCam->getPosition());
		
		shaderFog.setUniform3f("lightDirection", ofVec3f(
			lightPosition->get().x,
			lightPosition->get().y,
			lightPosition->get().z
		));
		shaderFog.setUniform4f("ambientColor", ofVec4f(0.1, 0.1, 0.1, 1.0));

		shaderFog.setUniformTexture("depthTexture", depthMap->getDepthTexture(), 0);
		shaderFog.setUniformTexture("noiseTexture", noise.fBuffer.getTexture(0), 1);
		meshFog.draw();
	}
}

//--------------------------------------------------------------
void Fog::fogEnd() {
	shaderFog.end();
}

//--------------------------------------------------------------
void Fog::drawGui() {

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
	//ofPushStyle();
	//ofSetColor(255);

	//ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	//float guiWidth = int(ofGetWidth() / 8);
	//ofDrawBitmapStringHighlight("noise", guiWidth + 5, 0);
	//noise.fBuffer.getTexture(0).draw(
	//	guiWidth + 5,
	//	10,
	//	noise.fBuffer.getWidth() * 0.2,
	//	noise.fBuffer.getHeight() * 0.2
	//);

	//ofPopStyle();
}

//--------------------------------------------------------------
float Fog::getNoiseFboWidth() {
	return noise.fBuffer.getWidth();
};

//--------------------------------------------------------------
float Fog::getNoiseFboHeight() {
	return noise.fBuffer.getHeight();
};

//--------------------------------------------------------------
void Fog::drawNoiseFboTex(float _x, float _y, float _w, float _h) {
	noise.fBuffer.getTexture(0).draw(_x, _y, _w, _h);
};

//--------------------------------------------------------------
float Fog::getDepthFboWidth() {
	return depthMap->getWidth();
};

//--------------------------------------------------------------
float Fog::getDepthFboHeight() {
	return depthMap->getHeight();
};

//--------------------------------------------------------------
void Fog::drawDepthFboTex(float _x, float _y, float _w, float _h) {
	depthMap->getDepthTexture().draw(_x, _y, _w, _h);
};

//--------------------------------------------------------------
void Fog::changeFogSetting(float &fogPosZ) {
	fogChangeFlag = true;
}
//--------------------------------------------------------------
void Fog::changeFogSetting2(float &fogTexS) {
	fogChangeFlag = true;
}
//--------------------------------------------------------------
void Fog::createNoise() {
	noise.generate();
}

//--------------------------------------------------------------
void Fog::setNormals(ofMesh &mesh) {

	//The number of the vertices
	int nV = mesh.getNumVertices();

	//The number of the triangles
	int nT = mesh.getNumIndices() / 3;

	vector<ofVec3f> norm(nV);//Array for the normals

	//Scan all the triangles. For each triangle add its
	//normal to norm's vectors of triangle's vertices
	for (int t = 0; t < nT; t++) {

		//Get indices of the triangle t
		int i1 = mesh.getIndex(3 * t);
		int i2 = mesh.getIndex(3 * t + 1);
		int i3 = mesh.getIndex(3 * t + 2);

		//Get vertices of the triangle
		const ofPoint &v1 = mesh.getVertex(i1);
		const ofPoint &v2 = mesh.getVertex(i2);
		const ofPoint &v3 = mesh.getVertex(i3);

		//Compute the triangle's normal
		ofPoint dir = ((v2 - v1).crossed(v3 - v1)).normalized();

		//Accumulate it to norm array for i1, i2, i3
		norm[i1] += dir;
		norm[i2] += dir;
		norm[i3] += dir;
	}

	//Set the normals to mesh
	mesh.clearNormals();

	//Normalize the normal's length
	for (int i = 0; i < nV; i++) {
		mesh.addNormal(norm[i].normalize());
	}
}
