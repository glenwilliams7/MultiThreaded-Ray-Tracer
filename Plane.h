#ifndef _PLANE_H
#define _PLANE_H

#include "math.h"
#include "Object.h"
#include "Vect.h"
#include "Color.h"

class Plane : public Object {
private:
	Vect normal;
	double distance;
	Color color;

public:
	Plane();
	Plane(Vect, double, Color);

	Vect getPlaneNormal() { return normal; }
	double getPlaneDistance() { return distance; }
	virtual Color getColor() { return color; }


	virtual Vect getNormalAt(Vect point) {
		return normal;
	}

	//get distance from ray origin to point of intersection
	virtual double findIntersection(Ray ray) {

		Vect ray_direction = ray.getRayDirection();

		double a = ray_direction.dotProduct(normal);

		if (a == 0) {		//checks if parallel
			return -1;
		}
		else {
			double b = normal.dotProduct(ray.getRayOrigin().vectAdd(normal.vectMult(distance).negative()));
			return -1 * b / a;		//distance from ray origin to intersection point
		}
	}

};

Plane::Plane() {
	normal = Vect(1.0, 0.0, 0.0);
	distance = 0.0;
	color = Color(0.5, 0.5, 0.5, 0.0, 0);
}

Plane::Plane(Vect norm, double dis, Color c) {
	normal = norm;
	distance = dis;
	color = c;
}

#endif
