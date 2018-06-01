#ifndef _SPHERE_H
#define _SPHERE_H

#include "math.h"
#include "Object.h"
#include "Vect.h"
#include "Color.h"

class Sphere : public Object{
private:
	Vect center;
	double radius;
	Color color;

public:
	Sphere();
	Sphere(Vect, double, Color);

	Vect getSphereCenter() { return center; }
	double getSphereRadius() { return radius; }
	virtual Color getColor() { return color; }

	virtual Vect getNormalAt(Vect point) {
		Vect norm = point.vectAdd(center.negative()).normalize();
		return norm;
	}

	virtual Vect getNormalAt2(Vect point) {							//get normal for backside of sphere
		Vect norm = point.vectAdd(center).normalize();
		return norm;
	}

	virtual double findIntersection(Ray ray) {

		Vect rayOrigin = ray.getRayOrigin();
		double rayOriginX = rayOrigin.getVectX();
		double rayOriginY = rayOrigin.getVectY();
		double rayOriginZ = rayOrigin.getVectZ();

		Vect rayDir = ray.getRayDirection();
		double rayDirX = rayDir.getVectX();
		double rayDirY = rayDir.getVectY();
		double rayDirZ = rayDir.getVectZ();


		Vect sphereCenter = center;
		double sphereCenterX = sphereCenter.getVectX();
		double sphereCenterY = sphereCenter.getVectY();
		double sphereCenterZ = sphereCenter.getVectZ();

//		double a = 1.0;
		double b = (2 * (rayOriginX - sphereCenterX)*rayDirX) + (2 * (rayOriginY - sphereCenterY)*rayDirY) + (2 * (rayOriginZ - sphereCenterZ)*rayDirZ);
		double c = pow(rayOriginX - sphereCenterX, 2) + pow(rayOriginY - sphereCenterY, 2) + pow(rayOriginZ - sphereCenterZ, 2) - (radius*radius);
		
		double discriminant = b * b - 4 * c;

		//check if ray intersects sphere
		if (discriminant > 0) {

			double root_1 = ((-1 * b - sqrt(discriminant)) / 2) - 0.000001;		//.00001 to counter accuracy errors

			if (root_1 > 0) {
				return root_1;		//if first root is smallest positive
			}
			else {
				double root_2 = ((sqrt(discriminant) - b) / 2) - .000001;		//.000001 to counter accuracy errors
				return root_2;		//second root is smallest positive
			}
		}
		else {
			//ray missed sphere
			return -1;
		}
	} 

	virtual double findIntersection2(Ray ray) {

		Vect rayOrigin = ray.getRayOrigin();
		double rayOriginX = rayOrigin.getVectX();
		double rayOriginY = rayOrigin.getVectY();
		double rayOriginZ = rayOrigin.getVectZ();

		Vect rayDir = ray.getRayDirection();
		double rayDirX = rayDir.getVectX();
		double rayDirY = rayDir.getVectY();
		double rayDirZ = rayDir.getVectZ();


		Vect sphereCenter = center;
		double sphereCenterX = sphereCenter.getVectX();
		double sphereCenterY = sphereCenter.getVectY();
		double sphereCenterZ = sphereCenter.getVectZ();

		double a = 1.0;
		double b = (2 * (rayOriginX - sphereCenterX)*rayDirX) + (2 * (rayOriginY - sphereCenterY)*rayDirY) + (2 * (rayOriginZ - sphereCenterZ)*rayDirZ);
		double c = pow(rayOriginX - sphereCenterX, 2) + pow(rayOriginY - sphereCenterY, 2) + pow(rayOriginZ - sphereCenterZ, 2) - (radius*radius);

		double discriminant = b * b - 4 * c;

		//check if ray intersects sphere
		if (discriminant > 0) {

			double root_1 = ((-1 * b - sqrt(discriminant)) / 2) - 0.000001;		//.00001 to counter accuracy errors

	//		if (root_1 > 0) {
	//			return root_1;		//if first root is smallest positive
	//		}
	//		else {
					double root_2 = ((sqrt(discriminant) - b) / 2) - .000001;		//.000001 to counter accuracy errors
					return root_2;		//second root is smallest positive
	//		}
		}
		else {
			//array missed sphere
			return -1;
		}
	}

};

Sphere::Sphere() {
	center = Vect(0.0, 0.0, 0.0);
	radius = 1.0;
	color = Color(0.5, 0.5, 0.5, 0.0, 0.0);
}

Sphere::Sphere(Vect cent, double rad, Color c) {
	center = cent;
	radius = rad;
	color = c;
}

#endif
