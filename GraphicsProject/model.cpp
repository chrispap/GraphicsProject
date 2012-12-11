#include <stdio.h>
#include <ctime>
#include <cfloat>
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include "model.h"
#include "geom.h"

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

/* Constructrors */
Model::Model(string filename, bool ccw,  bool vt)
{
 clock_t t = clock();
    loadTrianglesFromOBJ(filename, mVertices, mTriangles, ccw, vt);
	createTriangleLists();
	createBoundingBox();
	updateTriangleData();
	printf ("Model loading took:\t%4.2f sec | %d triangles | Kalypsi: %3.1f \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size(), boxCoverage());
}

Model::Model(const Model &m1, const Model &m2, bool both)
{
 clock_t t = clock();
	findCollisions(m1, m2, mVertices, mTriangles, both);
	createTriangleLists();
	createBoundingBox();
	updateTriangleData();
 printf ("Model collision took:\t%4.2f sec | %d triangles.\n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}

Model::~Model()
{

}

void Model::createBoundingBox()
{
    Point boxMin,boxMax;
	boxMin.x = boxMin.y = boxMin.z = FLT_MAX;
    boxMax.x = boxMax.y = boxMax.z = FLT_MIN;

    vector<Point>::const_iterator vi;
    for (vi=mVertices.begin(); vi!=mVertices.end(); ++vi) {
        boxMax.x = vi->x > boxMax.x? vi->x: boxMax.x;
        boxMax.y = vi->y > boxMax.y? vi->y: boxMax.y;
        boxMax.z = vi->z > boxMax.z? vi->z: boxMax.z;
        boxMin.x = vi->x < boxMin.x? vi->x: boxMin.x;
        boxMin.y = vi->y < boxMin.y? vi->y: boxMin.y;
        boxMin.z = vi->z < boxMin.z? vi->z: boxMin.z;
    }

	mBox = Box(boxMin, boxMax);
}

float Model::boxCoverage()
{
	
	return 50.0;
}

void Model::createTriangleLists()
{
	mVertexTriangles.clear();
	mVertexTriangles.resize(mVertices.size());

	int i=0;
	vector<Triangle>::iterator ti;
	for (ti=mTriangles.begin(); ti!= mTriangles.end(); ++ti) {
		mVertexTriangles[ti->vi1].insert(i);
		mVertexTriangles[ti->vi2].insert(i);
		mVertexTriangles[ti->vi3].insert(i++);
	}
}

void Model::updateTriangleData()
{
    vector<Triangle>::iterator ti;
    for (ti=mTriangles.begin(); ti!= mTriangles.end(); ++ti) 
        ti->updateData();
}

void Model::loadTrianglesFromOBJ(string filename, vector<Point> &vertices, vector<Triangle> &triangles, bool ccw, bool vt)
{
    Point v;
    int f1, f2, f3;
    char line[128], trash[16];
    FILE *objfile;

    if (!(objfile = fopen(filename.c_str(), "rt"))) return;

    while (fgets(line, 128, objfile)) {
        switch (line[0]) {
        case 'v':
            sscanf(&line[1],"%f %f %f", &v.x, &v.y, &v.z);
            vertices.push_back(v);
            break;
         case 'f':
            if (vt) sscanf(&line[1],"%d%s%d%s%d%s", &f1, trash, &f2, trash, &f3, trash);
            else sscanf(&line[1],"%d%d%d", &f1, &f2, &f3);
            if (ccw) triangles.push_back(Triangle(&vertices, --f1, --f3, --f2));
            else     triangles.push_back(Triangle(&vertices, --f1, --f2, --f3));
            break;
         default:
           continue;
        };
    }

    fclose(objfile);
}

void Model::findCollisions(const Model &m1, const Model &m2, vector<Point> &vertices, vector<Triangle> &triangles, bool both)
{
	//TODO Eliminate vertex repetition
	
	vector<Triangle> const &m1t = m1.mTriangles;
	vector<Triangle> const &m2t = m2.mTriangles;
	
	int mti1, mti2, count=0;
	bool mt1Collided;			// Flag indicating the collision of a triangle in the outer loop
    vector<char> mt2Collided;	// Keep track of allready collided triangles
	if (both) 
		mt2Collided.resize(m2t.size(), 0);

	vector<Triangle>::const_iterator mt1, mt2;
	for (mti1=0; mti1<m1t.size(); ++mti1) {
		mt1Collided=false;
		for (mti2=0; mti2<m2t.size(); ++mti2) {
			if (mt1Collided && mt2Collided[mti2]) continue;
			if (!Triangle::intersects(m1t[mti1], m2t[mti2])) continue;
			
			/* Add the colliding triangle of the first model. */
			if (!mt1Collided) { 
				vertices.push_back(m1t[mti1].v1());
				vertices.push_back(m1t[mti1].v2());
				vertices.push_back(m1t[mti1].v3());
				triangles.push_back(Triangle(&vertices, 3*count, 3*count+1, 3*count+2));
				count++;
				mt1Collided=true;
			}
			
			/* Add the colliding triangle of the second model if this is requested, otherwise break. */
			if (!both)
				break;
			else if (!mt2Collided[mti2]) {
				vertices.push_back(m2t[mti2].v1());
				vertices.push_back(m2t[mti2].v2());
				vertices.push_back(m2t[mti2].v3());
				triangles.push_back(Triangle(&vertices, 3*count, 3*count+1, 3*count+2));
				count++;
				mt2Collided[mti2]=true;
			}
			
		}
	}
}

/* Editing */
void Model::setSize(float size)
{
    float s = size / (max(mBox.max.x-mBox.min.x,
                          max(mBox.max.y-mBox.min.y,
                              mBox.max.z-mBox.min.z)));

    vector<Point>::iterator vi;
	for (vi=mVertices.begin(); vi!= mVertices.end(); ++vi)
        vi->scale(s);
    
	mBox.scale(s);
	updateTriangleData();
}

void Model::alingCornerToOrigin()
{
    vector<Point>::iterator vi;
    for (vi=mVertices.begin(); vi!= mVertices.end(); ++vi)
        vi->sub(mBox.min);

    mBox.sub(Point(mBox.min));
	updateTriangleData();
}

void Model::alingCenterToOrigin()
{
	Point c2(mBox.max);
	Point c1(mBox.min);
	c2.sub(c1);
	c2.scale(0.5);
	c1.add(c2);
 
    vector<Point>::iterator vi;   
	for (vi=mVertices.begin(); vi!= mVertices.end(); ++vi)
        vi->sub(c1);

    mBox.sub(c1);
	updateTriangleData();
}

void Model::reduce(int LoD)
{
 clock_t t = clock();
	set<int> procList;		// List with triangles to process
	set<int>::iterator pli;	// Iterator to procList
	int ti, tx;				// Indices of triangles to delete
	
	/* Populate triangle list with all the triangles */
	pli = procList.begin();
	for (ti=0; ti < mTriangles.size(); ++ti) 
		procList.insert(pli, ti++);
		
	/* Do the proccessing */
	for (pli = procList.begin(); pli != procList.end(); ++pli) {
		ti = *pli;

		if (mTriangles[ti].deleted)
			continue;

		/*1. Pick two vertices that will form the collapsing edge */
		int vk = mTriangles[ti].vi1;				// Vertex we keep of the collapsing edge
		int vx = mTriangles[ti].vi2;				// Vertex we discard of the collapsing edge
		set<int> &vkList = mVertexTriangles[vk];	// Reference to vertex's Vk triangle list
		set<int> &vxList = mVertexTriangles[vx];	// Reference to vertex's Vx triangle list
		set<int>::iterator vkLi, vxLi;				// Iterators for vertex triangle lists

		/*2. Find the second triangle, apart ti, with edge [vk,vx]=tx */
		vxLi = vxList.begin();
		vkLi = vkList.begin();
		while (vxLi != vxList.end() && vkLi != vkList.end()) {
			if (*vxLi < *vkLi) ++vxLi;
			else if (*vxLi > *vkLi) ++vkLi;
			else { if (*vxLi == ti) { ++vxLi; ++vkLi; }
				   else { tx = *vxLi; break; }}
		}

		if (mTriangles[tx].deleted) 
			continue;

		/*3. Delete the triangles of the collapsing edge */
		mTriangles[ti].deleted = 1;
		mTriangles[tx].deleted = 1;
		
		/*5. Update the affected triangles' vertices */
		for (vxLi = vxList.begin(); vxLi != vxList.end(); ++vxLi) {
			if (!mTriangles[*vxLi].deleted) {
				if      (mTriangles[*vxLi].vi1==vx) mTriangles[*vxLi].vi1 = vk;
				else if (mTriangles[*vxLi].vi2==vx) mTriangles[*vxLi].vi2 = vk;
				else if (mTriangles[*vxLi].vi3==vx) mTriangles[*vxLi].vi3 = vk;
			}
		}

		/*6. Move the triangle list of the discarded vertex to the one we keeped */
		vkList.insert(vxList.begin(), vxList.end());
		vxList.clear();
		vkList.erase(ti);
		vkList.erase(tx);

		/* 7. Remove all the triangles of this area of the process list */
		procList.erase(tx);
		for (vkLi = vkList.begin(); vkLi != vkList.end(); ++vkLi) 
			procList.erase(*vkLi);

	}
	
	/* Clean up the data structures holding the model data */
	int from, to;
	for (from=0; from < mTriangles.size(); /*-*/) {
		if (mTriangles[from].deleted) {
			for (to=from; to+1<mTriangles.size() && mTriangles[to+1].deleted ; ++to);
			mTriangles.erase(mTriangles.begin()+from, mTriangles.begin()+to+1);
		} else ++from;
	}

	createTriangleLists();
	updateTriangleData();
 printf ("Model reduction took:\t%4.2f sec | %d triangles.\n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}

/* Drawing */
void Model::drawTriangles(bool wire)
{
	glPolygonMode(GL_FRONT_AND_BACK, wire? GL_LINE: GL_FILL);
	glBegin(GL_TRIANGLES);
    vector<Triangle>::const_iterator ti;
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti) {
		glVertex3fv(ti->v1().data);
		glVertex3fv(ti->v2().data);
		glVertex3fv(ti->v3().data);		
    }
    glEnd();
}

void Model::drawTriangleBoxes()
{
    vector<Triangle>::const_iterator ti;
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti)
		ti->getBox().draw();
}

void Model::drawNormals()
{
    glBegin(GL_LINES);
    vector<Triangle>::const_iterator ti;
	for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti) {
		glVertex3fv(ti->getCenter().data);
		glVertex3fv(ti->getNormal2().data);
    }
    
    glEnd();
    return;
}

void Model::drawAABB()
{
	mBox.draw();
}

void Model::draw(int x)
{
    if (x & (1<<0)) drawTriangles(0);
	
	glColor3ub(0,0,0);
	if (x & (1<<1)) drawTriangles(1);
	
	glColor3ub(0xFF,0,0);
	if (x & (1<<2)) drawNormals();
	
	if (x & (1<<3)) drawAABB();
	
	if (x & (1<<4)) drawTriangleBoxes();
}
