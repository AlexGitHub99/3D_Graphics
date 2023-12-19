#pragma once

#include <Eigen/Dense>

class Camera
{
public:
	Camera() {
		x = 0;
		y = 0;
		z = 0;
		pitch = 0;
		yaw = 0;
		roll = 0;
		f = 1;
	}
	void setX(float x);
	void setY(float y);
	void setZ(float z);
	void addX(float x);
	void addY(float y);
	void addZ(float z);
	void setCoords(float x, float y, float z);
	void setPitch(float pitch);
	void setYaw(float yaw);
	void setRoll(float roll);
	void addPitch(float pitch);
	void addYaw(float yaw);
	void addRoll(float roll);
	void setRotation(float pitch, float yaw, float roll);
	void setFocal(float f);
	float getX();
	float getY();
	float getZ();
	Eigen::Vector3f getCoords();
	float getYaw();
	float getPitch();
	float getRoll();
	Eigen::Vector3f getRotation();
	float getFocal();

private:
	float x;
	float y;
	float z;
	float pitch;
	float yaw;
	float roll;
	float f;
};