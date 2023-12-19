#pragma once

#include <vector>

class 
Poly {
public:
	Poly(int n) {
		x = 0;
		y = 0;
		z = 0;
		size = 1;
		points = new float* [n];
		relativePoints = new float* [n];
		for (int i = 0; i < n; i++) {
			points[i] = new float[3];
			relativePoints[i] = new float[3];
		}
		this->n = n;
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
	}
	void setX(float x);
	void setY(float y);
	void setZ(float z);
	void setSize(float size);
	void setCube();
	void setLine(const float (&start)[3], const float (&end)[3]);
	void setColor(int R, int G, int B);
	float** getPoints();
	int getN();
	int* getColor();
private:
	float x;
	float y;
	float z;
	float size;
	int n;
	float** relativePoints;
	float** points;
	int color[3];
};