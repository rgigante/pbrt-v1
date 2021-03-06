
/*
    pbrt source code Copyright(c) 1998-2010 Matt Pharr and Greg Humphreys.

    This file is part of pbrt.

    pbrt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.  Note that the text contents of
    the book "Physically Based Rendering" are *not* licensed under the
    GNU GPL.

    pbrt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

// cone.cpp*
#include "shape.h"
// Cone Declarations
class Cone: public Shape {
public:
	// Cone Public Methods
	Cone(const Transform &o2w, bool ro,
	     float height, float rad, float tm );
	BBox ObjectBound() const;
	bool Intersect(const Ray &ray, float *tHit,
	               DifferentialGeometry *dg) const;
	bool IntersectP(const Ray &ray) const;
	float Area() const;
protected:
	// Cone Data
	float radius, height, phiMax;
};
// Cone Method Definitions
Cone::Cone(const Transform &o2w, bool ro,
           float ht, float rad, float tm )
	: Shape(o2w, ro) {
	radius = rad;
	height = ht;
	phiMax = Radians( Clamp( tm, 0.0f, 360.0f ) );
}
BBox Cone::ObjectBound() const {
	Point p1 = Point( -radius, -radius, 0 );
	Point p2 = Point(  radius,  radius, height );
	return BBox( p1, p2 );
}
bool Cone::Intersect(const Ray &r, float *tHit,
		DifferentialGeometry *dg) const {
	float phi;
	Point phit;
	// Transform _Ray_ to object space
	Ray ray;
	WorldToObject(r, &ray);
	// Compute quadratic cone coefficients
	float k = radius / height;
	k = k*k;
	float A = ray.d.x * ray.d.x + ray.d.y * ray.d.y -
		k * ray.d.z * ray.d.z;
	float B = 2 * (ray.d.x * ray.o.x + ray.d.y * ray.o.y -
		k * ray.d.z * (ray.o.z-height) );
	float C = ray.o.x * ray.o.x + ray.o.y * ray.o.y -
		k * (ray.o.z -height) * (ray.o.z-height);
	// Solve quadratic equation for _t_ values
	float t0, t1;
	if (!Quadratic(A, B, C, &t0, &t1))
		return false;
	// Compute intersection distance along ray
	if (t0 > ray.maxt || t1 < ray.mint)
		return false;
	float thit = t0;
	if (t0 < ray.mint) {
		thit = t1;
		if (thit > ray.maxt) return false;
	}
	// Compute cone inverse mapping
	phit = ray(thit);
	phi = atan2f(phit.y, phit.x);
	if (phi < 0.) phi += 2.f*M_PI;
	// Test cone intersection against clipping parameters
	if (phit.z < 0 || phit.z > height || phi > phiMax) {
		if (thit == t1) return false;
		thit = t1;
		if (t1 > ray.maxt) return false;
		// Compute cone inverse mapping
		phit = ray(thit);
		phi = atan2f(phit.y, phit.x);
		if (phi < 0.) phi += 2.f*M_PI;
		if (phit.z < 0 || phit.z > height || phi > phiMax)
			return false;
	}
	// Find parametric representation of cone hit
	float u = phi / phiMax;
	float v = phit.z / height;
	// Compute cone \dpdu and \dpdv
	Vector dpdu(-phiMax * phit.y, phiMax * phit.x, 0);
	Vector dpdv(-phit.x / (1.f - v),
	            -phit.y / (1.f - v), height);
	// Compute cone \dndu and \dndv
	Vector d2Pduu = -phiMax * phiMax *
	                Vector(phit.x, phit.y, 0.);
	Vector d2Pduv = phiMax / (1.f - v) *
	                Vector(-phit.y, -phit.x, 0.);
	Vector d2Pdvv(0, 0, 0);
	// Compute coefficients for fundamental forms
	float E = Dot(dpdu, dpdu);
	float F = Dot(dpdu, dpdv);
	float G = Dot(dpdv, dpdv);
	Vector N = Normalize(Cross(dpdu, dpdv));
	float e = Dot(N, d2Pduu);
	float f = Dot(N, d2Pduv);
	float g = Dot(N, d2Pdvv);
	// Compute \dndu and \dndv from fundamental form coefficients
	float invEGF2 = 1.f / (E*G - F*F);
	Vector dndu = (f*F - e*G) * invEGF2 * dpdu +
		(e*F - f*E) * invEGF2 * dpdv;
	Vector dndv = (g*F - f*G) * invEGF2 * dpdu +
		(f*F - g*E) * invEGF2 * dpdv;
	// Initialize _DifferentialGeometry_ from parametric information
	*dg = DifferentialGeometry(ObjectToWorld(phit),
	                           ObjectToWorld(dpdu),
							   ObjectToWorld(dpdv),
	                           ObjectToWorld(dndu),
							   ObjectToWorld(dndv),
	                           u, v, this);
	// Update _tHit_ for quadric intersection
	*tHit = thit;
	return true;
}

bool Cone::IntersectP(const Ray &r) const {
	float phi;
	Point phit;
	// Transform _Ray_ to object space
	Ray ray;
	WorldToObject(r, &ray);
	// Compute quadratic cone coefficients
	float k = radius / height;
	k = k*k;
	float A = ray.d.x * ray.d.x + ray.d.y * ray.d.y -
		k * ray.d.z * ray.d.z;
	float B = 2 * (ray.d.x * ray.o.x + ray.d.y * ray.o.y -
		k * ray.d.z * (ray.o.z-height) );
	float C = ray.o.x * ray.o.x + ray.o.y * ray.o.y -
		k * (ray.o.z -height) * (ray.o.z-height);
	// Solve quadratic equation for _t_ values
	float t0, t1;
	if (!Quadratic(A, B, C, &t0, &t1))
		return false;
	// Compute intersection distance along ray
	if (t0 > ray.maxt || t1 < ray.mint)
		return false;
	float thit = t0;
	if (t0 < ray.mint) {
		thit = t1;
		if (thit > ray.maxt) return false;
	}
	// Compute cone inverse mapping
	phit = ray(thit);
	phi = atan2f(phit.y, phit.x);
	if (phi < 0.) phi += 2.f*M_PI;
	// Test cone intersection against clipping parameters
	if (phit.z < 0 || phit.z > height || phi > phiMax) {
		if (thit == t1) return false;
		thit = t1;
		if (t1 > ray.maxt) return false;
		// Compute cone inverse mapping
		phit = ray(thit);
		phi = atan2f(phit.y, phit.x);
		if (phi < 0.) phi += 2.f*M_PI;
		if (phit.z < 0 || phit.z > height || phi > phiMax)
			return false;
	}
	return true;
}
float Cone::Area() const {
	return phiMax*height*height*
		sqrtf((height*height)+
		      (radius*radius))/(2.0f*radius);
}
extern "C" DLLEXPORT Shape *CreateShape(const Transform &o2w,
		bool reverseOrientation, const ParamSet &params) {
	float radius = params.FindOneFloat( "radius", 1 );
	float height = params.FindOneFloat( "height", 1 );
	float phimax = params.FindOneFloat( "phimax", 360 );
	return new Cone(o2w, reverseOrientation, height, radius, phimax);
}
