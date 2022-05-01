#pragma once

#define MODEL_NUM 7

#include "ofMain.h"
#include "ofxGui.h"
#include "Fog.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void renderScene(int mode);

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

	private:
		void changeNearClip(float &camNear);
		void changeFarClip(float &camFar);
		bool flagCameraClipChanged = false;

		ofEasyCam easyCam;
		ofShader shader;
		ofVboMesh vboMesh;
		ofSpherePrimitive sphere;
		ofBoxPrimitive box;

		ofVec3f pos;
		float time;

		/*--------------------------------------------------------------


		 ofGui


		 --------------------------------------------------------------*/
		bool isAxisShown = false;
		bool isGuiShown = true;
		bool isFabricGuiShown = false;
		bool isLightGuiShown = true;

		ofxPanel gui;
		ofxToggle debugFlag;
		ofxToggle rotationFlag;
		//ofxVec3Slider lightPosition;
		ofParameter<ofVec3f> lightPosition;
		ofxFloatSlider camNear;
		ofxFloatSlider camFar;
		ofxFloatSlider boxScale;
		ofxFloatSlider boxAngle;

		/*--------------------------------------------------------------


		 Depth


		 --------------------------------------------------------------*/
		ofShader shaderDepth;
		ofFbo depthMap;
		int depthMapRes;

		/*--------------------------------------------------------------


		 Fog


		 --------------------------------------------------------------*/
		Fog* fog;
		ofxIntSlider octave;
		ofxIntSlider offset;
		ofxFloatSlider persistence;
		ofxFloatSlider distLength;
		ofxFloatSlider fogHighlightX;
		ofxFloatSlider fogHighlightY;
		ofxFloatSlider fogPosZ;
		ofxFloatSlider fogTexS;
		ofxFloatSlider fogScale;
};
