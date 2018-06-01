#ifndef _COLOR_H
#define _COLOR_H

//#include "math.h"

class Color {
private:
	double red, green, blue, special, special2;

public:
	Color();
	Color(double, double, double, double, double);

	double getColorRed() { return red; }
	double getColorGreen() { return green; }
	double getColorBlue() { return blue; }
	double getColorSpecial() { return special; }
	double getColorSpecial2() { return special2; }

	void setColorRed(double rVal) { red = rVal; }
	void setColorGreen(double gVal) { green = gVal; }
	void setColorBlue(double bVal) { blue = bVal; }
	void setColorSpecial(double sVal) { special = sVal; }
	void setColorSpecial2(double s2Val) { special2 = s2Val; }

	double brightness() {
		return (red + green + blue)/3;
	}

	Color colorScalar(double scalar) {
		return Color(red*scalar, green*scalar, blue*scalar, special, special2);
	}

	Color colorAdd(Color color) {
		return Color(red + color.getColorRed(), green + color.getColorGreen(), blue + color.getColorBlue(), special, special2);
	}

	Color colorMultiply(Color color) {
		return Color(red * color.getColorRed(), green * color.getColorGreen(), blue * color.getColorBlue(), special, special2);
	}

	Color colorAverage(Color color) {
		return Color((red + color.getColorRed()) / 2, (green + color.getColorGreen()) / 2, (blue + color.getColorBlue()) / 2, special, special2);
	}


	//clip colors to range of 0.0 to 1.0
	Color clip() {
		double allLight = red + green + blue;
		double excessLight = allLight - 3;

		if (excessLight > 0) {
			red = red + excessLight*(red / allLight);
			green = green + excessLight*(green / allLight);
			blue = blue + excessLight*(blue / allLight);
		}

		if (red > 1) { red = 1; }
		if (green > 1) { green = 1; }
		if (blue > 1) { blue = 1; }

		if (red < 0) { red = 0; }
		if (green < 0) { green = 0; }
		if (blue < 0) { blue = 0; }

		return Color(red, green, blue, special, special2);
	}

};

Color::Color() {
	red = 0.5;
	green = 0.5;
	blue = 0.5;
}

Color::Color(double r, double g, double b, double s, double s2) {
	red = r;
	green = g;
	blue = b;
	special = s;
	special2 = s2;
}

#endif