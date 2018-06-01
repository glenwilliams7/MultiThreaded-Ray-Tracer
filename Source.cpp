// Based off Caleb Piercy's raytracer tutorial
// modified by Glen Williams to use up to 12 cpu cores to render.

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>

#include <vector>
#include <cmath>

#include <ctime>

#include "Vect.h"
#include "Ray.h"
#include "Camera.h"
#include "Color.h"
#include "LSource.h"
#include "Light.h"
#include "Sphere.h"
#include "Object.h"
#include "Plane.h"
#include "Triangle.h"

using namespace std;

//stores pixel data
struct RGBType {
	double r, g, b;
};

// Global variables
mutex textSync;				//keeps text uncluttered

int thisone;				// placeholder for pixels
int width = 800;			//	scene width
int height = 600;			//	scene height
int dpi = 72;				//bitmap save parameter
int n = width*height;		// # of pixels
const int aadepth = 4;		//anti aliasing level, 1 = none

double aspectRatio = (double)width / (double)height;
double accuracy = 0.000001;				//accuracy of positions

RGBType *pixels = new RGBType[n];		//holds each individual pixel

//multithreading stuff
int slice = width/4;
RGBType *pixels0 = new RGBType[(n)];		//holds each individual pixel for thread 0
RGBType *pixels1 = new RGBType[(n)];		//holds each individual pixel for thread 1
RGBType *pixels2 = new RGBType[(n)];		//holds each individual pixel...
RGBType *pixels3 = new RGBType[(n)];		//holds each individual pixel...
RGBType *pixels4 = new RGBType[(n)];		//holds each individual pixel...
RGBType *pixels5 = new RGBType[(n)];		//holds each individual pixel...
RGBType *pixels6 = new RGBType[(n)];		//holds each individual pixel...
RGBType *pixels7 = new RGBType[(n)];		//holds each individual pixel for thread 7

RGBType *pixels8 = new RGBType[(n)];		//holds each individual pixel
RGBType *pixels9 = new RGBType[(n)];		//holds each individual pixel
RGBType *pixels10 = new RGBType[(n)];		//holds each individual pixel...
RGBType *pixels11 = new RGBType[(n)];		//holds each individual pixel...

//end multithreading stuff

//some static vectors
Vect X(1, 0, 0);
Vect Y(0, 1, 0);
Vect Z(0, 0, 1);

//camera stuff
Vect camPos(-2.1, 1.25, -2.5);		//assigned values		z=-2.5
Vect look_at(-0.9, 0.45, 0);			//assigned values

// colors
Color white_light(0.6, 0.6, 0.6, 0.0, 0.0);		//scene_light
Color white_light2(0.4, 0.4, 0.4, 0.0, 0.0);	//scene_light2
Color pretty_green(0.5, 1.0, 0.5, 0.8, 0);		//special is shinyness/reflectivity
Color pretty_maroon(0.7, 0.2, 0.25, 0.5, 0);
Color silver(0.5, 0.5, 0.5, 0.99, 0);
Color black(0.0, 0.0, 0.0, 0.0, 0);
Color maroon(0.5, 0.2, 0.25, 0.0, 0);
Color tile_floor(0.8, 0.8, 0.8, 0.3, 2);
Color orange(0.94, 0.75, 0.31, 0.1, 3);
Color Sun(0.94, 0.75, 0.31, .2, 1);
Color mirrorFinish(0.0, 0.0, 0.0, 0.9, 1);
//end colors

//lights
vector<LSource*> light_sources;		//holds all light sources
double ambientLight = 0.2;			//ambient light
Vect lightPos(-16, 16, -1);
Light scene_light(lightPos, white_light);
Vect lightPos2(16, 16, -1);
Light scene_light2(lightPos2, white_light2);
//end lights

//assigned colors
Color ground(0.6, 0.6, 0.6, 0.5, 2);		//checkered floor
Color checkerMt1(0.7, 0, 0, 0.5, 3);		//checkered sphere
Color bricks(0, 0, 0, 0.5, 4);				//brick pattern for teapot
Color refractive(0.0, 0, 0.3, 0.1, 5);			//glass ball

//3 required objects
Plane scene_plane(Vect(0, 1, 0), 0, tile_floor);					//up direction, distance from origin, color
Sphere refractedSphere(Vect(-1.5, .5, 1.2), .5, refractive);		//center, radius, color
Sphere checkeredSphere(Vect(1.5, .5, 1.2), .5, checkerMt1);		//center, radius, color

//grid markers, not active in final version
Vect trueOrigin(0, 0, 0);
Sphere scene_sphere0(trueOrigin, .5, pretty_maroon);		//center, radius, color
Vect origin(-1, 0, 0);
Sphere scene_sphere(origin, .5, pretty_green);		//center, radius, color
Vect origin2(1, 0, 0);
Sphere scene_sphere2(origin2, .5, pretty_green);		//center, radius, color

Vect origin3(-1.5, .5, 3.5);
Sphere scene_sphere3(origin3, .5, orange);		//center, radius, color
//end grid markers

Triangle teapot[896];				//holds teapot triangles

vector<Object*> scene_objects;		//holds all objects in scene
//end objects

Vect diff_btw(camPos.getVectX() - look_at.getVectX(), camPos.getVectY() - look_at.getVectY(), camPos.getVectZ() - look_at.getVectZ());
Vect camDir = diff_btw.negative().normalize();
Vect camRight = Y.crossProduct(camDir).normalize();
Vect camDown = camRight.crossProduct(camDir);
Camera scene_cam(camPos, camDir, camRight, camDown);

// end global variables

//save render to bitmap
void savebmp(const char *filename, int w, int h, int dpi, RGBType *data) {

	FILE *f;
	int k = w*h;
	int s = 4 * k;
	int filesize = 54 + s;

	double factor = 39.375;
	int m = static_cast<int>(factor);

	int ppm = dpi*m;
	unsigned char bmpfileheader[14] = { 'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0 };
	unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};

	bmpfileheader[2] = (unsigned char)(filesize);
	bmpfileheader[3] = (unsigned char)(filesize >> 8);
	bmpfileheader[4] = (unsigned char)(filesize >> 16);
	bmpfileheader[5] = (unsigned char)(filesize >> 24);

	bmpinfoheader[4] = (unsigned char)(w);
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);

	bmpinfoheader[8] = (unsigned char)(h);
	bmpinfoheader[9] = (unsigned char)(h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	bmpinfoheader[21] = (unsigned char)(s);
	bmpinfoheader[22] = (unsigned char)(s >> 8);
	bmpinfoheader[23] = (unsigned char)(s >> 16);
	bmpinfoheader[24] = (unsigned char)(s >> 24);

	bmpinfoheader[25] = (unsigned char)(ppm);
	bmpinfoheader[26] = (unsigned char)(ppm >> 8);
	bmpinfoheader[27] = (unsigned char)(ppm >> 16);
	bmpinfoheader[28] = (unsigned char)(ppm >> 24);

	bmpinfoheader[29] = (unsigned char)(ppm);
	bmpinfoheader[30] = (unsigned char)(ppm >> 8);
	bmpinfoheader[31] = (unsigned char)(ppm >> 16);
	bmpinfoheader[32] = (unsigned char)(ppm >> 24);



	f = fopen(filename, "wb");

	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);

	for (int i = 0; i < k; i++) {

		RGBType rgb = data[i];

		double red = (data[i].r) * 255;
		double green = (data[i].g) * 255;
		double blue = (data[i].b) * 255;

		unsigned char color[3] = { (int)floor(blue), (int)floor(green), (int)floor(red) };

		fwrite(color, 1, 3, f);
	}

	fclose(f);

}

//find index of first intersection
int winningObjectIndex(vector<double> object_intersections) {
	int index_of_minimum_value;


	if (object_intersections.size() == 0) {		//check if intersection exists!
		return -1;
	}
	else if (object_intersections.size() == 1) {	//only 1 intersection
		if (object_intersections.at(0) > 0) {
			return 0;	//index 0
		}
		else {	//intersection is negative, no hits
			return -1;
		}
	}
	else {		//find first intersection if more than 1 exists

		double max = 0;
		for (int i = 0; i < object_intersections.size(); i++) {		//find intersection closest to camera
			if (max < object_intersections.at(i)) {
				max = object_intersections.at(i);
			}	//end if
		}	//end for

		if (max > 0) {
			for (int index = 0; index < object_intersections.size(); index++) {
				if (object_intersections.at(index) > 0 && object_intersections.at(index) <= max) {
					max = object_intersections.at(index);
					index_of_minimum_value = index;				//index of nearest intersection
				}	//end if
			}	//end for

			return index_of_minimum_value;
		}	//end if max > 0
		else {
			return -1;		//all intersections are negative
		}


	}	//end outermost if/else
	
}		//end winningObjectIndex

// get Refract ray, not used in the end
Vect Refract(Vect intersection_position, Vect intersecting_ray_direction0, vector<Object*> scene_objects, int index_of_winning_object, vector<LSource*>light_sources, double accuracy, double ambientLight) {

	Color winning_object_color = scene_objects.at(index_of_winning_object)->getColor();
	Vect winning_object_normal = scene_objects.at(index_of_winning_object)->getNormalAt(intersection_position);
	Vect intersecting_ray_direction = intersecting_ray_direction0;

	Vect tempPosition = intersection_position;			//sphere face position
	Vect tempNormal = winning_object_normal;			//get sphere face normal
	Vect tempDirection0 = intersecting_ray_direction;	//sphere face ray direction

	double refl = winning_object_color.getColorSpecial();
	double trans = 0.0;

	double refractiveIndex = 1.1;

	double n1 = 0;
	double n2 = 0;
	double nr = 0;

	double cosI = tempDirection0.dotProduct(tempNormal);

	if (cosI > 0.0) {
		tempDirection0 = tempDirection0.negative();
		n1 = refractiveIndex;
		n2 = 1.0;
	}
	else {
		n1 = 1.0;
		n2 = refractiveIndex;
		cosI = -cosI;
	}

	nr = n1 / n2;
	double sinT2 = nr*nr*(1.0 - cosI*cosI);
	double cosT = sqrt(1.0 - sinT2);

	double rn = (n1*cosI - n2*cosT) / (n1*cosI + n2*cosT);
	double rt = (n2*cosI - n1*cosT) / (n2*cosI + n2*cosT);

	rn *= rn;
	rt *= rt;
	refl = (rn + rt)*0.5;
	trans = 1.0 - refl;

	if (nr == 1.0) {
		intersecting_ray_direction = tempDirection0;		//return tempDirection0;
	}

	if (cosT*cosT < 0.0) {		//total internal reflection
		refl = 1.0;
		trans = 0.0;
		Vect rdir = intersecting_ray_direction;

		double rdirTemp = rdir.dotProduct(tempNormal);
		Vect rdirTemp2 = tempNormal.vectMult(rdirTemp * -2);
		Vect dir1 = rdir.vectAdd(rdirTemp2);

		return dir1.normalize();					//return calcReflectingRay(r, intersection, normal);
	}

	Vect dir = tempDirection0.vectMult(nr).vectAdd(tempNormal.vectMult(nr*cosI - cosT));
	return intersecting_ray_direction = dir.normalize();				//	return Ray(intersection + dir*BIAS, dir);
	//tempPosition = tempPosition.vectAdd(dir);
}

// get origin of refract ray, also not used
Vect RefractPos(Vect intersection_position, Vect intersecting_ray_direction0, vector<Object*> scene_objects, int index_of_winning_object, vector<LSource*>light_sources, double accuracy, double ambientLight) {

	Color winning_object_color = scene_objects.at(index_of_winning_object)->getColor();
	Vect winning_object_normal = scene_objects.at(index_of_winning_object)->getNormalAt(intersection_position);
	Vect intersecting_ray_direction = intersecting_ray_direction0;

	Vect tempPosition = intersection_position;			//sphere face position
	Vect tempNormal = winning_object_normal;			//get sphere face normal
	Vect tempDirection0 = intersecting_ray_direction;	//sphere face ray direction

	double refl = winning_object_color.getColorSpecial();
	double trans = 0.0;

	double refractiveIndex = 1.1;

	double n1 = 0;
	double n2 = 0;
	double nr = 0;

	double cosI = tempDirection0.dotProduct(tempNormal);

	if (cosI > 0.0) {
		tempDirection0 = tempDirection0.negative();
		n1 = refractiveIndex;
		n2 = 1.0;
	}
	else {
		n1 = 1.0;
		n2 = refractiveIndex;
		cosI = -cosI;
	}

	nr = n1 / n2;
	double sinT2 = nr*nr*(1.0 - cosI*cosI);
	double cosT = sqrt(1.0 - sinT2);

	double rn = (n1*cosI - n2*cosT) / (n1*cosI + n2*cosT);
	double rt = (n2*cosI - n1*cosT) / (n2*cosI + n2*cosT);

	rn *= rn;
	rt *= rt;
	refl = (rn + rt)*0.5;
	trans = 1.0 - refl;

	if (nr == 1.0) {
	//	return intersecting_ray_direction = tempDirection0;		//return tempDirection0;
		return tempPosition;
	}

	if (cosT*cosT < 0.0) {		//total internal reflection
		refl = 1.0;
		trans = 0.0;

		Vect rdir = intersecting_ray_direction;

		double rdirTemp = rdir.dotProduct(tempNormal);
		Vect rdirTemp2 = tempNormal.vectMult(rdirTemp * -2);
		Vect dir1 = rdir.vectAdd(rdirTemp2);

		//return dir1.normalize();					//return calcReflectingRay(r, intersection, normal);
		//return calcReflectingRay(r, intersection, normal);
		return tempPosition;
	}

	Vect dir = tempDirection0.vectMult(nr).vectAdd(tempNormal.vectMult(nr*cosI - cosT));
	intersecting_ray_direction = dir.normalize();				//	return Ray(intersection + dir*BIAS, dir);
	return tempPosition = tempPosition.vectAdd(dir);
}

Color getColorAt(Vect intersection_position, Vect intersecting_ray_direction0, vector<Object*> scene_objects, int index_of_winning_object, vector<LSource*>light_sources, double accuracy, double ambientLight) {

	Color winning_object_color = scene_objects.at(index_of_winning_object)->getColor();
	Vect winning_object_normal = scene_objects.at(index_of_winning_object)->getNormalAt(intersection_position);
	Vect intersecting_ray_direction = intersecting_ray_direction0;

	//patterns, textures, refractions
	if (winning_object_color.getColorSpecial2() == 2) {		//if object special2 color == 2, bit checkers
		int square = (int)floor(intersection_position.getVectX()) + (int)floor(intersection_position.getVectZ());
		if ((square % 2) == 0) {					//dark tile
			winning_object_color.setColorRed(0.2);
			winning_object_color.setColorGreen(0.2);
			winning_object_color.setColorBlue(0.2);
		}
	} 
	else if(winning_object_color.getColorSpecial2() == 3) {		//if object special2 color == 3, small checkers
		int square = (int)floor(8*intersection_position.getVectX()) + (int)floor(8 * intersection_position.getVectY()) + (int)floor(8*intersection_position.getVectZ());

		if ((square % 2) == 0) {					//dark tile
			winning_object_color.setColorRed(0.3);
			winning_object_color.setColorGreen(0.0);
			winning_object_color.setColorBlue(0.0);
		}
	}
	else if (winning_object_color.getColorSpecial2() == 4) {		//if object special2 color == 4, make it bricked
	
		// apply textures
	}
	else if (winning_object_color.getColorSpecial2() == 5) {		//if object special2 color == 5, make it refract

		//Refract(intersection_position, intersecting_ray_direction0, scene_objects, index_of_winning_object, light_sources, accuracy, ambientLight);
		//RefractPos(intersection_position, intersecting_ray_direction0, scene_objects, index_of_winning_object, light_sources, accuracy, ambientLight);

		//cout << "Running option 5" << endl;
		/*
		double n1 = 1.0;
		double n2 = 1.52;
		double nr = n1 / n2;

		Vect tempPosition = intersection_position;			//sphere face position
		Vect tempNormal = winning_object_normal;			//get sphere face normal
		Vect tempDirection0 = intersecting_ray_direction;	//sphere face ray direction

		double temp1 = (nr*(tempNormal.dotProduct(tempDirection0))); 
		double temp2 = tempNormal.dotProduct(tempDirection0)*tempNormal.dotProduct(tempDirection0);
		double temp3 = temp1 - sqrt(1 - nr*nr*(1 - temp2));
		Vect temp4 = tempNormal.vectMult(temp3);

		Vect temp5 = tempDirection0.vectMult(-nr*nr);
		Vect T1 = temp4.vectAdd(temp5);

		intersecting_ray_direction = T1;

	//	cout << "X = " << T1.getVectX() << endl;
	//	cout << "Y = " << T1.getVectY() << endl;
	//	cout << "Z = " << T1.getVectZ() << endl;

		//done entering sphere
		
		//begin leaving sphere
		double nr2 = n2 / n1;

		double distanceFromFirstPoint2Second = scene_objects.at(index_of_winning_object)->findIntersection2(Ray(tempPosition, T1));	//find the intersections
		Vect tempPosition2 = tempPosition.vectAdd(T1.vectMult(distanceFromFirstPoint2Second));	//sphere backside position

		Vect tempNormal2 = scene_objects.at(index_of_winning_object)->getNormalAt(tempPosition2);			//get sphere backside normal
		Vect tempDirection2 = intersecting_ray_direction;	//sphere backside ray direction

		double temp11 = (nr2*(tempNormal2.dotProduct(tempDirection2)));
		double temp12 = tempNormal2.dotProduct(tempDirection2)*tempNormal.dotProduct(tempDirection2);
		double temp13 = temp11 - sqrt(1 - nr2*nr2*(1 - temp12));
		Vect temp14 = tempNormal2.vectMult(temp13);

		Vect temp15 = tempDirection2.vectMult(-nr2*nr2);
		Vect T11 = temp14.vectAdd(temp15);

		intersecting_ray_direction = T11;

		//proceed to next object behind sphere
		*/
	}

	//end patterns, textures, refractions

	Color final_color = winning_object_color.colorScalar(ambientLight);		//set initial final color for pixel to object under ambient light

	//reflections
	if (winning_object_color.getColorSpecial() > 0 && winning_object_color.getColorSpecial() <= 1) {
		// reflection from objects with specular intensity
		double dot1 = winning_object_normal.dotProduct(intersecting_ray_direction.negative());
		Vect scalar1 = winning_object_normal.vectMult(dot1);
		Vect add1 = scalar1.vectAdd(intersecting_ray_direction);
		Vect scalar2 = add1.vectMult(2);
		Vect add2 = intersecting_ray_direction.negative().vectAdd(scalar2);
		Vect reflection_direction = add2.normalize();

		Ray reflection_ray(intersection_position, reflection_direction);

		//find first intersection
		vector<double> reflection_intersections;

		for (int reflection_index = 0; reflection_index < scene_objects.size(); reflection_index++) {
			reflection_intersections.push_back(scene_objects.at(reflection_index)->findIntersection(reflection_ray));
		}

		int index_of_winning_object_with_reflection = winningObjectIndex(reflection_intersections);

		if (index_of_winning_object_with_reflection != -1) {		//reflection ray hits nothing
			if (reflection_intersections.at(index_of_winning_object_with_reflection) > accuracy ) {

				Vect reflection_intersection_position = intersection_position.vectAdd(reflection_direction.vectMult(reflection_intersections.at(index_of_winning_object_with_reflection)));
				Vect reflection_intersection_ray_direction = reflection_direction;

				Color reflection_intersection_color = getColorAt(reflection_intersection_position, reflection_intersection_ray_direction, scene_objects, index_of_winning_object_with_reflection, light_sources, accuracy, ambientLight);

				final_color = final_color.colorAdd(reflection_intersection_color.colorScalar(winning_object_color.getColorSpecial()));
			}		//end if
		}		//end if
	}		//end if
	//end reflections

	//lights and shadows
	for (int light_index = 0; light_index < light_sources.size(); light_index++) {

		Vect light_direction = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();
		float cosine_angle = winning_object_normal.dotProduct(light_direction);

		if (cosine_angle > 0) {
			//test for shadows
			bool shadowed = false;

			Vect distance_to_light = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();
			float distance_to_light_magnitude = distance_to_light.magnitute();
			
			Ray shadow_ray(intersection_position, light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize());
			vector<double> secondary_intersections;

			for (int object_index = 0; object_index < scene_objects.size() && shadowed == false; object_index++) {
				secondary_intersections.push_back(scene_objects.at(object_index)->findIntersection(shadow_ray));
			}

			//check if pixel is shadowed
			for (int c = 0; c < secondary_intersections.size(); c++) {
				if (secondary_intersections.at(c) > accuracy) {
					if (secondary_intersections.at(c) <= distance_to_light_magnitude) {
						shadowed = true;
					}
					break;
				}
				
			}	//end shadow check

			if (shadowed == false) {
				final_color = final_color.colorAdd(winning_object_color.colorMultiply(light_sources.at(light_index)->getLightColor()).colorScalar(cosine_angle));
			
				//get shinyness
				if (winning_object_color.getColorSpecial() > 0 && winning_object_color.getColorSpecial() <= 1) {
					double dot1 = winning_object_normal.dotProduct(intersecting_ray_direction.negative());
					Vect scalar1 = winning_object_normal.vectMult(dot1);
					Vect add1 = scalar1.vectAdd(intersecting_ray_direction);
					Vect scalar2 = add1.vectMult(2);
					Vect add2 = intersecting_ray_direction.negative().vectAdd(scalar2);
					Vect Reflection_direction = add2.normalize();						//get reflection direction

					double specular = Reflection_direction.dotProduct(light_direction);
					if (specular > 0) {
						specular = pow(specular, 10);
						final_color = final_color.colorAdd(light_sources.at(light_index)->getLightColor().colorScalar(specular*winning_object_color.getColorSpecial()));
					}
				}
			}	//end shadow if
		}	//end if(cosine_angle > 0)
	} // end outermost for
	//end lights and shadows

	return final_color.clip();
}

void renderSlice(int xMin, int xMax, int threadNum);

void meshLoader(string filename);

void main() {



	//add lights to scene
	light_sources.push_back(dynamic_cast<LSource*>(&scene_light));
	light_sources.push_back(dynamic_cast<LSource*>(&scene_light2));
	//end lights

	//add grid markers to scene
	//scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere0));		//grid
	//scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere));		//grid
	//scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere2));		//grid
	//scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere3));

	//add required objects to scene
	scene_objects.push_back(dynamic_cast<Object*>(&scene_plane));
	scene_objects.push_back(dynamic_cast<Object*>(&refractedSphere));
	scene_objects.push_back(dynamic_cast<Object*>(&checkeredSphere));

	//add additional objects to scene
	//Sphere ball(Vect(-1.0, 2.5, -1.0), 1.0, Sun);
	//scene_objects.push_back(dynamic_cast<Object*>(&ball));

	Vect A(-0.5, 1.0, 1.2);
	Vect B(0.5, 1.0, 1.2);
	Vect C(0.5, 1.0, 2.2);
	Vect D(-0.5, 1, 2.2);
	Vect E(0.0, 0.0, 1.7);

	Triangle PyrimidF;
	Triangle PyrimidL;
	Triangle PyrimidR;
	Triangle PyrimidB;
	Triangle PyrimidBottom;
	Triangle PyrimidBottom2;

	PyrimidF.setA(A);
	PyrimidF.setB(B);
	PyrimidF.setC(E);
	Vect pn = PyrimidF.calcTriangleNormal();
	PyrimidF.setTriangleNormal(pn);
	PyrimidF.setColor(orange);

	PyrimidL.setA(A);
	PyrimidL.setB(D);
	PyrimidL.setC(E);
	pn = PyrimidL.calcTriangleNormal();
	PyrimidL.setTriangleNormal(pn);
	PyrimidL.setColor(orange);

	PyrimidR.setA(B);
	PyrimidR.setB(C);
	PyrimidR.setC(E);
	pn = PyrimidR.calcTriangleNormal();
	PyrimidR.setTriangleNormal(pn);
	PyrimidR.setColor(orange);

	PyrimidB.setA(D);
	PyrimidB.setB(C);
	PyrimidB.setC(E);
	pn = PyrimidB.calcTriangleNormal();
	PyrimidB.setTriangleNormal(pn);
	PyrimidB.setColor(orange);

	PyrimidBottom.setA(A);
	PyrimidBottom.setB(B);
	PyrimidBottom.setC(C);
	pn = PyrimidBottom.calcTriangleNormal();
	PyrimidBottom.setTriangleNormal(pn);
	PyrimidBottom.setColor(orange);

	PyrimidBottom2.setA(A);
	PyrimidBottom2.setB(C);
	PyrimidBottom2.setC(D);
	pn = PyrimidBottom2.calcTriangleNormal();
	PyrimidBottom2.setTriangleNormal(pn);
	PyrimidBottom2.setColor(orange);

	//scene_objects.push_back(dynamic_cast<Object*>(&PyrimidF));
	//scene_objects.push_back(dynamic_cast<Object*>(&PyrimidL));
	//scene_objects.push_back(dynamic_cast<Object*>(&PyrimidR));
	//scene_objects.push_back(dynamic_cast<Object*>(&PyrimidB));
	//scene_objects.push_back(dynamic_cast<Object*>(&PyrimidBottom));
//	scene_objects.push_back(dynamic_cast<Object*>(&PyrimidBottom2));

//	Plane mirror1(Vect(0, 0, -1.0), -3, mirrorFinish);					//up direction, distance from origin, color
//	scene_objects.push_back(dynamic_cast<Object*>(&mirror1));

	//meshLoader("tris1.txt");											//generates mesh from file
	//end add objects

	bool flag = false;
	int threadCount = 1;
	while (flag == false) {
		cout << "How many CPU threads to render with (type 1, 2, 4, 6, 8, or 12):\n";
		cin >> threadCount;

		if (threadCount == 1 || threadCount == 2 || threadCount == 4 || threadCount == 6 || threadCount == 8 || threadCount == 12) {
			flag = true;
		}
		else {
			cout << "Invalid entry, try again. \n\n";
		}
	}
	slice = width/(double)threadCount;

	cout << "\nMultithreading is wonderful. Set to use " << threadCount << " CPU threads.\n\n";

	cout << "Rendering...\n\n";
	clock_t t1, t2;		//timers
	t1 = clock();		//start time

	//multithreading FTW!

	if (threadCount == 1) {
		std::thread thread0(renderSlice, 0, width, 0);

		thread0.join();

		cout << "Combining slices...";

		//combine all render slices
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {

				thisone = y*width + x;				//individual pixel

				pixels[thisone].r = pixels0[thisone].r;
				pixels[thisone].g = pixels0[thisone].g;
				pixels[thisone].b = pixels0[thisone].b;
			}
		}
	}
	else if (threadCount == 2) {
		std::thread thread0(renderSlice, 0, slice, 0);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread1(renderSlice, slice, width, 1);

		thread0.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread1.join();

		cout << "Combining slices...";

		//combine all render slices
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {

				thisone = y*width + x;				//individual pixel

				if (x < slice) {
					pixels[thisone].r = pixels0[thisone].r;
					pixels[thisone].g = pixels0[thisone].g;
					pixels[thisone].b = pixels0[thisone].b;
				}
				else {
					pixels[thisone].r = pixels1[thisone].r;
					pixels[thisone].g = pixels1[thisone].g;
					pixels[thisone].b = pixels1[thisone].b;
				}
			}
		}
	}
	else if (threadCount == 4) {
		std::thread thread0(renderSlice, 0, slice, 0);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread1(renderSlice, slice, 2 * slice, 1);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread2(renderSlice, 2 * slice, 3 * slice, 2);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread3(renderSlice, 3 * slice, width, 3);

		thread0.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread1.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread2.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread3.join();

		cout << "Combining slices...";

		//combine all render slices
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {

				thisone = y*width + x;				//individual pixel

				if (x < slice) {
					pixels[thisone].r = pixels0[thisone].r;
					pixels[thisone].g = pixels0[thisone].g;
					pixels[thisone].b = pixels0[thisone].b;
				}
				else if (x >= slice && x < 2 * slice) {
					pixels[thisone].r = pixels1[thisone].r;
					pixels[thisone].g = pixels1[thisone].g;
					pixels[thisone].b = pixels1[thisone].b;
				}
				else if (x >= 2 * slice && x < 3 * slice) {
					pixels[thisone].r = pixels2[thisone].r;
					pixels[thisone].g = pixels2[thisone].g;
					pixels[thisone].b = pixels2[thisone].b;
				}
				else {
					pixels[thisone].r = pixels3[thisone].r;
					pixels[thisone].g = pixels3[thisone].g;
					pixels[thisone].b = pixels3[thisone].b;
				}
			}
		}
	}
	else if (threadCount == 6) {
		std::thread thread0(renderSlice, 0, slice, 0);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread1(renderSlice, slice, 2 * slice, 1);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread2(renderSlice, 2 * slice, 3 * slice, 2);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread3(renderSlice, 3 * slice, 4 * slice, 3);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread4(renderSlice, 4 * slice, 5 * slice, 4);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread5(renderSlice, 5 * slice, width, 5);

		//cout << endl << endl;

		thread0.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread1.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread2.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread3.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread4.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread5.join();

		cout << "Combining slices...";

		//combine all render slices
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {

				thisone = y*width + x;				//individual pixel

				if (x < slice) {
					pixels[thisone].r = pixels0[thisone].r;
					pixels[thisone].g = pixels0[thisone].g;
					pixels[thisone].b = pixels0[thisone].b;
				}
				else if (x >= slice && x < 2 * slice) {
					pixels[thisone].r = pixels1[thisone].r;
					pixels[thisone].g = pixels1[thisone].g;
					pixels[thisone].b = pixels1[thisone].b;
				}
				else if (x >= 2 * slice && x < 3 * slice) {
					pixels[thisone].r = pixels2[thisone].r;
					pixels[thisone].g = pixels2[thisone].g;
					pixels[thisone].b = pixels2[thisone].b;
				}
				else if (x >= 3 * slice && x < 4 * slice) {
					pixels[thisone].r = pixels3[thisone].r;
					pixels[thisone].g = pixels3[thisone].g;
					pixels[thisone].b = pixels3[thisone].b;
				}
				else if (x >= 4 * slice && x < 5 * slice) {
					pixels[thisone].r = pixels4[thisone].r;
					pixels[thisone].g = pixels4[thisone].g;
					pixels[thisone].b = pixels4[thisone].b;
				}
				else {
					pixels[thisone].r = pixels5[thisone].r;
					pixels[thisone].g = pixels5[thisone].g;
					pixels[thisone].b = pixels5[thisone].b;
				}
			}
		}
	}
	else if (threadCount == 8) {
		std::thread thread0(renderSlice, 0, slice, 0);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread1(renderSlice, slice, 2 * slice, 1);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread2(renderSlice, 2 * slice, 3 * slice, 2);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread3(renderSlice, 3 * slice, 4 * slice, 3);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread4(renderSlice, 4 * slice, 5 * slice, 4);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread5(renderSlice, 5 * slice, 6 * slice, 5);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread6(renderSlice, 6 * slice, 7 * slice, 6);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread7(renderSlice, 7 * slice, width, 7);

		thread0.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread1.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread2.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread3.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread4.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread5.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread6.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread7.join();

		cout << "Combining slices...";

		//combine all render slices
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {

				thisone = y*width + x;				//individual pixel

				if (x < slice) {
					pixels[thisone].r = pixels0[thisone].r;
					pixels[thisone].g = pixels0[thisone].g;
					pixels[thisone].b = pixels0[thisone].b;
				}
				else if (x >= slice && x < 2 * slice) {
					pixels[thisone].r = pixels1[thisone].r;
					pixels[thisone].g = pixels1[thisone].g;
					pixels[thisone].b = pixels1[thisone].b;
				}
				else if (x >= 2 * slice && x < 3 * slice) {
					pixels[thisone].r = pixels2[thisone].r;
					pixels[thisone].g = pixels2[thisone].g;
					pixels[thisone].b = pixels2[thisone].b;
				}
				else if (x >= 3 * slice && x < 4 * slice) {
					pixels[thisone].r = pixels3[thisone].r;
					pixels[thisone].g = pixels3[thisone].g;
					pixels[thisone].b = pixels3[thisone].b;
				}
				else if (x >= 4 * slice && x < 5 * slice) {
					pixels[thisone].r = pixels4[thisone].r;
					pixels[thisone].g = pixels4[thisone].g;
					pixels[thisone].b = pixels4[thisone].b;
				}
				else if (x >= 5 * slice && x < 6 * slice) {
					pixels[thisone].r = pixels5[thisone].r;
					pixels[thisone].g = pixels5[thisone].g;
					pixels[thisone].b = pixels5[thisone].b;
				}
				else if (x >= 6 * slice && x < 7 * slice) {
					pixels[thisone].r = pixels6[thisone].r;
					pixels[thisone].g = pixels6[thisone].g;
					pixels[thisone].b = pixels6[thisone].b;
				}
				else {
					pixels[thisone].r = pixels7[thisone].r;
					pixels[thisone].g = pixels7[thisone].g;
					pixels[thisone].b = pixels7[thisone].b;
				}
			}
		}
	} 
	else if (threadCount == 12) {
		std::thread thread0(renderSlice, 0, slice, 0);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread1(renderSlice, slice, 2 * slice, 1);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread2(renderSlice, 2 * slice, 3 * slice, 2);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread3(renderSlice, 3 * slice, 4 * slice, 3);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread4(renderSlice, 4 * slice, 5 * slice, 4);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread5(renderSlice, 5 * slice, 6 * slice, 5);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread6(renderSlice, 6 * slice, 7 * slice, 6);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread7(renderSlice, 7 * slice, 8 * slice, 7);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread8(renderSlice, 8 * slice, 9 * slice, 8);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread9(renderSlice, 9 * slice, 10 * slice, 9);
		this_thread::sleep_for(chrono::milliseconds(2));
		std::thread thread10(renderSlice, 10 * slice, 11 * slice, 10);
		this_thread::sleep_for(chrono::milliseconds(10));
		std::thread thread11(renderSlice, 11 * slice, width, 11);

		thread0.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread1.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread2.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread3.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread4.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread5.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread6.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread7.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread8.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread9.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread10.join();
		this_thread::sleep_for(chrono::milliseconds(2));
		thread11.join();

		cout << "Combining slices...";

		//combine all render slices
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {

				thisone = y*width + x;				//individual pixel

				if (x < slice) {
					pixels[thisone].r = pixels0[thisone].r;
					pixels[thisone].g = pixels0[thisone].g;
					pixels[thisone].b = pixels0[thisone].b;
				}
				else if (x >= slice && x < 2 * slice) {
					pixels[thisone].r = pixels1[thisone].r;
					pixels[thisone].g = pixels1[thisone].g;
					pixels[thisone].b = pixels1[thisone].b;
				}
				else if (x >= 2 * slice && x < 3 * slice) {
					pixels[thisone].r = pixels2[thisone].r;
					pixels[thisone].g = pixels2[thisone].g;
					pixels[thisone].b = pixels2[thisone].b;
				}
				else if (x >= 3 * slice && x < 4 * slice) {
					pixels[thisone].r = pixels3[thisone].r;
					pixels[thisone].g = pixels3[thisone].g;
					pixels[thisone].b = pixels3[thisone].b;
				}
				else if (x >= 4 * slice && x < 5 * slice) {
					pixels[thisone].r = pixels4[thisone].r;
					pixels[thisone].g = pixels4[thisone].g;
					pixels[thisone].b = pixels4[thisone].b;
				}
				else if (x >= 5 * slice && x < 6 * slice) {
					pixels[thisone].r = pixels5[thisone].r;
					pixels[thisone].g = pixels5[thisone].g;
					pixels[thisone].b = pixels5[thisone].b;
				}
				else if (x >= 6 * slice && x < 7 * slice) {
					pixels[thisone].r = pixels6[thisone].r;
					pixels[thisone].g = pixels6[thisone].g;
					pixels[thisone].b = pixels6[thisone].b;
				}
				else if (x >= 7 * slice && x < 8 * slice) {
					pixels[thisone].r = pixels7[thisone].r;
					pixels[thisone].g = pixels7[thisone].g;
					pixels[thisone].b = pixels7[thisone].b;
				}
				else if (x >= 8 * slice && x < 9 * slice) {
					pixels[thisone].r = pixels8[thisone].r;
					pixels[thisone].g = pixels8[thisone].g;
					pixels[thisone].b = pixels8[thisone].b;
				}
				else if (x >= 9 * slice && x < 10 * slice) {
					pixels[thisone].r = pixels9[thisone].r;
					pixels[thisone].g = pixels9[thisone].g;
					pixels[thisone].b = pixels9[thisone].b;
				}
				else if (x >= 10 * slice && x < 11 * slice) {
					pixels[thisone].r = pixels10[thisone].r;
					pixels[thisone].g = pixels10[thisone].g;
					pixels[thisone].b = pixels10[thisone].b;
				}
				else  {
					pixels[thisone].r = pixels11[thisone].r;
					pixels[thisone].g = pixels11[thisone].g;
					pixels[thisone].b = pixels11[thisone].b;
				}
			}
		}
	}
	else {
		cout << "Thread count messed up, defaulting to 4\n\n";

		std::thread thread0(renderSlice, 0, slice, 0);
		std::thread thread1(renderSlice, slice, 2 * slice, 1);
		std::thread thread2(renderSlice, 2 * slice, 3 * slice, 2);
		std::thread thread3(renderSlice, 3 * slice, width, 3);

		thread0.join();
		thread1.join();
		thread2.join();
		thread3.join();
	}
	//fairwell my lovely threads

	
	cout << " done.\n";

	t2 = clock();
	float diff = ((float)t2 - (float)t1) / 1000;
	cout << "Rendered in " << diff << "s" << endl;					//output total render time

	savebmp("sceneCombined.bmp", width, height, dpi, pixels);		//save finished render
	cout << endl << endl << "Render saved.\n" << endl;

	delete pixels;													//cleanup

	//this_thread::sleep_for(chrono::milliseconds(3000));
	system("pause");
}


void renderSlice(int xMin, int xMax, int threadNum) {
	int thisThread = threadNum;

	textSync.lock();
	cout << "Thread " << thisThread << " has started rendering.\n";
	textSync.unlock();

	RGBType *tpixels = new RGBType[n];		//holds each individual pixel

	int thisone, aa_index;
	double xAmount, yAmount;
	double tempRed, tempGreen, tempBlue;

	int xMinimum = xMin;
	int xMaximum = xMax;

	for (int x = xMinimum; x < xMaximum; x++) {		//traverse screen width
		for (int y = 0; y < height; y++) {			//travers screen height

			thisone = y*width + x;				//individual pixel

			//begin anti-aliasing stuff
			double tempRed[aadepth*aadepth];
			double tempGreen[aadepth*aadepth];
			double tempBlue[aadepth*aadepth];

			for (int aax = 0; aax < aadepth; aax++) {			//AA outer loop
				for (int aay = 0; aay < aadepth; aay++) {		//AA inner loop

					aa_index = aay*aadepth + aax;
					//srand(time(0));

					//create the ray from camera to this pixel
					if (aadepth == 1) {		//no anti-aliasing

						if (width > height) {
							//wide screen
							xAmount = ((x + 0.5) / width)*aspectRatio - (((width - height) / (double)height) / 2);
							yAmount = (((height - y) + 0.5) / height);
						}
						else if (height > width) {
							//tall screen
							xAmount = (x + 0.5) / width;
							yAmount = (((height - y) + 0.5) / height) / aspectRatio - (((height - width) / (double)width) / 2);
						}
						else {
							//square screen
							xAmount = (x + 0.5) / width;
							yAmount = ((height - y) + 0.5) / height;
						}
					}
					else {		//AA used

						if (width > height) {
							//wide screen
							xAmount = ((x + (double)aax / ((double)aadepth - 1)) / width)*aspectRatio - (((width - height) / (double)height) / 2);
							yAmount = (((height - y) + (double)aax / ((double)aadepth - 1)) / height);
						}
						else if (height > width) {
							//tall screen
							xAmount = (x + (double)aax / ((double)aadepth - 1)) / width;
							yAmount = (((height - y) + (double)aax / ((double)aadepth - 1)) / height) / aspectRatio - (((height - width) / (double)width) / 2);
						}
						else {
							//square screen
							xAmount = (x + (double)aax / ((double)aadepth - 1)) / width;
							yAmount = ((height - y) + (double)aax / ((double)aadepth - 1)) / height;
						}
					}	//end outer else

					Vect cam_ray_origin = scene_cam.getCameraPosition();		//generate rays from camera
					Vect cam_ray_direction = camDir.vectAdd(camRight.vectMult(xAmount - .5).vectAdd(camDown.vectMult(yAmount - 0.5))).normalize();

					Ray cam_ray(cam_ray_origin, cam_ray_direction); //ray goes through current x and y to the scene

					vector<double> intersections;		//hold all intersections for single ray

					for (int index = 0; index < scene_objects.size(); index++) {
						intersections.push_back(scene_objects.at(index)->findIntersection(cam_ray));	//find the intersections
					}

					int index_of_winning_object = winningObjectIndex(intersections);	//determines which object is intersected first, if any

					if (index_of_winning_object == -1) {	//set background color
						tempRed[aa_index] = 0.2;
						tempGreen[aa_index] = 0.2;
						tempBlue[aa_index] = 0.2;

					}
					else {									//index corresponds to object in scene
						if (intersections.at(index_of_winning_object) > accuracy) {
							////////////
							Vect intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersections.at(index_of_winning_object)));
							Vect intersecting_ray_direction = cam_ray_direction;

							Color winning_object_color = scene_objects.at(index_of_winning_object)->getColor();		//get color attributes for refraction option
							Vect winning_object_normal = scene_objects.at(index_of_winning_object)->getNormalAt(intersection_position);

							if (winning_object_color.getColorSpecial2() == 5) {		//if object special2 color == 5, make it refract
								/*
								Vect tempPosition = intersection_position;			//sphere face position
								Vect tempNormal = winning_object_normal;			//get sphere face normal
								Vect tempDirection0 = intersecting_ray_direction;	//sphere face ray direction

								double n1 = 1;
								double n2 = 1.52;
								double nr = n1/n2;

								double temp1 = (nr*(tempNormal.dotProduct(tempDirection0)));
								double temp2 = tempNormal.dotProduct(tempDirection0)*tempNormal.dotProduct(tempDirection0);
								double temp3 = temp1 - sqrt(1 - nr*nr*(1 - temp2));
								Vect temp4 = tempNormal.vectMult(temp3);

								Vect temp5 = tempDirection0.vectMult(-nr*nr);
								Vect T1 = temp4.vectAdd(temp5);

								intersecting_ray_direction = T1;

								//done entering sphere

								//begin leaving sphere
								double nr2 = n2 / n1;

								tempPosition = tempPosition.vectAdd(Vect(0.0, 0.0, 0.0001));		//slight Z offset to get off first intersection point

								double distanceFromFirstPoint2Second = scene_objects.at(index_of_winning_object)->findIntersection(Ray(tempPosition, T1));	//find the intersections
								Vect tempPosition2 = tempPosition.vectAdd(T1.vectMult(distanceFromFirstPoint2Second));	//sphere backside position

								Vect tempNormal2 = scene_objects.at(index_of_winning_object)->getNormalAt(tempPosition2).negative();			//get sphere backside normal
								Vect tempDirection2 = intersecting_ray_direction;	//sphere backside ray direction

								double temp11 = (nr2*(tempNormal2.dotProduct(tempDirection2)));
								double temp12 = tempNormal2.dotProduct(tempDirection2)*tempNormal.dotProduct(tempDirection2);
								double temp13 = temp11 - sqrt(1 - nr2*nr2*(1 - temp12));
								Vect temp14 = tempNormal2.vectMult(temp13);

								Vect temp15 = tempDirection2.vectMult(-nr2*nr2);
								Vect T11 = temp14.vectAdd(temp15);


								//proceed to next object behind sphere
								intersections.pop_back();
								index_of_winning_object = winningObjectIndex(intersections);	//determines which object is intersected first, if any
								intersection_position = tempPosition2;
								intersecting_ray_direction = T11;								
								*/
							}

							Color intersection_color = getColorAt(intersection_position, intersecting_ray_direction, scene_objects, index_of_winning_object, light_sources, accuracy, ambientLight);		//object intersected, get the pixel color with getColorAt()

							tempRed[aa_index] = intersection_color.getColorRed();			//store partial pixel
							tempGreen[aa_index] = intersection_color.getColorGreen();		//store partial pixel
							tempBlue[aa_index] = intersection_color.getColorBlue();			//store partial pixel
						}	//end successful intersection with object

					}		//end else index_of_winning_object
				}		//end inner y loop
			}

				//average the pixel color
				double totalRed = 0;
				double totalGreen = 0;
				double totalBlue = 0;

				for (int iRed = 0; iRed < aadepth*aadepth; iRed++) {		//combine reds from multiple subpixels
					totalRed = totalRed + tempRed[iRed];
				}

				for (int iGreen = 0; iGreen < aadepth*aadepth; iGreen++) {	//combine greens from multiple subpixels
					totalGreen = totalGreen + tempGreen[iGreen];
				}

				for (int iBlue = 0; iBlue < aadepth*aadepth; iBlue++) {		//combine blues from multiple subpixels
					totalBlue = totalBlue + tempBlue[iBlue];
				}

				double avgRed = totalRed / (aadepth*aadepth);				//average subpixels
				double avgGreen = totalGreen / (aadepth*aadepth);			//average subpixels
				double avgBlue = totalBlue / (aadepth*aadepth);				//average subpixels
				//done averaging colors per pixel	

				//save final pixel color to the thread assoctiated RGBType struct
				if (thisThread == 0) {
					pixels0[thisone].r = avgRed;
					pixels0[thisone].g = avgGreen;
					pixels0[thisone].b = avgBlue;
				}
				else if (thisThread == 1) {
					pixels1[thisone].r = avgRed;
					pixels1[thisone].g = avgGreen;
					pixels1[thisone].b = avgBlue;
				}
				else if (thisThread == 2) {
					pixels2[thisone].r = avgRed;
					pixels2[thisone].g = avgGreen;
					pixels2[thisone].b = avgBlue;
				}
				else if (thisThread == 3) {
					pixels3[thisone].r = avgRed;
					pixels3[thisone].g = avgGreen;
					pixels3[thisone].b = avgBlue;
				}
				else if (thisThread == 4) {
					pixels4[thisone].r = avgRed;
					pixels4[thisone].g = avgGreen;
					pixels4[thisone].b = avgBlue;
				}
				else if (thisThread == 5) {
					pixels5[thisone].r = avgRed;
					pixels5[thisone].g = avgGreen;
					pixels5[thisone].b = avgBlue;
				}
				else if (thisThread == 6) {
					pixels6[thisone].r = avgRed;
					pixels6[thisone].g = avgGreen;
					pixels6[thisone].b = avgBlue;
				}	else if (thisThread == 7) {
					pixels7[thisone].r = avgRed;
					pixels7[thisone].g = avgGreen;
					pixels7[thisone].b = avgBlue;
				}
				else if (thisThread == 8) {
					pixels8[thisone].r = avgRed;
					pixels8[thisone].g = avgGreen;
					pixels8[thisone].b = avgBlue;
				}
				else if (thisThread == 9) {
					pixels9[thisone].r = avgRed;
					pixels9[thisone].g = avgGreen;
					pixels9[thisone].b = avgBlue;
				}
				else if (thisThread == 10) {
					pixels10[thisone].r = avgRed;
					pixels10[thisone].g = avgGreen;
					pixels10[thisone].b = avgBlue;
				}
				else {
					pixels11[thisone].r = avgRed;
					pixels11[thisone].g = avgGreen;
					pixels11[thisone].b = avgBlue;
				}
			}		//end y loop		
		}//end x loop

	textSync.lock();
	cout << "Thread " << thisThread << " has finished rendering.\n";
	textSync.unlock();
}

void meshLoader(string filename) {

	cout << "Loading model\n";

	FILE * file = fopen("Tris1.txt", "r");

	//add triangles directly to scene
	//Triangle tempTriangle;
	float lineHeader = 0;

	float trash1, trash2, trash3, trash4;

	float Ax = 0.0;
	float Ay = 0.0;
	float Az = 0.0;

	float Axn = 0.0;
	float Ayn = 0.0;
	float Azn = 0.0;
	
	float Bx = 0.0;
	float By = 0.0;
	float Bz = 0.0;

	float Bxn = 0.0;
	float Byn = 0.0;
	float Bzn = 0.0;

	float Cx = 0.0;
	float Cy = 0.0;
	float Cz = 0.0;

	float Cxn = 0.0;
	float Cyn = 0.0;
	float Czn = 0.0;
	
	fscanf(file, "%f", &lineHeader);		// get # of triangles, not actually used

	lineHeader = lineHeader - 1;
	int eof = 3;
	int i = 0;
	while (i < 895) {
		eof = fscanf(file, "%f %f %f", &Ax, &Ay, &Az);				//get vertex A
		eof = fscanf(file, "%f %f %f", &Axn, &Ayn, &Azn);			//get vertex A normal
		eof = eof = fscanf(file, "%f %f %f %f", &trash1, &trash2, &trash3, &trash4);			//get vertex A trash
		//cout << lineHeader << ",   " << Ax << ",   " << Ay << ",   " << Az << ",   " << Axn << ",   " << Ayn << ",   " << Azn << ",   " << trash1 << ",   " << trash2 << ",   " << trash3 << endl;


		eof = fscanf(file, "%f %f %f", &Bx, &By, &Bz);				//get vertex A
		eof = fscanf(file, "%f %f %f", &Bxn, &Byn, &Bzn);			//get vertex A normal
		eof = eof = fscanf(file, "%f %f %f %f", &trash1, &trash2, &trash3, &trash4);			//get vertex B trash
	//	cout << lineHeader << ",   " << Bx << ",   " << By << ",   " << Bz << ",   " << Bxn << ",   " << Byn << ",   " << Bzn << ",   " << trash1 << ",   " << trash2 << ",   " << trash3 << endl;


		eof = fscanf(file, "%f %f %f", &Cx, &Cy, &Cz);				//get vertex C
		eof = fscanf(file, "%f %f %f", &Cxn, &Cyn, &Czn);			//get vertex C normal
		eof = eof = fscanf(file, "%f %f %f %f", &trash1, &trash2, &trash3, &trash4);			//get vertex C trash
	//	cout << lineHeader << ",   " << Cx << ",   " << Cy << ",   " << Cz << ",   " << Cxn << ",   " << Cyn << ",   " << Czn << ",   " << trash1 << ",   " << trash2 << ",   " << trash3 << endl;

		Vect vA(Ax, Ay, Az);
		Vect vB(Bx, By, Bz);
		Vect vC(Cx, Cy, Cz);

		Vect tempNormal((Axn+Bxn+Cxn), (Ayn+Byn+Cyn), (Azn+Bzn+Czn));		//get average of vertex normals
		Vect normal = tempNormal.normalize();

		//cout << "Averaged normal = " << normal.getVectX() << "     " << normal.getVectY() << "    " << normal.getVectZ() << endl;

		teapot[i].setA(vA);										// set vertex A
		teapot[i].setA(vB);										// set vertex B
		teapot[i].setA(vC);
		//Vect normal = teapot[i].calcTriangleNormal();			// calculate normals from cross products, super broken!
		teapot[i].setTriangleNormal(normal);					// set normal based on average of vertex normals, also super broken!
		teapot[i].setColor(Sun);								// set teapot color

	//	scene_objects.push_back(dynamic_cast<Object*>(&teapot[i]));				// add triangles to scene
		i++;
	}

	fclose(file);
	cout << "Data successfully loaded\n";

}