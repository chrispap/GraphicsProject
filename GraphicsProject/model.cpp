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
    loadTrianglesFromOBJ(filename, mVertice, mTriangles, ccw, vt);
	createTriangleLists();
	createBoundingBox();
	updateTriangleData();
}

Model::Model(const Model &m1, const Model &m2, bool both)
{
	clock_t t = clock();
	findCollisions(m1, m2, mVertice, mTriangles, both);
	printf ("Collision detection took: %4.2f sec.\n", ((float)clock()-t)/CLOCKS_PER_SEC);
	
	createTriangleLists();
	createBoundingBox();
	updateTriangleData();
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
    for (vi=mVertice.begin(); vi!=mVertice.end(); ++vi) {
        boxMax.x = vi->x > boxMax.x? vi->x: boxMax.x;
        boxMax.y = vi->y > boxMax.y? vi->y: boxMax.y;
        boxMax.z = vi->z > boxMax.z? vi->z: boxMax.z;
        boxMin.x = vi->x < boxMin.x? vi->x: boxMin.x;
        boxMin.y = vi->y < boxMin.y? vi->y: boxMin.y;
        boxMin.z = vi->z < boxMin.z? vi->z: boxMin.z;
    }

	mBox = Box(boxMin, boxMax);
}

void Model::createTriangleLists()
{
	mVertexTriangles.clear();
	mVertexTriangles.resize(mVertice.size());

	int i=0;
	vector<Triangle>::iterator ti;
	for (ti=mTriangles.begin(); ti!= mTriangles.end(); ++ti) {
		if (ti->deleted) continue;
		mVertexTriangles[ti->vi1].insert(i);
		mVertexTriangles[ti->vi2].insert(i);
		mVertexTriangles[ti->vi3].insert(i++);
	}
}

void Model::updateTriangleData()
{
    vector<Triangle>::iterator ti;
    for (ti=mTriangles.begin(); ti!= mTriangles.end(); ++ti) {
		if (ti->deleted) continue;
        ti->updateData();
    }
}

void Model::loadTrianglesFromOBJ(string filename, vector<Point> &mVertice, vector<Triangle> &mTriangles, bool ccw, bool vt)
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
            mVertice.push_back(v);
            break;
         case 'f':
            if (vt) sscanf(&line[1],"%d%s%d%s%d%s", &f1, trash, &f2, trash, &f3, trash);
            else sscanf(&line[1],"%d%d%d", &f1, &f2, &f3);
            if (ccw) mTriangles.push_back(Triangle(&mVertice, --f1, --f3, --f2));
            else     mTriangles.push_back(Triangle(&mVertice, --f1, --f2, --f3));
            break;
         default:
           continue;
        };
    }

    fclose(objfile);
}

void Model::findCollisions(const Model &m1, const Model &m2, vector<Point> &mVertice, vector<Triangle> &mTriangles, bool both)
{
	//TODO Eliminate vertex repetition

	int mti1, mti2, count=0;
	bool mt1Collided;
    vector<char> mt2Collided;
	mt2Collided.resize(m2.mTriangles.size(), 0);

	vector<Triangle> const &m1t = m1.mTriangles;
	vector<Triangle> const &m2t = m2.mTriangles;
	vector<Triangle>::const_iterator mt1, mt2;
	for (mti1=0; mti1<m1t.size(); ++mti1) {
		mt1Collided=false;
		for (mti2=0; mti2<m2t.size(); ++mti2) {
			if (mt1Collided && mt2Collided[mti2]) continue;
			if (!Triangle::intersects(m1t[mti1], m2t[mti2])) continue;
			
			if (!mt1Collided) {
				mVertice.push_back(m1t[mti1].v1());
				mVertice.push_back(m1t[mti1].v2());
				mVertice.push_back(m1t[mti1].v3());
				mTriangles.push_back(Triangle(&mVertice, 3*count, 3*count+1, 3*count+2));
				count++;
				mt1Collided=true;
			}
			if (both) {
				if (!mt2Collided[mti2]){
					mVertice.push_back(m2t[mti2].v1());
					mVertice.push_back(m2t[mti2].v2());
					mVertice.push_back(m2t[mti2].v3());
					mTriangles.push_back(Triangle(&mVertice, 3*count, 3*count+1, 3*count+2));
					count++;
					mt2Collided[mti2]=true;
				}
			} else {
				mt2Collided[mti2]=true;
				break;
			}
			
		}
	}
	cout << "Intersections: " << count << endl;
}

/* Editing */
void Model::setSize(float size)
{
    float s = size / (max(mBox.max.x-mBox.min.x,
                          max(mBox.max.y-mBox.min.y,
                              mBox.max.z-mBox.min.z)));

    vector<Point>::iterator vi;
	for (vi=mVertice.begin(); vi!= mVertice.end(); ++vi)
        vi->scale(s);
    
	mBox.scale(s);
	updateTriangleData();
}

void Model::alingCornerToOrigin()
{
    vector<Point>::iterator vi;
    for (vi=mVertice.begin(); vi!= mVertice.end(); ++vi)
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
	for (vi=mVertice.begin(); vi!= mVertice.end(); ++vi)
        vi->sub(c1);

    mBox.sub(c1);
	updateTriangleData();
}

void Model::reduce(int t)
{
	set<int> procList;		// List with mTriangles to process
	set<int>::iterator pli;	// Iterator to procList
	int ti, tx;				// Indices of mTriangles to delete
	int vk, vx;				// Vertices of the collapsing edge
	
	/* Initialize triangle list with all the triangles */
	pli = procList.begin();
	for (ti=0; ti < mTriangles.size(); ++ti) 
		procList.insert(pli, ti++);
		
	/* Do the proccessing */
	for (pli = procList.begin(); pli != procList.end(); ++pli) {
		ti = *pli;
		if (mTriangles[ti].deleted) { cout << "d1 "; continue; }

		/*2. Pick two vertices that will form the collapsing edge */
		vk = mTriangles[ti].vi1;
		vx = mTriangles[ti].vi2;
		set<int> &vkList = mVertexTriangles[vk];	// Reference to vertex's Vk triangle list
		set<int> &vxList = mVertexTriangles[vx];	// Reference to vertex's Vx triangle list
		set<int>::iterator vkLi, vxLi;				// Iterators for vertex triangle lists

		/*3. Find the second triangle, apart ti, with edge [vk,vx]=tx */
		vxLi = vxList.begin();
		vkLi = vkList.begin();
		while (vxLi != vxList.end() && vkLi != vkList.end()) {
			if (*vxLi < *vkLi) ++vxLi;
			else if (*vxLi > *vkLi) ++vkLi;
			else { if (*vxLi == ti) { ++vxLi; ++vkLi; }
				   else { tx = *vxLi; break; }}
		}

		if (mTriangles[tx].deleted) { cout << "d2 "; continue; }

		/*4. Delete the triangles of the collapsing edge */
		mTriangles[ti].deleted = 1;
		mTriangles[tx].deleted = 1;
		
		/*5. Update the affected mTriangles | Remove the affected from trProcList */
		for (vxLi = vxList.begin(); vxLi != vxList.end(); ++vxLi) {
			if (!mTriangles[*vxLi].deleted) {
				if      (mTriangles[*vxLi].vi1==vx) mTriangles[*vxLi].vi1 = vk;
				else if (mTriangles[*vxLi].vi2==vx) mTriangles[*vxLi].vi2 = vk;
				else if (mTriangles[*vxLi].vi3==vx) mTriangles[*vxLi].vi3 = vk;
			}
		}

		vkList.insert(vxList.begin(), vxList.end());
		vxList.clear();
		vkList.erase(ti);
		vkList.erase(tx);
		procList.erase(tx);
		for (vkLi = vkList.begin(); vkLi != vkList.end(); ++vkLi) 
			procList.erase(*vkLi);

	}
	
	/* Clean up the data structures holding the model data */
	vector<set<int> >::iterator _vi;
	for (_vi=mVertexTriangles.begin(); _vi!= mVertexTriangles.end(); ++_vi)
		_vi->clear();
	
	int from, to;
	for (from=0; from < mTriangles.size(); ) {
		if (!mTriangles[from].deleted) ++from;
		else {
			for (to=from; to+1<mTriangles.size() && mTriangles[to+1].deleted ; ++to);
			mTriangles.erase(mTriangles.begin()+from, mTriangles.begin()+to+1);
		}
	}

	createTriangleLists();
	updateTriangleData();
}

/* Drawing */
void Model::drawTriangles(bool wire)
{
    glPushMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, wire? GL_LINE: GL_FILL);
    glBegin(GL_TRIANGLES);

    vector<Triangle>::const_iterator ti;
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti) {
		if (ti->deleted) continue;
		glVertex3fv(ti->v1().data);
		glVertex3fv(ti->v2().data);
		glVertex3fv(ti->v3().data);
    }

    glEnd();
    glPopMatrix();
    return;
}

void Model::drawNormals()
{
    glPushMatrix();
    glBegin(GL_LINES);
    vector<Triangle>::const_iterator ti;
	for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti) {
		if (ti->deleted) continue;
		glVertex3fv(ti->getCenter().data);
		glVertex3fv(ti->getNormal2().data);
    }
    
    glEnd();
    glPopMatrix();
    return;
}

void Model::draw(int x)
{
    glPushMatrix();
    if (x & (1<<0)) drawTriangles(0);
	glColor3ub(0,0,0);
	if (x & (1<<1)) drawTriangles(1);
	glColor3ub(0xFF,0,0);
	if (x & (1<<2)) drawNormals();
    glPopMatrix();
}
