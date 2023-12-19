#include "Poly.h"

void Poly::setX(float x)
{
    this->x = x;
}

void Poly::setY(float y)
{
    this->y = y;
}

void Poly::setZ(float z)
{
    this->z = z;
}

void Poly::setSize(float size)
{
    this->size = size;
}

void Poly::setCube()
{
    relativePoints[0][0] = 0;
    relativePoints[0][1] = 0;
    relativePoints[0][2] = 0;

    relativePoints[1][0] = 1;
    relativePoints[1][1] = 0;
    relativePoints[1][2] = 0;

    relativePoints[2][0] = 0;
    relativePoints[2][1] = 1;
    relativePoints[2][2] = 0;

    relativePoints[3][0] = 0;
    relativePoints[3][1] = 0;
    relativePoints[3][2] = 1;

    relativePoints[4][0] = 1;
    relativePoints[4][1] = 1;
    relativePoints[4][2] = 0;

    relativePoints[5][0] = 1;
    relativePoints[5][1] = 0;
    relativePoints[5][2] = 1;

    relativePoints[6][0] = 0;
    relativePoints[6][1] = 1;
    relativePoints[6][2] = 1;

    relativePoints[7][0] = 1;
    relativePoints[7][1] = 1;
    relativePoints[7][2] = 1;

}

void Poly::setLine(const float(&start)[3], const float(&end)[3])
{
    float slope[3] = { (end[0] - start[0]) / n, (end[1] - start[1]) / n, (end[2] - start[2]) / n };
    for (int i = 0; i < n; i++) {
        relativePoints[i][0] = slope[0] * i + start[0];
        relativePoints[i][1] = slope[1] * i + start[1];
        relativePoints[i][2] = slope[2] * i + start[2];
    }
}

void Poly::setColor(int R, int G, int B)
{
    color[0] = R;
    color[1] = G;
    color[2] = B;
}

float** Poly::getPoints()
{
    for (int i = 0; i < n; i++) {
        points[i][0] = relativePoints[i][0] * size + x;
        points[i][1] = relativePoints[i][1] * size + y;
        points[i][2] = relativePoints[i][2] * size + z;
    }
    return points;
}

int Poly::getN()
{
    return n;
}

int* Poly::getColor()
{
    return color;
}
