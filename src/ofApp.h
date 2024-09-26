#pragma once

#include "ofMain.h"
#include "utils/ofJson.h"
#include "OpenNI.h"
#include "OniManager.h"
#include "StarMesh.h"
#include "GifSaver.h"
#include "Constellation.h"

#define FREECAM

#define FOCUS

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

	private:
		ofImage userFrame;
		glm::vec3 celestialPole;
		glm::vec3 betelgeuse;
		glm::vec3 rigel;
		glm::vec3 sirius;

		vector<StarMesh> star_meshes;
		vector<Constellation> constellations;

#ifdef FREECAM
		ofEasyCam camera;
#else
		ofCamera camera;
#endif
		OniManager oni_manager;

		ofShader starShader;
		bool shaderDirty = false;

		void reloadShader();

		bool recording = false;
		void startRecording();
		void endRecording();
		ofImage recordCapture;
		unsigned int numRecordFrames = 0;
		GifSaver* imgSaver;

		void snapshotConstellation();
};
