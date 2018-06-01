#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include "math.h"
#include "Object.h"
#include "Vect.h"
#include "Color.h"

class Triangle : public Object {
private:
	Vect A, B, C;
	Vect normal;
	double distance;
	Color color;

public:
	Triangle();
	Triangle(Vect, Vect, Vect, Vect, Color);

	Vect getTriangleNormal() {
		return normal;
	}


	
	Vect calcTriangleNormal() { 
		Vect CA(C.getVectX() - A.getVectX(), C.getVectY() -A.getVectY(), C.getVectZ() - A.getVectZ());
		Vect BA(B.getVectX() - A.getVectX(), B.getVectY() - A.getVectY(), B.getVectZ() - A.getVectZ());
		normal = CA.crossProduct(BA).normalize();
		return normal;
	}
	
	double getTriangleDistance() {
		normal = getTriangleNormal();
		distance = normal.dotProduct(A);
		return distance; 
	}

	virtual Color getColor() { return color; }
	
	virtual Vect getNormalAt(Vect point) {
		normal = getTriangleNormal();
		return normal;
	}

	//get distance from ray origin to point of intersection
	virtual double findIntersection(Ray ray) {

		Vect ray_direction = ray.getRayDirection();

		Vect ray_origin = ray.getRayOrigin();

		normal = getTriangleNormal();
		distance = getTriangleDistance();

		double a = ray_direction.dotProduct(normal);

		if (a == 0) {		//checks if parallel
			return -1;
		}
		else {
			double b = normal.dotProduct(ray.getRayOrigin().vectAdd(normal.vectMult(distance).negative()));

			double distance2plane = -1 * b / a;

			double Qx = ray_direction.vectMult(distance2plane).getVectX() + ray_origin.getVectX();
			double Qy = ray_direction.vectMult(distance2plane).getVectY() + ray_origin.getVectY();
			double Qz = ray_direction.vectMult(distance2plane).getVectZ() + ray_origin.getVectZ();

			Vect Q(Qx, Qy, Qz);		//point of intersection

			// [CA x QA] * n >= 0
			Vect CA(C.getVectX() - A.getVectX(), C.getVectY() - A.getVectY(), C.getVectZ() - A.getVectZ());
			Vect QA(Q.getVectX() - A.getVectX(), Q.getVectY() - A.getVectY(), Q.getVectZ() - A.getVectZ());
			double test1 = (CA.crossProduct(QA)).dotProduct(normal);

			// [BC x QC] * n >= 0
			Vect BC(B.getVectX() - C.getVectX(), B.getVectY() - C.getVectY(), B.getVectZ() - C.getVectZ());
			Vect QC(Q.getVectX() - C.getVectX(), Q.getVectY() - C.getVectY(), Q.getVectZ() - C.getVectZ());
			double test2 = (BC.crossProduct(QC)).dotProduct(normal);
			
			// [AB x QB] * n >= 0
			Vect AB(A.getVectX() - B.getVectX(), A.getVectY() - B.getVectY(), A.getVectZ() - B.getVectZ());
			Vect QB(Q.getVectX() - B.getVectX(), Q.getVectY() - B.getVectY(), Q.getVectZ() - B.getVectZ());
			double test3 = (AB.crossProduct(QB)).dotProduct(normal);

			//if inside triangle
			if (test1 >= 0 && test2 >= 0 && test3 >= 0) {
				return distance2plane;		//point Q is inside triangle
			}
			else {	//outside triangle
				return -1;				//point Q is not inside triangle
			}
			return -1 * b / a;		//distance from ray origin to intersection point
		}
	}

	void setA(Vect tempA) { A = tempA; }
	void setB(Vect tempB) { B = tempB; }
	void setC(Vect tempC) { C = tempC; }
	void setColor(Color col) { color = col; }
	void setTriangleNormal(Vect n) { normal = n; }


};

Triangle::Triangle() {
	A = Vect(1, 0, 0);
	B = Vect(0, 1, 0);
	C = Vect(0, 0, 1);
	normal = Vect(0, 0, 0);
	color = Color(0.5, 0.5, 0.5, 0.0, 0);
}

Triangle::Triangle(Vect pointA, Vect pointB, Vect pointC, Vect n, Color c) {
	A = pointA;
	B = pointB;
	C = pointC;
	normal = n;

	color = c;
}

#endif
