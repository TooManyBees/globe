#include "StarMesh.h"

void StarMesh::push(glm::vec3 position, float magnitude, ofFloatColor color) {
	positions.push_back(position);
	colors.push_back(color);
	magnitudes.push_back(magnitude);
	lastFocus.push_back(0);
	inConstellation.push_back(0);
}

void StarMesh::init(ofShader &shader) {
	float size = positions.size();

	for (size_t n = 0; n < NUM_PROBES; n++) {
		probes[n] = positions[size / NUM_PROBES * n];
	}

	bufMagnitudes.allocate(magnitudes, GL_STATIC_DRAW);
	bufLastFocus.allocate(lastFocus, GL_DYNAMIC_DRAW);
	bufInConstellation.allocate(inConstellation, GL_DYNAMIC_DRAW);

	ofVbo &starVbo = star.getVbo();

	star.addVertices(positions);
	star.addColors(colors);
	star.setMode(OF_PRIMITIVE_POINTS);

	GLint magnitudeLocation = shader.getAttributeLocation("magnitude");
	lastFocusedLocation = shader.getAttributeLocation("lastFocused");
	inConstellationLocation = shader.getAttributeLocation("inConstellation");

	starVbo.setAttributeBuffer(magnitudeLocation, bufMagnitudes, 1, 0);
	starVbo.setAttributeBuffer(lastFocusedLocation, bufLastFocus, 1, 0);
	starVbo.setAttributeBuffer(inConstellationLocation, bufInConstellation, 1, 0);
}

void updateStarFocus(const ofCamera &camera, const nite::UserMap &userMap, const vector<glm::vec3> &positions, vector<uint32_t> &lastFocus) {
	uint64_t focusTime = ofGetFrameNum();
	float windowWidth = ofGetWidth();
	float windowHeight = ofGetHeight();
	float mapWidth = userMap.getWidth();
	float mapHeight = userMap.getHeight();
	const nite::UserId* userPixels = userMap.getPixels();

	for (size_t i = 0; i < positions.size(); i++) {
		glm::vec3 screenCoordinates = camera.worldToScreen(positions[i]);
		int x = screenCoordinates.x / windowWidth * mapWidth;
		if (x < 0 || x >= mapWidth) continue;
		int y = screenCoordinates.y / windowHeight * mapHeight;
		if (y < 0 || y >= mapHeight) continue;
		if (x >= 0 && y >= 0 && userPixels[y * userMap.getWidth() + x] > 0) {
			lastFocus[i] = (uint32_t)focusTime;
		}
	}
}

void StarMesh::updateFocus(ofCamera &camera, nite::UserMap &userMap) {
	if (!isInView(camera)) {
		return;
	}
	updateStarFocus(camera, userMap, positions, lastFocus);

	ofVbo &starVbo = star.getVbo();
	starVbo.updateAttributeData(lastFocusedLocation, (float*)lastFocus.data(), (int)positions.size());
	//starVbo.updateAttributeData(starColorLocation, (float*)colors.data(), size);
}

bool StarMesh::isStarInView(ofCamera &camera, glm::vec3 &star) {
	glm::vec3 screenCoordinates = camera.worldToScreen(star);
	return (
		screenCoordinates.x > 0 && screenCoordinates.x < ofGetWidth() &&
		screenCoordinates.y > 0 && screenCoordinates.y < ofGetHeight() &&
		screenCoordinates.z < 1 // weird; find out why
	);
}

bool StarMesh::isInView(ofCamera &camera) {
	for (glm::vec3 &probe : probes) {
		if (isStarInView(camera, probe)) {
			return true;
		}
	}
	return false;
}

void StarMesh::draw() {
	star.draw(OF_MESH_POINTS);
}

vector<StarMesh::StarAddress> StarMesh::brightestStarsInFocus(ofCamera &camera, uint64_t frameNo, size_t count) {
	vector<StarAddress> brightest;
	if (!isInView(camera)) return brightest;
	brightest.reserve(count);

	for (size_t i = 0; i < positions.size() && brightest.size() < count; i++) {
		if (frameNo - lastFocus[i] > 10) {
			continue;
		}
		brightest.push_back(StarAddress{ 0, i, magnitudes[i], positions[i] });
	}

	return brightest;
}

void StarMesh::setConstellation(vector<StarAddress> &stars, size_t thisMesh) {
	for (auto &const address : stars) {
		if (thisMesh == address.meshIndex) {
			inConstellation[address.starIndex] = 1.0;
		}
	}
	ofVbo &starVbo = star.getVbo();
	starVbo.updateAttributeData(inConstellationLocation, (float*)inConstellation.data(), (int)positions.size());
}
