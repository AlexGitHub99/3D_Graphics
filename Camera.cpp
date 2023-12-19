#include "Camera.h"

void Camera::setX(float x)
{
	this->x = x;
}

void Camera::setY(float y)
{
	this->y = y;
}

void Camera::setZ(float z)
{
	this->z = z;
}

void Camera::addX(float x)
{
	this->x += x;
}

void Camera::addY(float y)
{
	this->y += y;
}

void Camera::addZ(float z)
{
	this->z += z;
}

void Camera::setCoords(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void Camera::setPitch(float pitch)
{
	this->pitch = pitch;
}

void Camera::setYaw(float yaw)
{
	this->yaw = yaw;
}

void Camera::setRoll(float roll)
{
	this->roll = roll;
}

void Camera::addPitch(float pitch)
{
	this->pitch += pitch;
}

void Camera::addYaw(float yaw)
{
	this->yaw += yaw;
}

void Camera::addRoll(float roll)
{
	this->roll += roll;
}

void Camera::setRotation(float pitch, float yaw, float roll)
{
	this->pitch = pitch;
	this->yaw = yaw;
	this->roll = roll;
}

void Camera::setFocal(float f)
{
	this->f = f;
}

float Camera::getX()
{
	return x;
}

float Camera::getY()
{
	return y;
}

float Camera::getZ()
{
	return z;
}

Eigen::Vector3f Camera::getCoords()
{
	return Eigen::Vector3f(x, y, z);
}

float Camera::getYaw()
{
	return yaw;
}

float Camera::getPitch()
{
	return pitch;
}

float Camera::getRoll()
{
	return roll;
}

Eigen::Vector3f Camera::getRotation()
{
	return Eigen::Vector3f(pitch, yaw, roll);
}

float Camera::getFocal()
{
	return f;
}
