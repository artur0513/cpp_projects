#pragma once
#include "3dMath/3dMath.h"
#include "Mesh.h"

namespace opt {

	class PhysMesh : public OBJ::Mesh {
	public:
		float n = 1.5;
	};

	struct Ray {
		m3d::vec3f start, dir;
		float length;

		bool outside = true;

		Ray findClosestCollision(const PhysMesh& m);

		bool intersects(m3d::vec3f v0, m3d::vec3f v1, m3d::vec3f v2);

		m3d::vec3f intersectionPos(m3d::vec3f v0, m3d::vec3f v1, m3d::vec3f v2);

		Ray refract(m3d::vec3f va, m3d::vec3f vb, m3d::vec3f vc, float n1, float n2);
	};
}