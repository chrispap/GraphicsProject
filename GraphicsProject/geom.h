#ifndef GEOM_H
#define GEOM_H

#include <iostream>
#include <vector>
#include <list>

#ifndef QT_CORE_LIB
#ifdef __linux__
#include <GL/glut.h>
#else
#include "gl/glut.h"
#endif
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

using namespace std;

struct Point {
    union {	
	  struct { float x, y, z;};
	  float data[3];
	};

	Point()
	{
	}

    Point(float _x, float _y, float _z) 
	{
        x=_x; y=_y; z=_z;
    }

    void print()
    {
        cout << "(" << x << ", " << y << ", " << z << ")" << endl;
    }

    Point &add(const Point &v)
    {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }

    Point &sub(const Point &v)
    {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }

    Point &scale(const float s)
    {
        x *= s; y *= s; z *= s;
        return *this;
    }

    static Point crossprod(const Point &v1, const Point &v2)
    {
        return Point( v1.y * v2.z - v1.z * v2.y,
                       v1.z * v2.x - v1.x * v2.z,
                       v1.x * v2.y - v1.y * v2.x );
    }

    static float dotprod(const Point &v1, const Point &v2)
    {
        return v1.x * v2.x +
               v1.y * v2.y +
               v1.z * v2.z;
    }

    static unsigned char mkcode(const Point &v, const Point &c1, const Point &c2)
    {
      unsigned char code = 0x00;
      if (v.z>c2.z) code|=0x01; else if (v.z<c1.z) code |=0x02;
      if (v.y>c2.y) code|=0x04; else if (v.y<c1.y) code |=0x08;
      if (v.x>c2.x) code|=0x10; else if (v.x<c1.x) code |=0x20;
      return code;
    }

};

struct Line
{
	Point start, end;

	Line()
	{
	}

	Line(const Point &_start, const Point &_end)
	{
		start = _start;
		end = _end;
	}

	~Line(void)
	{
	}
};

struct Box
{
	Point min;
	Point max;

	Box()
	{
	}

	Box(const Point &vmin, const Point &vmax)
	{
		min = vmin;
		max = vmax;
	}

	Box(const Point &v1, const Point &v2, const Point &v3)
	{
      min = Point(std::min(v1.x, std::max(v2.x, v3.x)),
                   std::min(v1.y, std::max(v2.y, v3.y)),
                   std::min(v1.z, std::max(v2.z, v3.z)));
      max = Point(std::max(v1.x, std::max(v2.x, v3.x)),
                   std::max(v1.y, std::max(v2.y, v3.y)),
                   std::max(v1.z, std::max(v2.z, v3.z)));
                   
	}

	float getXSize() const { return max.x - min.x;}
	float getYSize() const { return max.y - min.y;}
	float getZSize() const { return max.z - min.z;}
	
	void draw() const
	{
		Point p[8] = { 
			Point(min.x, min.y, min.z), //0
			Point(min.x, max.y, min.z), //1
			Point(min.x, max.y, max.z), //2
			Point(min.x, min.y, max.z), //3
			Point(max.x, min.y, min.z), //4
			Point(max.x, max.y, min.z), //5
			Point(max.x, max.y, max.z), //6
			Point(max.x, min.y, max.z), //7
		};
		
		int i;
		
		glBegin(GL_LINE_LOOP);
		for (i=0; i<4; ++i) glVertex3fv(p[i].data);
		glEnd();

		glBegin(GL_LINE_LOOP);
		for (i=4; i<8; ++i) glVertex3fv(p[i].data);
		glEnd();

		glBegin(GL_LINES);
		for (i=0; i<4; ++i) {
			glVertex3fv(p[i].data);
			glVertex3fv(p[i+4].data);
		}
		glEnd();

	}

	Box &add(const Point &v)
    {
        min.add(v);
        max.add(v);
        return *this;
    }

    Box &sub(const Point &v)
    {
        min.sub(v);
        max.sub(v);
        return *this;
    }

    Box &scale(const float s)
    {
        min.scale(s);
        max.scale(s);
        return *this;
    }
	
	static bool intersect(const Box &b1, const Box &b2)
	{
		return (b1.min.x < b2.max.x) && (b1.max.x > b2.min.x) &&
			   (b1.min.y < b2.max.y) && (b1.max.y > b2.min.y) &&
			   (b1.min.z < b2.max.z) && (b1.max.z > b2.min.z);
	}

};

struct Triangle
{	
	vector<Point> *vecList;	// Pointer to the vector containing the mVertices
	int vi1, vi2, vi3;		// Indices to the above vector
	float A, B, C, D;		// Plane equation coefficients
	Box box;				// Bounding box of the triangle
	bool deleted;			// Flag indicating that a triangle should be considered deleted
	
    Triangle(vector<Point> *_vecList, int _v1, int _v2, int _v3)
    {
        vi1 = _v1;
        vi2 = _v2;
        vi3 = _v3;
        vecList = _vecList;
        deleted=0;
        updateData();
    }
    
	const Point &v1() const { return (*vecList)[vi1];}
	const Point &v2() const { return (*vecList)[vi2];}
	const Point &v3() const { return (*vecList)[vi3];}
	const Box &getBox() const { return box;}
	const Point getNormal() const { return Point(A,B,C);}
	const Point getNormal2() const { return Point(v1()).add(v2()).add(v3()).scale(1.0f/3).add(Point(A,B,C));}
	const Point getCenter() const { return Point(v1()).add(v2()).add(v3()).scale(1.0f/3);}
	
	void updateData()
	{
		box = Box(v1(), v2(), v3());
		A = v1().y*(v2().z-v3().z) + v2().y*(v3().z-v1().z) + v3().y*(v1().z-v2().z);
		B = v1().z*(v2().x-v3().x) + v2().z*(v3().x-v1().x) + v3().z*(v1().x-v2().x);
		C = v1().x*(v2().y-v3().y) + v2().x*(v3().y-v1().y) + v3().x*(v1().y-v2().y);
		D = -v1().x*(v2().y*v3().z-v3().y*v2().z) -v2().x*(v3().y*v1().z-v1().y*v3().z) -v3().x*(v1().y*v2().z-v2().y*v1().z);
	}

	float planeEquation(const Point &r) const
	{
		return A*r.x + B*r.y + C*r.z + D;
		//return Point::dotprod(getNormal(), Point(r).sub(v3()));
	}

	static bool intersects (const Triangle &t, const Line &l)
	{
		if (t.planeEquation(l.start) * t.planeEquation(l.end) > 0)
			return false;
        else {
			return true;
			
			/// Oxi aparaitita, thelei diereynisi.
		}
	}

	static bool intersects(const Triangle &t1, const Triangle &t2)
	{
		/* 1. Arxika elegxoume an sygkrouontai ta bounding boxes,
		 * 2. stin synexeia elegxoume akmh-akmh,
		 * 3. kai an telika den yparxei sygkrousi, 
		 *    simainei oti ta bounding boxes eipan psemata.
		 */
		if (!Box::intersect(t1.box, t2.box)) 
			return false;
		else if ( 
			intersects(t1, Line(t2.v1(), t2.v2())) ||
			intersects(t1, Line(t2.v2(), t2.v3())) ||
			intersects(t1, Line(t2.v3(), t2.v1())) ||
			intersects(t2, Line(t1.v1(), t1.v2())) ||
			intersects(t2, Line(t1.v2(), t1.v3())) ||
			intersects(t2, Line(t1.v3(), t1.v1()))) 
			return true;
		else 
			return false;
	}
	
};

#endif
