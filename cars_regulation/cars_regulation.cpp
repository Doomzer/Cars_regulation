#include "pch.h"
#include <stdlib.h>
#include <vector>
#include <unordered_set>

#include <windows.h>
#include <iostream>
#include <cmath>

#include <algorithm>

struct sPos 
{
	sPos() { x = 0; y = 0; }
	sPos(int aX, int aY) { x = aX; y = aY; }
	int x;
	int y;
};

struct sSize
{
	sSize() { width = 0; height = 0; }
	sSize(int aW, int aH) { width = aW; height = aW; }
	int width;
	int height;
};

struct sRect
{
	sRect() {};
	sRect(int x, int y, int w, int h) { pos.x = x; pos.y = y; size.width = w; size.height = h; }
	sPos pos;
	sSize size;
	bool intersects(const sRect& other) 
	{
		return !((other.pos.x + other.size.width < pos.x) || (other.pos.y + other.size.height < pos.y) || (other.pos.x > pos.x + size.width) || (other.pos.y > pos.y + size.height));
	}
};

enum class eDirection 
{
	UP,
	LEFT,
	RIGHT,
	DOWN,
	EMPTY
};

eDirection movingNow = eDirection::EMPTY;
std::unordered_set<eDirection> sides_blocked;

struct sCar 
{
	sRect rect;
	eDirection dir;
	int speed;

	virtual ~sCar(){}

	virtual void move()
	{
		switch (dir) 
		{
		case eDirection::UP:
			rect.pos.y += speed;
			break;
		case eDirection::DOWN:
			rect.pos.y -= speed;
			break;
		case eDirection::RIGHT:
			rect.pos.x += speed;
			break;
		case eDirection::LEFT:
			rect.pos.x -= speed;
			break;
		}
	}

	sRect getFuturePos() 
	{
		switch (dir) 
		{
		case eDirection::UP:
			return sRect(rect.pos.x,                           rect.pos.y,                            rect.size.width,             rect.size.height * 2 + speed);
			break;
		case eDirection::DOWN:
			return sRect(rect.pos.x,                           rect.pos.y - rect.size.height - speed, rect.size.width,             rect.size.height * 2 + speed);
			break;
		case eDirection::RIGHT:
			return sRect(rect.pos.x,                           rect.pos.y,                            rect.size.width * 2 + speed, rect.size.height);
			break;
		case eDirection::LEFT:
			return sRect(rect.pos.x - rect.size.width - speed, rect.pos.y,                            rect.size.width * 2 + speed, rect.size.height);
			break;
		}
	}
	
	bool needPassOtherCar(sCar* otherCar)
	{
		bool result = false;
		switch (dir)
		{
		case eDirection::UP:
		{
			if (otherCar->dir == eDirection::UP)
				return rect.pos.y < otherCar->rect.pos.y;
			if (otherCar->dir == eDirection::LEFT)
				return rect.pos.y + rect.size.height < otherCar->rect.pos.y;
			else
				if (sides_blocked.size() < 4)
					return rect.pos.x < otherCar->rect.pos.x + otherCar->rect.size.width;
				else
					return rect.pos.y + rect.size.height < otherCar->rect.pos.y;
		}
		break;
		case eDirection::DOWN:
		{
			if (otherCar->dir == eDirection::DOWN)
				return rect.pos.y > otherCar->rect.pos.y;
			if (otherCar->dir == eDirection::RIGHT)
				return rect.pos.y > otherCar->rect.pos.y + otherCar->rect.size.height;
			else
				if (sides_blocked.size() < 4)
					return rect.pos.x + rect.size.width > otherCar->rect.pos.x;
				else
					return rect.pos.y > otherCar->rect.pos.y + otherCar->rect.size.height;
		}
		break;
		case eDirection::RIGHT:
		{
			if (otherCar->dir == eDirection::RIGHT)
				return rect.pos.x < otherCar->rect.pos.x;
			if (otherCar->dir == eDirection::UP)
				if (sides_blocked.size() < 4)
					return rect.pos.x + rect.size.width < otherCar->rect.pos.x;
				else
					return rect.pos.y < otherCar->rect.pos.y + otherCar->rect.size.height;
			else
				return rect.pos.y + rect.size.height > otherCar->rect.pos.y;
		}
		break;
		case eDirection::LEFT:
		{
			if (otherCar->dir == eDirection::LEFT)
				return rect.pos.x > otherCar->rect.pos.x;
			if (otherCar->dir == eDirection::DOWN)
				if (sides_blocked.size() < 4)
					return rect.pos.x > otherCar->rect.pos.x + otherCar->rect.size.width;
				else
					return rect.pos.y + otherCar->rect.size.height > otherCar->rect.pos.y;
			else
				return rect.pos.y < otherCar->rect.pos.y + otherCar->rect.size.height;
		}
		break;
		}
		return result;
	}

	virtual int getFuel() = 0;
	virtual void refill(int count) = 0;
};

struct sGasEngine : virtual sCar
{
	virtual ~sGasEngine() {}
	virtual int getFuel() { return fuel; }
	virtual void refill(int count) { fuel += count; }
	virtual void move() { fuel--; sCar::move(); }
	int fuel;
};

struct sElectroCar : virtual sCar
{
	virtual ~sElectroCar() {}
	virtual int getFuel() { return charge; }
	virtual void refill(int count) { charge += count; }
	virtual void move() { charge--; sCar::move(); }
	int charge;
};

struct sHybrid : sGasEngine, sElectroCar 
{
	void refill(int count) { charge += count / 2; fuel += count / 2; }
	int getFuel() { return charge + fuel; }
	void move() 
	{
		if (rand() % 2 == 0)
			charge--;
		else
			fuel--;
		sCar::move();
	}
};

std::vector<sCar*> cars;
const int initialCarsCount = 15;
constexpr auto SCREEN_WIDTH = 512;	//1024;
constexpr auto SCREEN_HEIGHT = 388;	//768;
constexpr auto CAR_WIDTH = 20;		//100
constexpr auto CAR_HEIGHT = 20;		//100

void check_cars(sCar* car)
{
	for (auto car_old : cars)
	{
		if (car->rect.intersects(car_old->rect))
		{
			switch (car->dir)
			{
			case eDirection::UP:
			{
				car->rect.pos.y = car_old->rect.pos.y + CAR_HEIGHT * 2.1;
			}
			break;
			case eDirection::DOWN:
			{
				car->rect.pos.y = car_old->rect.pos.y - CAR_HEIGHT * 2.1;
			}
			break;
			case eDirection::RIGHT:
			{
				car->rect.pos.x = car_old->rect.pos.x + CAR_WIDTH * 2.1;
			}
			break;
			case eDirection::LEFT:
			{
				car->rect.pos.x = car_old->rect.pos.x - CAR_WIDTH * 2.1;
			}
			break;
			}
			check_cars(car);
			return;
		}
	}
}

void spawnCarRandom();

void spawnCar(eDirection dir)
{
	int dir_count = 0;
	for (auto car : cars)
	{
		if (car->dir == dir)
			dir_count++;
	}

	if (dir_count > initialCarsCount / 3)
	{
		return spawnCarRandom();
	}

	sCar* car;
	int carType = rand();
	if (carType % 3 == 0)
	{
		car = new sGasEngine();
	}
	else if (carType % 3 == 1)
	{
		car = new sElectroCar();
	}
	else
	{
		car = new sHybrid();
	}	

	car->dir = dir;

	switch (dir)
	{
	case eDirection::UP:
	{
		car->rect = sRect(SCREEN_WIDTH / 2 + CAR_WIDTH, 0, CAR_WIDTH, CAR_HEIGHT);
	}
	break;
	case eDirection::DOWN:
	{
		car->rect = sRect(SCREEN_WIDTH / 2 - CAR_WIDTH, SCREEN_HEIGHT, CAR_WIDTH, CAR_HEIGHT);
	}
	break;
	case eDirection::RIGHT:
	{
		car->rect = sRect(0, SCREEN_HEIGHT / 2 - CAR_HEIGHT, CAR_WIDTH, CAR_HEIGHT);
	}
	break;
	case eDirection::LEFT:
		car->rect = sRect(SCREEN_WIDTH, SCREEN_HEIGHT / 2 + CAR_HEIGHT, CAR_WIDTH, CAR_HEIGHT);
	break;
	}

	car->speed = 10;

	check_cars(car);

	cars.push_back(car);
}

void spawnCarRandom()
{
	if (rand() % 4 == 1)
		spawnCar(eDirection::LEFT);
	else 
		if (rand() % 4 == 2)
		spawnCar(eDirection::DOWN);
	else if (rand() % 4 == 3)
		spawnCar(eDirection::UP);
	else
		spawnCar(eDirection::RIGHT);
}


void draw()
{
	static int test_count = 0;
	std::cout << test_count++<<std::endl;
	std::cout << sides_blocked.size();
	HWND hwnd = GetConsoleWindow();
	HDC hdc = GetDC(hwnd);
	for (auto car : cars)
	{
		for (int x = car->rect.pos.x + 100; x < (car->rect.pos.x + car->rect.size.width) + 100; x++)
		{
			for (int y = car->rect.pos.y + 100; y < (car->rect.pos.y + car->rect.size.height) + 100; y++)
			{
				SetPixel(hdc, x, SCREEN_HEIGHT - y + 150, RGB(255, 255, 255));
			}
		}	
		
		int x_futur = car->rect.pos.x;
		int y_futur = car->rect.pos.y;
		switch (car->dir)
		{
		case eDirection::UP:
			y_futur += car->rect.size.height + car->speed;
			break;
		case eDirection::DOWN:
			y_futur -= car->rect.size.height + car->speed;
			break;
		case eDirection::RIGHT:
			x_futur += car->rect.size.width + car->speed;
			break;
		case eDirection::LEFT:
			x_futur -= car->rect.size.width + car->speed;
			break;
		}		
		for (int x = x_futur + 100; x < (x_futur + car->rect.size.width) + 100; x++)
		{
			for (int y = y_futur + 100; y < (y_futur + car->rect.size.height) + 100; y++)
			{
				//SetPixel(hdc, x, SCREEN_HEIGHT - y + 150, RGB(255, 0, 0));
			}
		}
		
	}
	ReleaseDC(hwnd, hdc);
	std::cin.ignore();
	system("cls");
}

bool main_loop() 
{
	bool can_move = false;
	while (true)
	{
		draw();
		sides_blocked.clear();
		for (auto car : cars)
		{
			for (auto other_car : cars)
			{
				sRect future_pos = other_car->getFuturePos();
				if((future_pos.pos.x > SCREEN_WIDTH / 2 - (CAR_WIDTH * 3 + other_car->speed)) && (future_pos.pos.x < SCREEN_WIDTH / 2 + CAR_WIDTH * 3 + other_car->speed) && (future_pos.pos.y > SCREEN_HEIGHT / 2 - (CAR_HEIGHT * 3 + other_car->speed)) && (future_pos.pos.y < SCREEN_HEIGHT / 2 + CAR_HEIGHT * 3 + other_car->speed))
					sides_blocked.insert(other_car->dir);
			}
			for (auto other_car : cars)
			{
				can_move = true;
				if (car != other_car)
				{
					if (car->getFuturePos().intersects(other_car->getFuturePos()))
					{
						if (car->needPassOtherCar(other_car))
						{
							can_move = false;
							break;
						}	
					}
				}
			}
			if (can_move)
			{
				car->move();				
			}
			if (car->rect.pos.x < 0 || car->rect.pos.x > SCREEN_WIDTH || car->rect.pos.y < 0 || car->rect.pos.y > SCREEN_HEIGHT)
			{
				auto it = find(cars.begin(), cars.end(), car);
				cars.erase(it);
				delete car;
				spawnCarRandom();
			}
		}
	}
}

int main(int argc, char** argv) 
{
	std::cin.ignore();
	for (auto i = 0; i < initialCarsCount; ++i) 
	{
		spawnCarRandom();
	}	
	main_loop();
	return 0;
}
