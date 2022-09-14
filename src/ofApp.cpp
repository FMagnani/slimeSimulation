#include "ofApp.h"
#include "ofConstants.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);

	compute.setupShaderFromFile(GL_COMPUTE_SHADER,"compute.glsl");
	compute.linkProgram();

	blur.load("shadersGL3/blur.vert", "shadersGL3/blur.frag");

	W = ofGetWidth();
	H = ofGetHeight();
	xMax = W;
	yMax = H;

	cout<<"Window width: "<<W<<"\nWindow height: "<<H<<'\n';

	float pi = 3.14159;

/*
	// Random initialization of pos and vel
	for(int i = 0; i<NPARTICLES; i++){
		posCpu[i].x = ofRandom(0, xMax);
		posCpu[i].y = ofRandom(0, yMax);

		float angle = ofRandom(0, 2*pi);

		velCpu[i].x = cos(angle);
		velCpu[i].y = sin(angle);
	}
*/


	// Circle initialization of pos and vel
	float deltaTheta = 2*pi/NPARTICLES;
	float R = 350;

	for(int i = 0; i<NPARTICLES; i++){

		float rand = ofRandom(0,R);

		posCpu[i].x = W/2 + rand*cos(i*deltaTheta);
		posCpu[i].y = H/2 + rand*sin(i*deltaTheta);

		// Vel directed to the center
		velCpu[i].x = -cos(i*deltaTheta);
		velCpu[i].y = -sin(i*deltaTheta);

		// Vel directed as a vortex
//		float rand2 = ofRandom(0,1);
//		velCpu[i].x = cos(i*deltaTheta) - rand2*sin(i*deltaTheta);
//		velCpu[i].y = sin(i*deltaTheta) + rand2*cos(i*deltaTheta);

	}


	posGpuPing.allocate(NPARTICLES*sizeof(glm::vec2), posCpu, GL_STATIC_DRAW);
	posGpuPong.allocate(NPARTICLES*sizeof(glm::vec2), posCpu, GL_STATIC_DRAW);
	velGpuPing.allocate(NPARTICLES*sizeof(glm::vec2), velCpu, GL_STATIC_DRAW);
	velGpuPong.allocate(NPARTICLES*sizeof(glm::vec2), velCpu, GL_STATIC_DRAW);

	ofBackground(0);

	gui.setup();
	gui.add(fps.set("fps", 60, 0,60));
	gui.add(deltaTime.setup("deltaTime", 1, 0,10));
	gui.add(decayRate.setup("decayRate", 10, 0,50));
	gui.add(turnSpeed.setup("turnSpeed", 1, 0,1));
	gui.add(sensorAngle.setup("sensorAngle", pi/3, 0,pi/2));
	gui.add(sensorDst.setup("sensorDst", 10, 0,150));
	gui.add(sensorSize.setup("sensorSize", 1, 1,2));
	gui.add(start.setup("start", false));

	posGpuPing.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	posGpuPong.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	velGpuPing.bindBase(GL_SHADER_STORAGE_BUFFER, 2);
	velGpuPong.bindBase(GL_SHADER_STORAGE_BUFFER, 3);

	pingPong = 0;

	vbo.setVertexBuffer(posGpuPing, 2, sizeof(glm::vec2));
	vbo.disableColors();

	fbos.allocate(W, H, GL_RGBA32F_ARB);

}

//--------------------------------------------------------------
void ofApp::update(){
	fps = ofGetFrameRate();

	if(start){

		// vel Ping pong
		pingPong = 1-pingPong;
		velGpuPing.bindBase(GL_SHADER_STORAGE_BUFFER, 2 +pingPong);
		velGpuPong.bindBase(GL_SHADER_STORAGE_BUFFER, 3 -pingPong);

		// compute shader - read image & update pos and vel
		fbos.src->getTexture().bindAsImage(4, GL_READ_ONLY);

		compute.begin();

		compute.setUniform1i("W", W);
		compute.setUniform1i("H", H);
		compute.setUniform1i("xMax", xMax);
		compute.setUniform1i("yMax", yMax);
		compute.setUniform1f("deltaTime", deltaTime);
		compute.setUniform1f("decayRate", decayRate);
		compute.setUniform1f("turnSpeed", turnSpeed);
		compute.setUniform1f("sensorAngle", sensorAngle);
		compute.setUniform1f("sensorDst", sensorDst);
		compute.setUniform1i("sensorSize", sensorSize);
		compute.setUniform1f("timeLastFrame", ofGetLastFrameTime());

		compute.dispatchCompute(1024, 1, 1);

		compute.end();

		// pos ping pong
		posGpuPing.copyTo(posGpuPong);

		// fbo 1: draw vertices & apply decay
		fbos.src->begin();
			ofEnableAlphaBlending();

			vbo.draw(GL_POINTS, 0, NPARTICLES);

			ofFill();
			ofSetColor(0,0,0, decayRate);
			ofDrawRectangle(0,0,W,H);
		fbos.src->end();


		// fbo 2: apply blur shader
		fbos.dst->begin();

			blur.begin();
			blur.setUniformTexture("tex0", fbos.src->getTexture(), 4);

			fbos.src->draw(0,0);

			blur.end();

		fbos.dst->end();

		// fbos ping pong
		fbos.swap();
	}

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);

	fbos.src->draw(0,0);
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if (key == 'f'){
		ofToggleFullscreen();
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
