#ifndef _LIGHT_H
#define _LIGHT_H

#include "LSource.h"
#include "Vect.h"
#include "Color.h"

class Light : public LSource {
private:
	Vect position;
	Color color;

public:
	Light();
	Light(Vect, Color);

	virtual Vect getLightPosition() { return position; }
	virtual Color getLightColor() { return color; }
};

Light::Light() {
	position = Vect(0.0, 0.0, 0.0);
	color = Color(1.0, 1.0, 1.0, 0.0, 0.0);
}

Light::Light(Vect p, Color c) {
	position = p;
	color = c;
}

#endif