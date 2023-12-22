#include <SDL.h>
#include <stdio.h>
#include <vector>
#include <windows.h>
#include <Eigen\Dense>
#include <iostream>
#include <cmath>
#include <chrono>

#include "Camera.h"
#include "Poly.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

const int TRIANGLE = 0;
const int LINE = 1;
const int CUBE = 2;

int initSDL(SDL_Window** windowPtr);
Eigen::MatrixXf getTransMatrix(Camera cam);
Eigen::Matrix3f getMovementRotationMatrix(Camera cam);
Eigen::Matrix3f getCameraMatrix(Camera cam);
Eigen::Vector3f findIntersection(Eigen::Vector4f plane, Eigen::Vector3f line);
Eigen::Vector4f getPlane(Eigen::Vector3f p1, Eigen::Vector3f p2, Eigen::Vector3f p3);
int** getLinePoints(int p1Raw[2], int p2Raw[2], int& size);
int cropLine(int p1[2], int p2[2]);
bool isInScreen(int p[2]);
int* findLineIntersection(int p1[2], int p2[2], int p3[2], int p4[2]);

int main(int argc, char* args[]) 
{

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
	Poly xLine = Poly(2);
	xLine.setLine({0, 0, 0}, {100, 0, 0});
	xLine.setColor(0, 0, 255);

	//y is red
	Poly yLine = Poly(2);
	yLine.setLine({ 0, 0, 0 }, { 0, 100, 0 });
	yLine.setColor(255, 0, 0);

	//z is green
	Poly zLine = Poly(2);
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

	//stores depth and polygon for every pixel
	float ***pixels = new float**[SCREEN_WIDTH];
	for (int i = 0; i < SCREEN_WIDTH; i++) {
		pixels[i] = new float* [SCREEN_HEIGHT];
		for (int j = 0; j < SCREEN_HEIGHT; j++) {
			pixels[i][j] = new float[2];
			pixels[i][j][0] = -1;
			pixels[i][j][1] = -1;
		}
	}

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

		for (int i = 0; i < SCREEN_WIDTH; i++) {
			for (int j = 0; j < SCREEN_HEIGHT; j++) {
				pixels[i][j][0] = -1;
				pixels[i][j][1] = -1;
			}
		}

		Eigen::MatrixXf P = getTransMatrix(cam);
		Eigen::Matrix3f K = getCameraMatrix(cam);

		for (int k = 0; k < polygons.size(); k++) {

			float** points = polygons[k]->getPoints();

			if (polygons[k]->getType() == LINE) {
				int positions[2][2] = { {0, 0}, {0, 0} };

				for (int j = 0; j < 2; j++) {
					Eigen::Vector4f p3D;
					p3D << points[j][0], points[j][1], points[j][2], 1;
					Eigen::Vector3f transP3D = P * p3D;
				/*	if (transP3D(2) < 0) {
						continue;
					}*/
					//float depth = sqrt(pow(transP3D[0], 2) + pow(transP3D[1], 2) + pow(transP3D[2], 2));

					Eigen::Vector3f p2D = K * transP3D;
					p2D(0) /= p2D(2);
					p2D(1) /= p2D(2);
					p2D(2) = 1;

					positions[j][0] = (int)(SCREEN_WIDTH - p2D(0));
					positions[j][1] = (int)(SCREEN_HEIGHT - p2D(1));

					
					//Eigen::Vector3f p3D2 = K.colPivHouseholderQr().solve(p2D);


					//if (!(xPos < 0 || xPos >= SCREEN_WIDTH || yPos < 0 || yPos >= SCREEN_HEIGHT)) { //if not out of bounds
					//	pixels[xPos][yPos][0] = depth;
					//	pixels[xPos][yPos][1] = k;
					//}
				}

				//points along the line
				int size;

				int** filledPoints = getLinePoints(positions[0], positions[1], size);

				//int p1[2] = { 202, 100 };
				//int p2[2] = { 60000, 200000 };
				//int** filledPoints = getLinePoints(p1, p2, size);



				for (int j = 0; j < size; j++) {
					if (!(filledPoints[j][0] < 0 || filledPoints[j][0] >= SCREEN_WIDTH || filledPoints[j][1] < 0 || filledPoints[j][1] >= SCREEN_HEIGHT)) { //if not out of bounds
						pixels[filledPoints[j][0]][filledPoints[j][1]][0] = 1;
						pixels[filledPoints[j][0]][filledPoints[j][1]][1] = k;
					}
					delete(filledPoints[j]);
				}
				delete(filledPoints);
			}
			
		}

		for (int x = 0; x < SCREEN_WIDTH; x++) {
			for (int y = 0; y < SCREEN_HEIGHT; y++) {
				if (pixels[x][y][1] == -1) {
					continue;
				}
				int* color = polygons[pixels[x][y][1]]->getColor();
				
				SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);
				SDL_RenderDrawPoint(renderer, x, y);
			}
		}
		//std::cout << "test" << std::endl;
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

//assumes line passes through origin. vector values will be coefficients of t.
//vector values for plane will be coefficients of x, y, z = (fourth value)
Eigen::Vector3f findIntersection(Eigen::Vector4f plane, Eigen::Vector3f line) {
	float t = plane(3) / (line(0) * plane(0) + line(1) * plane(1) + line(2) * plane(2));
	Eigen::Vector3f intersect = line * t;
	return intersect;
}

Eigen::Vector4f getPlane(Eigen::Vector3f p1, Eigen::Vector3f p2, Eigen::Vector3f p3) {
	Eigen::Vector3f norm = (p2 - p1).cross(p3 - p1);
	float k = norm.dot(p1);
	Eigen::Vector4f result(norm(0), norm(1), norm(2), k);
	return result;
}

int** getLinePoints(int p1Raw[2], int p2Raw[2], int &size) {

	int xDiff = p2Raw[0] - p1Raw[0];
	int yDiff = p2Raw[1] - p1Raw[1];

	int p1[2] = { p1Raw[0], p1Raw[1] };
	int p2[2] = { p2Raw[0], p2Raw[1] };
	cropLine(p1, p2);
	
	if (xDiff == 0 && yDiff == 0) {
		size = 1;
		int** points = new int* [1];
		points[0] = new int[2];
		points[0][0] = p1[0];
		points[0][1] = p1[1];
		return points;
	}
	//larger differce determines the number of points
	if (abs(xDiff) > abs(yDiff)) {
		int n = abs(p1[0] - p2[0]) + 1;
		size = n;
		float slope = (float)(yDiff) / (float)(xDiff);
		int** points = new int* [n];
		for (int i = 0; i < n; i++) {
			points[i] = new int[2];
			points[i][0] = p1[0] + i;
			points[i][1] = p1[1] + round(i * slope);
		}
		return points;
	}
	else {
		int n = abs(p1[1] - p2[1]) + 1;
		size = n;
		float slope = (float)(xDiff) / (float)(yDiff);
		int** points = new int* [n];
		for (int i = 0; i < n; i++) {
			points[i] = new int[2];
			points[i][1] = p1[1] + i;
			points[i][0] = p1[0] + round(i * slope);
		}
		return points;
	}
}

//returns 0 if line is completely outside screen,
//1 if partially inside screen, and 2 if completely insdie
//screen
int cropLine(int p1[2], int p2[2]) {
	if (isInScreen(p1) && isInScreen(p2)) {
		return 2;
	}
	if (!isInScreen(p1) && !isInScreen(p2)) {
		return 0;
	}
	int topLeft[2] = { 0, 0 };
	int topRight[2] = { SCREEN_WIDTH, 0 };
	int bottomLeft[2] = { 0, SCREEN_HEIGHT };
	int bottomRight[2] = { SCREEN_WIDTH, SCREEN_HEIGHT };
	int* croppedP = findLineIntersection(p1, p2, bottomLeft, topLeft);
	if (croppedP == NULL) {
		free(croppedP);
		croppedP = findLineIntersection(p1, p2, topLeft, topRight);
	}
	if (croppedP == NULL) {
		free(croppedP);
		croppedP = findLineIntersection(p1, p2, topRight, bottomRight);
	}
	if (croppedP == NULL) {
		free(croppedP);
		croppedP = findLineIntersection(p1, p2, bottomLeft, bottomRight);
	}
	if (croppedP == NULL) {
		std::cout << p1[0] << " " << p1[1] << " " << p2[0] << " " << p2[1] << std::endl;
	}
	assert(croppedP != NULL);
	if (isInScreen(p1)) {
		p2[0] = croppedP[0];
		p2[1] = croppedP[1];
	}
	else {
		p1[0] = croppedP[0];
		p1[1] = croppedP[1];
	}
	return 1;
}

//returns point of intersection of two lines or NULL if doesn't exist
//also returns NULL if intersection is not within both line segments
int* findLineIntersection(int p1[2], int p2[2], int p3[2], int p4[2]) {
	if (p2[0] - p1[0] == 0 && p4[0] - p3[0] == 0) {
		//std::cout << "returning null because both verticle" << std::endl;
		return NULL;
	}
	float x;
	float y;
	if (p2[0] - p1[0] == 0) {
		x = p1[0];
		float m2 = (float)(p4[1] - p3[1]) / (float)(p4[0] - p3[0]);
		float b2 = p3[1] - m2 * p3[0];
		y = m2 * x * b2;
	} else if (p4[0] - p3[0] == 0) {
		x = p3[0];
		float m1 = (float)(p2[1] - p1[1]) / (float)(p2[0] - p1[0]);
		float b1 = p1[1] - m1 * p1[0];
		y = m1 * x + b1;
	}
	else {
		float m1 = (float)(p2[1] - p1[1]) / (float)(p2[0] - p1[0]);
		float b1 = p1[1] - m1 * p1[0];
		float m2 = (float)(p4[1] - p3[1]) / (float)(p4[0] - p3[0]);
		float b2 = p3[1] - m2 * p3[0];
		if (m1 - m2 == 0) { //parallel
			//std::cout << "returning null because parallel slope" << std::endl;
			return NULL;
		}
		x = (b2 - b1) / (m1 - m2);
		y = m1 * x + b1;
	}
	int* pos = new int[2];
	pos[0] = round(x);
	pos[1] = round(y);
	//check if point is within both line segments
	if (((p1[0] >= p2[0] && pos[0] >= p2[0] && pos[0] <= p1[0]) || //on first line segment
		(p1[0] <= p2[0] && pos[0] >= p1[0] && pos[0] <= p2[0])) &&
		((p3[0] >= p4[0] && pos[0] >= p4[0] && pos[0] <= p3[0]) || //on second line segment
			(p3[0] <= p4[0] && pos[0] >= p3[0] && pos[0] <= p4[0]))) {
		return pos;
	}
	/*std::cout << "returning null because intersect is not on segments. Points:" << std::endl;
	std::cout << p1[0] << " " << p1[1] << std::endl;
	std::cout << p2[0] << " " << p2[1] << std::endl;
	std::cout << p3[0] << " " << p3[1] << std::endl;
	std::cout << p4[0] << " " << p4[1] << std::endl;
	std::cout << "intersect: " << std::endl;
	std::cout << pos[0] << " " << pos[1] << std::endl;*/
	return NULL;
}

bool isInScreen(int p[2]) {
	if (p[0] < 0 || p[0] > SCREEN_WIDTH || p[1] < 0 || p[1] > SCREEN_HEIGHT) {
		return false;
	}
	return true;
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