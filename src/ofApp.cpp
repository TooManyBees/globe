#include "ofApp.h"
#include "colorIndex.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"

void findStar(char* name, glm::vec3 &dst, glm::vec3 &pos, ofFloatColor &color, nlohmann::json &star) {
	if (dst == glm::vec3(0.0) && star["name"] == name) {
		cout << "Found " << name << "! (" << pos << ") (" << color << ") Hour " << star["hour"] << " " << star["magnitude"] << endl;
		dst = pos;
	}
}

//--------------------------------------------------------------
void ofApp::setup() {
#ifdef FOCUS
	if (oni_manager.setup(640, 480, 30, true)) {
		cout << "Setup device and streams.\n" << endl;
	}
	else {
		cerr << openni::OpenNI::getExtendedError() << endl;
		return std::exit(1);
	}
#endif

	{
		float angle = ofDegToRad(23.4);
		celestialPole = glm::vec3(glm::sin(angle), glm::cos(angle), 0.0);
	}

	starShader.load("stars.vert", "stars.frag");
	starShader.printActiveUniforms();
	starShader.printActiveAttributes();

	ofJson result = ofLoadJson("hyg.json");
	double r = 1000.0;
	glm::vec3 origin(0, 0, 0);
	glm::vec3 whatev;
	glm::vec3 mirror(-1.0, 1.0, 1.0);

	vector<ofJson> stars;
	stars.reserve(result.size());
	for (auto &star : result) {
		stars.push_back(star);
	}
	std::sort(stars.begin(), stars.end(), [](ofJson &const a, ofJson &const b) {
		return a["magnitude"] < b["magnitude"];
	});

	map<string, StarMesh> constellationZones;
	for (auto &const star : stars) {
		glm::vec3 xyz(star["x"], star["z"], star["y"]); // intentionally swap y/z
		glm::vec3 pos;
		bool b = glm::intersectLineSphere(origin, xyz, origin, r, pos, whatev, whatev, whatev);
		if (b) {
			pos *= mirror;
			size_t hour = star["hour"];
			ofFloatColor color = colorIndexToRGB2(star["ci"]);
			float magnitude = star["magnitude"];

			findStar("Betelgeuse", betelgeuse, pos, color, star);
			findStar("Rigel", rigel, pos, color, star);
			findStar("Sirius", sirius, pos, color, star);

			constellationZones[star["con"]].push(pos, magnitude, color);
		}
	}
	for (auto it : constellationZones) {
		//cout << "Constellation " << it.first << " has " << it.second.size() << " stars" << endl;
		star_meshes.push_back(it.second);
	}
	for (auto &mesh : star_meshes) {
		mesh.init(starShader);
	}

	ofSetFrameRate(oni_manager.getFps());
	ofSetVerticalSync(true);
#ifndef FREECAM
	camera.lookAt(glm::vec3(0.0, 0.0, -1.0), celestialPole);
	//camera.lookAt(betelgeuse, celestialPole);
#endif
	//cout << "x: " << camera.getXAxis() << ", y: " << camera.getYAxis() << ", z: " << camera.getZAxis() << endl;
	glEnable(GL_POINT_SMOOTH);
	glCullFace(GL_BACK);

	userFrame.allocate(oni_manager.getWidth(), oni_manager.getHeight(), OF_IMAGE_GRAYSCALE);
	ofEnableAlphaBlending();
	glPointSize(25);
}

//--------------------------------------------------------------
void ofApp::update() {
	reloadShader();
#ifndef FREECAM
	camera.rotateDeg(0.002, celestialPole);
#endif
#ifdef FOCUS
	nite::UserMap userMap = oni_manager.getUserMap();
	//oni_manager.getUserFrame(&userFrame);

	if (oni_manager.usersPresent()) {
		for (auto &mesh : star_meshes) {
			mesh.updateFocus(camera, userMap);
		}
	}
#endif
    if (recording) {
        numRecordFrames += 1;
        if (numRecordFrames > oni_manager.getFps() * 5) {
            endRecording();
        }
    }
}

void drawStar(glm::vec3 &star, char *name, ofCamera &camera) {
	if (star != glm::vec3(0.0)) {
		glm::vec3 s = camera.worldToScreen(star);
		ofSetColor(20);
		ofDrawCircle(s.x, s.y, 5);
		ofSetColor(255);
		ofDrawBitmapString(name, s.x + 10.0, s.y);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	if (recording) {
		ofBackground(ofColor(0, 4, 15));
	}
	else {
		ofBackgroundGradient(ofColor(0, 0, 0), ofColor(0, 8, 30), OF_GRADIENT_LINEAR);
	}
#ifdef FOCUS
	//userFrame.draw(0, 0, ofGetWidth(), ofGetHeight());
#endif

	//drawStar(betelgeuse, "Betelgeuse", camera);
	//drawStar(rigel, "Rigel", camera);
	//drawStar(sirius, "Sirius", camera);

	
	ofSetColor(120, 120, 180);
	camera.begin();
	for (auto &const constellation : constellations) {
		for (auto &const pair : constellation.edges) {
			ofDrawLine(std::get<0>(pair), std::get<1>(pair));
		}
	}

	ofSetColor(255);
	starShader.begin();
	starShader.setUniform1i("frameNo", ofGetFrameNum());
	for (auto &mesh : star_meshes) {
		mesh.draw();
	}
	starShader.end();
	camera.end();

	if (recording) {
		recordCapture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
		ofPixels* p = new ofPixels(recordCapture.getPixels());
		imgSaver->push(p);
        ofSetColor(255, 0, 0);
        ofDrawCircle(20, 20, 10);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

void ofApp::startRecording() {
	string filename = ofGetTimestampString("%F_%H-%M-%S.gif");
	string path = ofFilePath::join("screenshots", filename);
	cout << "New GifSaver @ " << ofGetWidth() << "x" << ofGetHeight() << endl;
	imgSaver = new GifSaver(ofGetWidth(), ofGetHeight(), path);
	recording = true;
}

void ofApp::endRecording() {
	imgSaver->save();
	delete imgSaver;
	numRecordFrames = 0;
	recording = false;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	switch (key) {
	case ' ':
		shaderDirty = true;
		break;
	case 'c':
		snapshotConstellation();
		break;
	case 'f':
		ofToggleFullscreen();
		break;
	case OF_KEY_RETURN:
		if (recording) endRecording();
		else startRecording();
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

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

void ofApp::reloadShader() {
	if (shaderDirty) {
		ofLogNotice() << "Reloading shader." << endl;
		starShader.load("stars.vert", "stars.frag");
		GLint err = glGetError();
		if (err != GL_NO_ERROR) {
			ofLogNotice() << "Shader failed to compile:" << endl << err << endl;
		}
		shaderDirty = false;
	}
}

vector<StarMesh::StarAddress> getBrightFocusedStars(ofCamera &camera, size_t count, vector<StarMesh> &const star_meshes) {
	vector<StarMesh::StarAddress> brightest;

	for (size_t i = 0; i < star_meshes.size(); i++) {
		auto stars = star_meshes[i].brightestStarsInFocus(camera, ofGetFrameNum(), count);
		for (auto &const star : stars) {
			brightest.push_back(StarMesh::StarAddress{ i, star.starIndex, star.magnitude, star.position });
		}
	}

	std::sort(
		brightest.begin(),
		brightest.end(),
		[](StarMesh::StarAddress &const a, StarMesh::StarAddress &const b) {
			return a.magnitude < b.magnitude;
		}
	);

	brightest.resize(count);
	return brightest;
}

void ofApp::snapshotConstellation() {
	auto brightest = getBrightFocusedStars(camera, 10, star_meshes);
	//cout << "Brightest";
	//for (auto &const star : brightest) {
	//	cout << " " << star.meshIndex << " " << star.starIndex;
	//}
	//cout << endl;
	for (size_t meshIndex = 0; meshIndex < star_meshes.size(); meshIndex++) {
		star_meshes[meshIndex].setConstellation(brightest, meshIndex);
	}

	Constellation c = Constellation::fromStars(brightest);
	constellations.push_back(c);
}
