#include <SDL.h>
#include <stdio.h>
#include <vector>
#include <windows.h>
#include <Eigen\Dense>
#include <iostream>
#include <cmath>

#include "Camera.h"
#include "Poly.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

int initSDL(SDL_Window** windowPtr);
Eigen::MatrixXf getTransMatrix(Camera cam);
Eigen::Matrix3f getMovementRotationMatrix(Camera cam);
Eigen::Matrix3f getCameraMatrix(Camera cam);

int main(int argc, char* args[]) 
{

	float f = 20;
	float x = 50;
	float y = 0;
	float z = 50;

	

	

	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;

	if (initSDL(&window) != 0) {
		printf("Failed SDL initialization\n");
		//Destroy window
		SDL_DestroyWindow(window);

		//Quit SDL subsystems
		SDL_Quit();

		return 0;
	}

	//Create renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	//create objects
	Poly cube = Poly(8);
	cube.setCube();
	cube.setSize(20);
	cube.setColor(0, 0, 0);

	//x is blue
	Poly xLine = Poly(100);
	xLine.setLine({0, 0, 0}, {100, 0, 0});
	xLine.setColor(0, 0, 255);

	//y is red
	Poly yLine = Poly(100);
	yLine.setLine({ 0, 0, 0 }, { 0, 100, 0 });
	yLine.setColor(255, 0, 0);

	//z is green
	Poly zLine = Poly(100);
	zLine.setLine({ 0, 0, 0 }, { 0, 0, 100 });
	zLine.setColor(0, 255, 0);

	std::vector<Poly*> polygons;
	//polygons.push_back(&cube);
	polygons.push_back(&xLine);
	polygons.push_back(&yLine);
	polygons.push_back(&zLine);

	//create camera
	Camera cam = Camera();
	cam.setCoords(0, 0, 0);
	cam.setFocal(300);

	//events
	SDL_Event ev;

	SDL_SetRelativeMouseMode(SDL_TRUE);

	while(1) {

		//process events
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_MOUSEMOTION:
				cam.addRoll(-ev.motion.xrel);
				cam.addPitch(ev.motion.yrel);
				if (cam.getPitch() < 0) {
					cam.setPitch(0);
				}
				if (cam.getPitch() > 180) {
					cam.setPitch(180);
				}
			}
		}

		const Uint8* keyStates = SDL_GetKeyboardState(NULL);

		//movement
		float speed = 0.5;
		Eigen::Vector3f movVec(0, 0, 0);
		if (keyStates[SDL_SCANCODE_W] == 1) {
			movVec(2) += 1 * speed;
		}
		if (keyStates[SDL_SCANCODE_S] == 1) {
			movVec(2) -= 1 * speed;
		}
		if (keyStates[SDL_SCANCODE_A] == 1) {
			movVec(0) += 1 * speed;
		}
		if (keyStates[SDL_SCANCODE_D] == 1) {
			movVec(0) -= 1 * speed;
		}
		if (keyStates[SDL_SCANCODE_SPACE] == 1) {
			movVec(1) += 1 * speed;
		}
		if (keyStates[SDL_SCANCODE_LSHIFT] == 1) {
			movVec(1) -= 1 * speed;
		}

		Eigen::Matrix3f R = getMovementRotationMatrix(cam);
		Eigen::Vector3f adjustedMovVec = R * movVec;
		cam.addX(adjustedMovVec(0));
		cam.addY(adjustedMovVec(1));
		cam.addZ(adjustedMovVec(2));

		//set color to white
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		SDL_RenderClear(renderer);
		
		//RENDER
		Eigen::MatrixXf P = getTransMatrix(cam);
		Eigen::Matrix3f K = getCameraMatrix(cam);

		for (int k = 0; k < polygons.size(); k++) {

			//set color
			int *color = polygons[k]->getColor();
			SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);

			float** points = polygons[k]->getPoints();
			for (int j = 0; j < polygons[k]->getN(); j++) {
				Eigen::Vector4f p3D;
				p3D << points[j][0], points[j][1], points[j][2], 1;
				Eigen::Vector3f transP3D = P * p3D;
				if (transP3D(2) < 0) {
					continue;
				}

				Eigen::Vector3f p2D = K * transP3D;
				p2D(0) /= p2D(2);
				p2D(1) /= p2D(2);
				p2D(2) = 1;
				SDL_Rect myRect{ SCREEN_WIDTH - p2D(0), SCREEN_HEIGHT - p2D(1), 4, 4 };
				SDL_RenderFillRect(renderer, &myRect);
			}
		}

		SDL_RenderPresent(renderer);
	}


	//Hack to get window to stay up
	SDL_Event e; bool quit = false; while (quit == false) { while (SDL_PollEvent(&e)) { if (e.type == SDL_QUIT) quit = true; } }





	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}

int initSDL(SDL_Window **windowPtr) {

	SDL_Window* window;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return 1;
		}
	}

	*windowPtr = window;

	return 0;
}

Eigen::MatrixXf getTransMatrix(Camera cam) {

	//pitch is X axis rotation
	//roll is Z axis rotation
	//yaw is Y axis, but is not used for now
	float pitch = -cam.getPitch() / 180 * M_PI;
	float yaw = -cam.getYaw() / 180 * M_PI;
	float roll = -cam.getRoll() / 180 * M_PI;

	Eigen::Matrix3f rPitch;
	Eigen::Matrix3f rRoll;
	rPitch << 1, 0, 0,
		0, cos(pitch), -sin(pitch),
		0, sin(pitch), cos(pitch);
	rRoll << cos(roll), -sin(roll), 0,
		sin(roll), cos(roll), 0,
		0, 0, 1;

	//roll first, then pitch;
	Eigen::Matrix3f R = rPitch * rRoll;

	Eigen::MatrixXf IC(3, 4);
	IC << 1, 0, 0, -cam.getX(),
		0, 1, 0, -cam.getY(),
		0, 0, 1, -cam.getZ();

	Eigen::MatrixXf P(3, 4);
	P = R * IC;

	return P;

}


//differs from rotation matrix used for camera translation
//instead of reversing camera rotation, provides a matrix
//which describes it in forward order
Eigen::Matrix3f getMovementRotationMatrix(Camera cam) {
	//pitch is X axis rotation
	//roll is Z axis rotation
	//yaw is Y axis, but is not used for now
	float pitch = cam.getPitch() / 180 * M_PI;
	float yaw = cam.getYaw() / 180 * M_PI;
	float roll = cam.getRoll() / 180 * M_PI;

	Eigen::Matrix3f rPitch;
	Eigen::Matrix3f rRoll;
	rPitch << 1, 0, 0,
		0, cos(pitch), -sin(pitch),
		0, sin(pitch), cos(pitch);
	rRoll << cos(roll), -sin(roll), 0,
		sin(roll), cos(roll), 0,
		0, 0, 1;

	//pitch first, then roll
	Eigen::Matrix3f R;
	R = rRoll * rPitch;
	return R;
}

Eigen::Matrix3f getCameraMatrix(Camera cam) {
	float px = SCREEN_WIDTH / 2;
	float py = SCREEN_HEIGHT / 2;
	Eigen::Matrix3f K;
	K << cam.getFocal(), 0, px,
		0, cam.getFocal(), py,
		0, 0, 1;

	return K;

}