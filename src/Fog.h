#pragma once
#include "ofMain.h"
#include "ofxGui.h"
#include "noiseBuffer.h"

class Fog {
public:
	Fog();
	void setup(ofEasyCam* _mainCam, ofFbo* _depthFbo, ofParameter<ofVec3f>* _lightPosition);
	void update();
	void updateFogSetting();
	void drawGui();
	void fogBegin();
	void fogEnd();
	float getNoiseFboWidth();
	float getNoiseFboHeight();
	void drawNoiseFboTex(float _x, float _y, float _w, float _h);
	float getDepthFboWidth();
	float getDepthFboHeight();
	void drawDepthFboTex(float _x, float _y, float _w, float _h);

private:
	void setNormals(ofMesh &mesh);
	void changeFogSetting(float &fogPosZ);
	void changeFogSetting2(float &fogTexS);
	void createNoise();

	ofEasyCam* easyCam;
	/*--------------------------------------------------------------


	 ofGui


	 --------------------------------------------------------------*/
	ofxPanel gui;
	ofxIntSlider octave;
	ofxIntSlider offset;
	ofxFloatSlider persistence;
	ofxFloatSlider distLength;

	ofxFloatSlider fogHighlightX;
	ofxFloatSlider fogHighlightY;
	ofxFloatSlider fogPosZ;
	ofxFloatSlider fogTexS;
	ofxFloatSlider fogScale;

	ofParameter<ofVec3f>* lightPosition;

	/*--------------------------------------------------------------


	 noise buffer


	 --------------------------------------------------------------*/
	noiseBuffer noise;

	/*--------------------------------------------------------------


	 Fog


	 --------------------------------------------------------------*/
	bool fogChangeFlag = false;
	const int fogBufferCount = 14;

	float * offsetPositionX;
	float * offsetPositionZ;
	float * offsetPositionS;
	float * offsetTexCoordS;
	float * offsetTexCoordT;

	/*--------------------------------------------------------------


	 depth


	 --------------------------------------------------------------*/
	ofMesh meshFog;
	ofFbo* depthMap;
	ofShader shaderFog;

};
