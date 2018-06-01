#ifndef _LSOURCE_H
#define _LSOURCE_H

//#include "Color.h"

class LSource {

public:

	LSource();

	virtual Vect getLightPosition() { return Vect(0.0, 0.0, 0.0); }
	virtual Color getLightColor() { return Color(1, 1, 1, 0, 0); }

};

LSource::LSource() {}

#endif
