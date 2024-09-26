#include "Constellation.h"

Constellation Constellation::fromStars(vector<StarMesh::StarAddress> stars) {
	set<size_t> unconnectedStars;
	size_t currentStar = ofRandom(stars.size());
	for (size_t i = 0; i < stars.size(); i++) {
		if (i == currentStar) continue;
		unconnectedStars.insert(i);
	}
	vector<tuple<size_t, size_t>> edgeIndices;

	while (unconnectedStars.size() > 0) {
		size_t closestStar;
		float closestDistance = INFINITY;

		for (auto i : unconnectedStars) {
			glm::vec3 posA = stars[i].position;
			glm::vec3 posB = stars[currentStar].position;
			float thisDistance = ofDistSquared(posA.x, posA.y, posA.z, posB.x, posB.y, posB.z);
			if (thisDistance < closestDistance) {
				closestStar = i;
				closestDistance = thisDistance;
			}
		}

		unconnectedStars.erase(closestStar);
		edgeIndices.emplace_back(currentStar, closestStar);
		currentStar = closestStar;
	}

	vector<tuple<glm::vec3, glm::vec3>> edges;
	edges.reserve(edgeIndices.size());
	for (auto i : edgeIndices) {
		edges.emplace_back(stars[std::get<0>(i)].position, stars[std::get<1>(i)].position);
	}

	return Constellation(stars, edges);
}
