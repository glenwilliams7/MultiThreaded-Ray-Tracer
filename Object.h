#ifndef _OBJECT_H
#define _OBJECT_H

#include "Ray.h"
#include "Vect.h"
#include "Color.h"

class Object {
private:


public:
	Object();

	virtual Color getColor() { return Color(0.0, 0.0, 0.0, 0.0, 0); }
	
	virtual Vect getNormalAt(Vect intersection_position) {
		return Vect(0.0,0.0,0.0);
	}
	
	virtual Vect getNormalAt2(Vect intersection_position) {
		return Vect(0.0, 0.0, 0.0);
	}

	virtual double findIntersection(Ray ray) {
		return 0.0;
	}

	virtual double findIntersection2(Ray ray) {
		return 0.0;
	}
};

Object::Object() {
	// empty
}


#endif

