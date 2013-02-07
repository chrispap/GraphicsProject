/** @file geom.h
 * Defintion and inline implementation of various geometrical classes.
 *
 * Containts classes used to manipulate geometrical shapes.
 */

#ifndef GEOM_H
#define GEOM_H

#include <iostream>
#include <cfloat>
#include <vector>
#include <list>
#include <math.h>

#ifndef QT_CORE_LIB
#ifdef __linux__
#include <GL/glut.h>
#else
#include "gl/glut.h"
#endif
#else
#include <GL/glu.h>
#endif

static const float PI = 3.14159f;

using namespace std;

/**
 * Struct that containts the data of a color in rgb byte format
 */
struct Colour {
    union {    struct { unsigned char r, g, b;};
    unsigned char data[3];};
    Colour (unsigned char _r, unsigned char _g, unsigned char _b):r(_r), g(_g), b(_b) {}
};

/**
 * Struct that containts 3 float numbers.
 */
struct Vector3f {
    union {
        struct { float x, y, z;};
        float data[3];
    };

    Vector3f(float _x=0, float _y=0, float _z=0): x(_x), y(_y), z(_z) { }

    bool operator== (const Vector3f &p) { return (x == p.x && y == p.y && z == p.z); }

    bool operator!= (const Vector3f &p) { return !(*this == p); }

    float r() { return sqrt(x*x+y*y+z*z);}

    void print() { cout << "(" << x << ", " << y << ", " << z << ")" << endl;}

    Vector3f &add(const Vector3f &v) { x += v.x; y += v.y; z += v.z; return *this;}

    Vector3f &sub(const Vector3f &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

    Vector3f &scale(const float s) { x *= s; y *= s; z *= s; return *this; }
};

/**
 * Class that containts a point in the 3D space.
 */
typedef Vector3f Point;

struct Line {
    Point start; ///< The starting edge of the line
    Point end;   ///< The ending edge of the line

    Line(): start(0,0,0), end(0,0,0) {}

    Line (const Point &_start, const Point &_end): start(_start), end(_end) {}

    ~Line(void) {}
};

/**
 * Struct that containts a Box
 */
struct Box {
    Point min; ///< The bottom-left-close corner of the box
    Point max; ///< The top-right-far corner of the box

    Box(): min(0,0,0), max(0,0,0) {}

    Box(const Point &vmin, const Point &vmax): min(vmin), max(vmax) {}

    Box(const Point &v1, const Point &v2, const Point &v3) {
        min = Point(std::min(v1.x, std::min(v2.x, v3.x)),
                    std::min(v1.y, std::min(v2.y, v3.y)),
                    std::min(v1.z, std::min(v2.z, v3.z)));

        max = Point(std::max(v1.x, std::max(v2.x, v3.x)),
                    std::max(v1.y, std::max(v2.y, v3.y)),
                    std::max(v1.z, std::max(v2.z, v3.z)));
    }

    Box(const vector<Point> &vertices) {
        Point min,max;
        min.x = min.y = min.z = FLT_MAX;
        max.x = max.y = max.z = FLT_MIN;
        vector<Point>::const_iterator vi;
        for (vi=vertices.begin(); vi!=vertices.end(); ++vi) {
            if      (vi->x > max.x) max.x = vi->x;
            else if (vi->x < min.x) min.x = vi->x;
            if      (vi->y > max.y) max.y = vi->y;
            else if (vi->y < min.y) min.y = vi->y;
            if      (vi->z > max.z) max.z = vi->z;
            else if (vi->z < min.z) min.z = vi->z;
        }
        this->min = min;
        this->max = max;
    }

    Box &forceMax (Box &maxBox) {
        if (min.x < maxBox.min.x) min.x = maxBox.min.x;
        if (min.y < maxBox.min.y) min.y = maxBox.min.y;
        if (min.z < maxBox.min.z) min.z = maxBox.min.z;
        if (max.x > maxBox.max.x) max.x = maxBox.max.x;
        if (max.y > maxBox.max.y) max.y = maxBox.max.y;
        if (max.z > maxBox.max.z) max.z = maxBox.max.z;
        return *this;
    }

    float getXSize() const { return max.x - min.x;}

    float getYSize() const { return max.y - min.y;}

    float getZSize() const { return max.z - min.z;}

    Point getSize() const { return Point(getXSize(),getYSize(), getZSize());}

    float getMinSize() const { return std::min(getXSize(),std::min(getYSize(),getZSize()));}

    float getMaxSize() const { return std::max(getXSize(),std::max(getYSize(),getZSize()));}

    float getVolume() const { return fabs((max.x - min.x)*(max.y - min.y)*(max.z - min.z)); }

    Box &add(const Point &v) { min.add(v); max.add(v); return *this; }

    Box &sub(const Point &v) {min.sub(v); max.sub(v);return *this; }

    Box &scale(const float s) {min.scale(s); max.scale(s);return *this;}

    void draw(const Colour &col, unsigned char a=0) const {
        static Point p[8];
        Point *v = p;
        *v++ = Point(min.x, min.y, min.z), //0
        *v++ = Point(min.x, max.y, min.z), //1
        *v++ = Point(min.x, max.y, max.z), //2
        *v++ = Point(min.x, min.y, max.z), //3
        *v++ = Point(max.x, min.y, min.z), //4
        *v++ = Point(max.x, max.y, min.z), //5
        *v++ = Point(max.x, max.y, max.z), //6
        *v++ = Point(max.x, min.y, max.z), //7


        glPolygonMode(GL_FRONT_AND_BACK, a?GL_FILL: GL_LINE);
        glBegin(GL_QUADS);
        if (a) glColor4ub(col.r, col.g, col.b, a);
        else glColor3ubv(col.data);

        glVertex3fv(p[0].data);
        glVertex3fv(p[1].data);
        glVertex3fv(p[2].data);
        glVertex3fv(p[3].data);

        glVertex3fv(p[1].data);
        glVertex3fv(p[2].data);
        glVertex3fv(p[6].data);
        glVertex3fv(p[5].data);

        glVertex3fv(p[4].data);
        glVertex3fv(p[5].data);
        glVertex3fv(p[6].data);
        glVertex3fv(p[7].data);

        glVertex3fv(p[0].data);
        glVertex3fv(p[4].data);
        glVertex3fv(p[7].data);
        glVertex3fv(p[3].data);

        glVertex3fv(p[2].data);
        glVertex3fv(p[3].data);
        glVertex3fv(p[7].data);
        glVertex3fv(p[6].data);

        glVertex3fv(p[0].data);
        glVertex3fv(p[1].data);
        glVertex3fv(p[5].data);
        glVertex3fv(p[4].data);
        glEnd();
    }
};

/**
 * Struct that contains a sphere.
 */
struct Sphere {
    Point center;   ///< The centre of the sphere.
    float rad;      ///< The radius of the sphere.

    Sphere (): center(0,0,0), rad(0) {}

    Sphere (const Point &c, float r): center(c), rad(r) {}

    Sphere (const vector<Point> &vertices, const set<int> &indices) {
        bool all = !indices.size();
        float dx,dy,dz;
        float rad_sq,xspan,yspan,zspan,maxspan;
        float old_to_p,old_to_p_sq,old_to_new;
        Point xmin,xmax,ymin,ymax,zmin,zmax,dia1,dia2;
        vector<Point>::const_iterator vi;
        set<int>::const_iterator vii;

        /* FIRST PASS: find 6 minima/maxima points */
        xmin.x=ymin.y=zmin.z= FLT_MAX;
        xmax.x=ymax.y=zmax.z= FLT_MIN;
        if (all) {
            for (vi=vertices.begin(); vi!=vertices.end(); ++vi) {
                if (vi->x < xmin.x) xmin = *vi;
                if (vi->x > xmax.x) xmax = *vi;
                if (vi->y < ymin.y) ymin = *vi;
                if (vi->y > ymax.y) ymax = *vi;
                if (vi->z < zmin.z) zmin = *vi;
                if (vi->z > zmax.z) zmax = *vi;
            }
        }
        else {
            for (vii=indices.begin(); vii!=indices.end(); ++vii) {
                vi = vertices.begin() + *vii;
                if (vi->x < xmin.x) xmin = *vi;
                if (vi->x > xmax.x) xmax = *vi;
                if (vi->y < ymin.y) ymin = *vi;
                if (vi->y > ymax.y) ymax = *vi;
                if (vi->z < zmin.z) zmin = *vi;
                if (vi->z > zmax.z) zmax = *vi;
            }
        }

        /* Set xspan = distance between the 2 points xmin & xmax (squared) */
        dx = xmax.x - xmin.x;
        dy = xmax.y - xmin.y;
        dz = xmax.z - xmin.z;
        xspan = dx*dx + dy*dy + dz*dz;

        /* Same for y & z spans */
        dx = ymax.x - ymin.x;
        dy = ymax.y - ymin.y;
        dz = ymax.z - ymin.z;
        yspan = dx*dx + dy*dy + dz*dz;

        dx = zmax.x - zmin.x;
        dy = zmax.y - zmin.y;
        dz = zmax.z - zmin.z;
        zspan = dx*dx + dy*dy + dz*dz;

        /* Set points dia1 & dia2 to the maximally separated pair */
        dia1 = xmin; dia2 = xmax; /* assume xspan biggest */
        maxspan = xspan;
        if (yspan>maxspan) {
            maxspan = yspan;
            dia1 = ymin; dia2 = ymax;
        }
        if (zspan>maxspan) {
            dia1 = zmin; dia2 = zmax;
        }

        /* dia1,dia2 is a diameter of initial sphere */
        /* calc initial center */
        center.x = (dia1.x+dia2.x)/2.0;
        center.y = (dia1.y+dia2.y)/2.0;
        center.z = (dia1.z+dia2.z)/2.0;
        /* calculate initial radius**2 and radius */
        dx = dia2.x-center.x; /* x component of radius vector */
        dy = dia2.y-center.y; /* y component of radius vector */
        dz = dia2.z-center.z; /* z component of radius vector */
        rad_sq = dx*dx + dy*dy + dz*dz;
        rad = sqrt(rad_sq);

        if (all) {
            for (vi=vertices.begin(); vi!=vertices.end(); ++vi) {
                dx = vi->x-center.x;
                dy = vi->y-center.y;
                dz = vi->z-center.z;
                old_to_p_sq = dx*dx + dy*dy + dz*dz;
                if (old_to_p_sq > rad_sq) {
                    old_to_p = sqrt(old_to_p_sq);
                    /* calc radius of new sphere */
                    rad = (rad + old_to_p) / 2.0;
                    rad_sq = rad*rad;   /* for next r**2 compare */
                    old_to_new = old_to_p - rad;
                    /* calc center of new sphere */
                    center.x = (rad*center.x + old_to_new*vi->x) / old_to_p;
                    center.y = (rad*center.y + old_to_new*vi->y) / old_to_p;
                    center.z = (rad*center.z + old_to_new*vi->z) / old_to_p;
                }
            }
        }
        else {
            for (vii=indices.begin(); vii!=indices.end(); ++vii) {
                vi = vertices.begin() + *vii;
                dx = vi->x-center.x;
                dy = vi->y-center.y;
                dz = vi->z-center.z;
                old_to_p_sq = dx*dx + dy*dy + dz*dz;
                if (old_to_p_sq > rad_sq) {
                    old_to_p = sqrt(old_to_p_sq);
                    /* calc radius of new sphere */
                    rad = (rad + old_to_p) / 2.0;
                    rad_sq = rad*rad;   /* for next r**2 compare */
                    old_to_new = old_to_p - rad;
                    /* calc center of new sphere */
                    center.x = (rad*center.x + old_to_new*vi->x) / old_to_p;
                    center.y = (rad*center.y + old_to_new*vi->y) / old_to_p;
                    center.z = (rad*center.z + old_to_new*vi->z) / old_to_p;
                }
            }
        }
    }

    float getVolume() const { return (4.0/3.0)*PI*(rad*rad*rad); }

    Sphere &add(const Point &v) { center.add(v); return *this;}

    Sphere &sub(const Point &v) { center.sub(v); return *this;}

    Sphere &scale (const float s) { center.scale(s); rad*=s; return *this;}

    bool contains (const Point &v) const { float dx=center.x-v.x, dy=center.y-v.y, dz=center.z-v.z;  return dx*dx+dy*dy+dz*dz < rad*rad;}

    void draw(const Colour &col, unsigned char a=0) const {
        glPushMatrix();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glTranslatef(center.x, center.y, center.z);

        if (a) {
            glColor4ub(col.r, col.g, col.b, a);
            glutSolidSphere(rad, 32, 32);
        } else {
            glColor3ubv(col.data);
            glutWireSphere(rad, 32, 32);
        }

        glPopMatrix();
    }

};

/**
 * Struct that contains a triangle.
 *
 * Doesn't contain the actual data for the 3 vertices.
 * Instead, contains indices to a vector and a pointer
 * to that vector.
 */
struct Triangle {
    union { struct { int vi1, vi2, vi3;}; int v[3];};   // Indices to the vector below
    vector<Point> *vecList;     // Pointer to the vector containing the vertices
    float A, B, C, D;           // Plane equation coefficients
    Box box;                    // Bounding box of the triangle
    bool deleted;               // Flag indicating that a triangle should be considered deleted

    Triangle(vector<Point> *_vecList=NULL, int _v1=0, int _v2=0, int _v3=0):
            vi1(_v1),
            vi2(_v2),
            vi3(_v3),
            vecList(_vecList),
            deleted(0)
    {
        update();
    }

    void update() {
        box = Box(v1(), v2(), v3());
        A = v1().y*(v2().z-v3().z) + v2().y*(v3().z-v1().z) + v3().y*(v1().z-v2().z);
        B = v1().z*(v2().x-v3().x) + v2().z*(v3().x-v1().x) + v3().z*(v1().x-v2().x);
        C = v1().x*(v2().y-v3().y) + v2().x*(v3().y-v1().y) + v3().x*(v1().y-v2().y);
        D = -v1().x*(v2().y*v3().z-v3().y*v2().z) -v2().x*(v3().y*v1().z-v1().y*v3().z) -v3().x*(v1().y*v2().z-v2().y*v1().z);
    }

    const Point &v1() const { return (*vecList)[vi1];}

    const Point &v2() const { return (*vecList)[vi2];}

    const Point &v3() const { return (*vecList)[vi3];}

    const Box &getBox() const { return box;}

    const Point getNormal() const { Point n(A,B,C); return n.scale(1.0f/n.r());}

    const Point getCenter() const { return Point(v1()).add(v2()).add(v3()).scale(1.0f/3);}

    float planeEquation(const Point &r) const { return A*r.x + B*r.y + C*r.z + D;}
};

class Geom {

public:
    static Point crossprod (const Point &v1, const Point &v2)
    {
        return Point( v1.y * v2.z - v1.z * v2.y,
                      v1.z * v2.x - v1.x * v2.z,
                      v1.x * v2.y - v1.y * v2.x );
    }

    static float dotprod (const Point &v1, const Point &v2)
    {
        return v1.x * v2.x +
                v1.y * v2.y +
                v1.z * v2.z;
    }

    static char mkcode (const Point &v, const Box &b)
    {
        char code = 0x00;
        if (v.z>=b.max.z) code|=0x01; else if (v.z<=b.min.z) code |=0x02;
        if (v.y>=b.max.y) code|=0x04; else if (v.y<=b.min.y) code |=0x08;
        if (v.x>=b.max.x) code|=0x10; else if (v.x<=b.min.x) code |=0x20;
        return code;
    }

    static bool intersects (const Box &b1, const Box &b2)
    {
        return (b1.min.x < b2.max.x) && (b1.max.x > b2.min.x) &&
               (b1.min.y < b2.max.y) && (b1.max.y > b2.min.y) &&
               (b1.min.z < b2.max.z) && (b1.max.z > b2.min.z);
    }

    static bool intersects (const Box &b, const Line &l)
    {
        /* local variables declare static in order to
           reduce the storage required for recursion */
        static char c1, c2, bit, axis;
        static float tdl;
        static Point dl, i;
        static vector<Point> Rv(3);
        static Triangle R = Triangle(&Rv, 0,1,2);

        /* Trivial accept / reject */
        c1 = mkcode(l.start, b);
        c2 = mkcode(l.end, b);

        if (!(c1 | c2)) return true;
        if ( (c1 & c2)) return false;
        if ( !c1||!c2)  return true;

        return true; // return prematurely because the code below is incorrect

        /* Find a plane of intersection */
        bit = -1;
        while (!((c1^c2) & (1<<(++bit))));
        axis = 2-bit/2;

        /* Construct a triangle on the plane of intersection */
        Rv[0] = Rv[1] = Rv[2] = bit%2? b.min: b.max;
        Rv[1].data[(axis+1)%3]+=10;     // keep the value on the plane axis
        Rv[2].data[(axis+2)%3]+=10;     // and alter the values on other 2 axes
        R.update();                     // Force the triangle to compute its coefficients

        /* Find the point of intersection */
        dl = Point(l.end).sub(l.start);
        tdl = -R.planeEquation(l.start)/(R.planeEquation(dl)- R.D);
        i = Point(l.start).add(dl.scale(tdl));

        return (rand()%2)? intersects(b, Line(l.start, i)) : intersects(b, Line(i, l.end));
    }

    static bool intersects (const Triangle &t, const Line &l)
    {
        if (t.planeEquation(l.start) * t.planeEquation(l.end) > 0)
            return false;
        else
        {
            /* Find the intersection point */
            Point dl = Point(l.end).sub(l.start);
            float tdl = -t.planeEquation(l.start)/(t.planeEquation(dl)- t.D);
            Point i = Point(l.start).add(dl.scale(tdl));

            /* Temporary vector containing the 6 vertices
             * that form the 3 perpendicular planes around the triangle */
            Point N(t.getNormal());
            vector<Point> tempVec(6);
            tempVec[0] = tempVec[3] = t.v1();
            tempVec[1] = tempVec[4] = t.v2();
            tempVec[2] = tempVec[5] = t.v3();
            tempVec[3].add(N);
            tempVec[4].add(N);
            tempVec[5].add(N);
            float eq1 = Triangle(&tempVec, 0, 1, 3).planeEquation(i);
            float eq2 = Triangle(&tempVec, 1, 2, 4).planeEquation(i);
            float eq3 = Triangle(&tempVec, 2, 3, 5).planeEquation(i);

            if ( (eq1>0 && eq2>0 && eq3>0) || (eq1<0 && eq2<0 && eq3<0) )
                return true;
            else return false;
        }
    }

    static bool intersects (const Triangle &t1, const Triangle &t2)
    {
        /* Arxika elegxoume an sygkrouontai ta bounding boxes. */
        if (!intersects(t1.box, t2.box)) return false;

        /* Stin synexeia elegxoume akmh-akmh. */
        else if (
                intersects(t1, Line(t2.v1(), t2.v2())) ||
                intersects(t1, Line(t2.v2(), t2.v3())) ||
                intersects(t1, Line(t2.v3(), t2.v1())) ||
                intersects(t2, Line(t1.v1(), t1.v2())) ||
                intersects(t2, Line(t1.v2(), t1.v3())) ||
                intersects(t2, Line(t1.v3(), t1.v1())))
            return true;

        /* Telika den ypaarxei sygkrousi. */
        else return false;
    }

    static float distance (const Point p1, const Point p2)
    {
        float dx = (p1.x - p2.x),
              dy = (p1.y - p2.y),
              dz = (p1.z - p2.z);
        return sqrt(dx*dx + dy*dy + dz*dz);
    }

    static float intersectionVolume (const Sphere &s1, const Sphere &s2)
    {
        bool s1GTs2 = s1.rad>s2.rad;
        float R = s1GTs2? s1.rad : s2.rad;
        float r = s1GTs2? s2.rad : s1.rad;
        const Sphere &S = s1GTs2? s1 : s2;
        const Sphere &s = s1GTs2? s2 : s1;
        float d = distance(S.center, s.center);

        if (d >= R+r) return 0.0;
        if (d <= R-r) return s.getVolume();

        float tmp1 = R+r-d; tmp1 = tmp1*tmp1;
        float tmp2 = (d*d) + (2.0*d*r) - (3.0*r*r) + (2.0*d*R) + (6.0*R*r) - (3.0*R*R);
        return ((PI * tmp1 * tmp2)) / (12.0*d);

    }

};

#endif
