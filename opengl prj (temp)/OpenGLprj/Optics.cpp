#include "Optics.h"

opt::Ray opt::Ray::findClosestCollision(const opt::PhysMesh& m) {
    auto v = m.getVBOvertices();
    auto in = m.getEBOindices();

    float distance = 100000.f;
    opt::Ray ans;
    for (size_t i = 0; i < m.getVertexCount(); i += 3) {
        auto v0 = (v + *(in + i))->pos, v1 = (v + *(in + i + 1))->pos, v2 = (v + *(in + i + 2))->pos;
        if (intersects(v0, v1, v2)) {
            float newdist = m3d::length(intersectionPos(v0, v1, v2) - start);
            if (distance > newdist) {
                distance = newdist;

                if (outside)
                    ans = refract(v0, v1, v2, 1.f, m.n);
                else
                    ans = refract(v0, v1, v2, m.n, 1.f);
                ans.outside = !outside;
            }
        }
    }
    return ans;
}

bool opt::Ray::intersects(m3d::vec3f v0, m3d::vec3f v1, m3d::vec3f v2)
{
    // compute the plane's normal
    // no need to normalize
    m3d::vec3f N = m3d::cross(v0 - v1, v0 - v2); // N
    float area2 = m3d::length(N);

    // Step 1: finding P

    // check if the ray and plane are parallel.
    float NdotRayDirection = m3d::dot(N, dir);
    if (fabs(NdotRayDirection) < 0.01) // almost 0
        return false; // they are parallel, so they don't intersect! 

    // compute d parameter using equation 2
    float d = m3d::dot(-1.f*N, v0);

    // compute t (equation 3)
    float t = -(m3d::dot(N, start)) / NdotRayDirection;

    // check if the triangle is behind the ray
    if (t < 0) return false; // the triangle is behind

    // compute the intersection point using equation 1
    m3d::vec3f P = start + t * dir;

    // Step 2: inside-outside test
    m3d::vec3f C; // vector perpendicular to triangle's plane

    // edge 0
    m3d::vec3f edge0 = v1 - v0;
    m3d::vec3f vp0 = P - v0;
    C = m3d::cross(edge0, vp0);
    if (m3d::dot(N, C) < 0) return false; // P is on the right side

    // edge 1
    m3d::vec3f edge1 = v2 - v1;
    m3d::vec3f vp1 = P - v1;
    C = m3d::cross(edge1, vp1);
    if (m3d::dot(N, C) < 0)  return false; // P is on the right side

    // edge 2
    m3d::vec3f edge2 = v0 - v2;
    m3d::vec3f vp2 = P - v2;
    C = m3d::cross(edge2, vp2);
    if (m3d::dot(N, C) < 0) return false; // P is on the right side;

    return true; // this ray hits the triangle
}

m3d::vec3f opt::Ray::intersectionPos(m3d::vec3f v0, m3d::vec3f v1, m3d::vec3f v2) {
    // compute the plane's normal
// no need to normalize
    m3d::vec3f N = m3d::cross(v0 - v1, v0 - v2); // N
    float area2 = m3d::length(N);

    // Step 1: finding P

    // check if the ray and plane are parallel.
    float NdotRayDirection = m3d::dot(N, dir);


    // compute d parameter using equation 2
    float d = m3d::dot(-1.f * N, v0);

    // compute t (equation 3)
    float t = -(m3d::dot(N, start)) / NdotRayDirection;

    // compute the intersection point using equation 1
    m3d::vec3f P = start + t * dir;
    return P;
}

opt::Ray opt::Ray::refract(m3d::vec3f v0, m3d::vec3f v1, m3d::vec3f v2, float n1, float n2) {
	m3d::vec3f n = m3d::normalize(m3d::cross(v0 - v1, v0 - v2));
	m3d::vec3f a = n1 / n2 * (dir - m3d::dot(dir, n) * n);

	Ray newray;
	newray.dir = a - n * sqrtf(1.f - m3d::norm(a));
    newray.start = intersectionPos(v0, v1, v2);

	return newray;
}