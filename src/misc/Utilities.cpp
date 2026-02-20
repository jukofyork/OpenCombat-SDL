#include "./Utilities.h"
#include <misc/Structs.h>

Utilities::Utilities(void)
{
}

Utilities::~Utilities(void)
{
}

// Finds the heading from (x0,y0) - (x1,y1)
Direction
Utilities::FindHeading(int x0, int y0, int x1, int y1)
{
	return ConvertAngle(FindAngle(x0, y0, x1, y1));
}

Direction
Utilities::FindHeading(Vector2 *dir)
{
	dir->y *= -1.0f;
	return ConvertAngle(FindAngle(dir));
}

float
Utilities::FindAngle(int x0, int y0, int x1, int y1)
{
	Vector2 dir;
	dir.x = static_cast<float>(x1 - x0);
	dir.y =  static_cast<float>(y0 - y1);
	dir.Normalize();
	return FindAngle(&dir);
}

float
Utilities::FindAngle(Vector2 *dir)
{
	float angle;
	if(dir->x == 0.0f) {
		if(dir->y > 0) {
			angle = static_cast<float>(M_PI/2.0f);
		} else {
			angle = static_cast<float>(3.0f*M_PI/2.0f);
		}
	} else if(dir->y == 0.0f) {
		if(dir->x > 0.0) {
			angle = 0.0f;
		} else {
			angle = static_cast<float>(M_PI);
		}
	} else {
		angle = fabs(atan(dir->y / dir->x));
		if(dir->x < 0.0f && dir->y > 0.0f) {
			angle = static_cast<float>(M_PI) - angle;
		} else if(dir->x < 0.0f && dir->y < 0.0f) {
			angle += static_cast<float>(M_PI);
		} else if(dir->x > 0.0f && dir->y > 0.0f) {
		} else if(dir->x > 0.0f && dir->y < 0.0f) {
			angle = static_cast<float>(2.0f*M_PI) - angle;
		}
	}
	return angle;
}

static float _directionAngles[] = { 6.0f*static_cast<float>(M_PI)/4.0f, 5.0f*static_cast<float>(M_PI)/4.0f,4.0f*static_cast<float>(M_PI)/4.0f,3.0f*static_cast<float>(M_PI)/4.0f,2.0f*static_cast<float>(M_PI)/4.0f, 1.0f*static_cast<float>(M_PI)/4.0f,0.0f,7.0f*static_cast<float>(M_PI)/4.0f};

float
Utilities::FindAngle(Direction dir)
{
	return _directionAngles[dir];
}

Direction 
Utilities::ConvertAngle(float angle)
{
	if(angle < 0.0) {
		angle += static_cast<float>(2.0f*M_PI);
	}

	float da = static_cast<float>(2.0f*M_PI / 16.0f);

	// Start South, go clockwise
	if(angle <=  ((6.0f*2.0f*M_PI / 8.0) + da)
		&& angle >=  ((6.0f*2.0f*M_PI / 8.0) - da)) 
	{
		return South;
	} 
	else if(angle <=  ((5.0f*2.0f*M_PI / 8.0) + da)
		&& angle >=  ((5.0f*2.0f*M_PI / 8.0) - da)) 
	{
		return SouthWest;
	}
	else if(angle <=  ((4.0f*2.0f*M_PI / 8.0) + da)
		&& angle >=  ((4.0f*2.0f*M_PI / 8.0) - da)) 
	{
		return West;
	}
	else if(angle <=  ((3.0f*2.0f*M_PI / 8.0) + da)
		&& angle >=  ((3.0f*2.0f*M_PI / 8.0) - da)) 
	{
		return NorthWest;
	}
	else if(angle <=  ((2.0f*2.0f*M_PI / 8.0) + da)
		&& angle >=  ((2.0f*2.0f*M_PI / 8.0) - da)) 
	{
		return North;
	}
	else if(angle <=  ((1.0f*2.0f*M_PI / 8.0) + da)
		&& angle >=  ((1.0f*2.0f*M_PI / 8.0) - da)) 
	{
		return NorthEast;
	}
	else if(angle <=  ((7.0f*2.0f*M_PI / 8.0) + da)
		&& angle >=  ((7.0f*2.0f*M_PI / 8.0) - da)) 
	{
		return SouthEast;
	} 
	else 
	{
		return East;
	}
}

void 
Utilities::Rotate(Point *dst, const Point *src, float angle)
{
	double cosT = cos(angle);
	double sinT = sin(angle);
	dst->x = static_cast<int>((src->x)*cosT + (src->y)*sinT);
	dst->y = static_cast<int>((src->y)*cosT - (src->x)*sinT);
}

