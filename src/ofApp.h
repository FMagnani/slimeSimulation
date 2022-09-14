#pragma once

#include "ofMain.h"
#include "ofBufferObject.h"
#include "ofxGui.h"
#define NPARTICLES 102400 // NPARTICLES = 1024*N ---> N must be set as local_size_x in compute.glsl

// struct for easy pingPong of FBOs
struct pingPongBuffer {
public:
	void allocate( int _width, int _height, int _internalformat = GL_RGBA){
		// Allocate
		for(int i = 0; i < 2; i++){
			FBOs[i].allocate(_width,_height, _internalformat );
			FBOs[i].getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
		}

		//Assign
		src = &FBOs[0];
		dst = &FBOs[1];

		// Clean
		clear();
	}

	void swap(){
		std::swap(src,dst);
	}

	void clear(){
		for(int i = 0; i < 2; i++){
			FBOs[i].begin();
			ofClear(0,255);
			FBOs[i].end();
		}
	}

	ofFbo& operator[]( int n ){ return FBOs[n];}
	ofFbo   *src;       // Source       ->  Ping
	ofFbo   *dst;       // Destination  ->  Pong

private:
	ofFbo   FBOs[2];    // Real addresses of ping/pong FBO«s
};


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

		ofShader compute, blur;

		glm::vec2 posCpu[NPARTICLES];
		ofBufferObject posGpuPing, posGpuPong;

		glm::vec2 velCpu[NPARTICLES];
		ofBufferObject velGpuPing, velGpuPong;

		int xMax, yMax;
		int W, H;

		ofVbo vbo;

		pingPongBuffer fbos;

		int pingPong;

		ofxPanel gui;
		ofParameter<float> fps;
		ofxFloatSlider deltaTime, decayRate, turnSpeed, sensorAngle, sensorDst;
		ofxIntSlider sensorSize;
		ofxToggle start;

};
