#pragma once

#include "ofMain.h"
#include "StarMesh.h"

class Constellation
{
public:
	Constellation(vector<StarMesh::StarAddress> s, vector<tuple<glm::vec3, glm::vec3>> e) : stars(s), edges(e) {};

	static Constellation fromStars(vector<StarMesh::StarAddress> stars);

	vector<StarMesh::StarAddress> stars;
	vector<tuple<glm::vec3, glm::vec3>> edges;
};

struct StarCluster {
	vector<StarMesh::StarAddress> stars;
};
