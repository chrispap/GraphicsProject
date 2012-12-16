#include <stdio.h>
#include <ctime>
#include <cfloat>
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include "mesh.h"
#include "geom.h"

#ifndef QT_CORE_LIB
#ifdef __linux__
#include <GL/glut.h>
#else
#include "gl/glut.h"
#endif
#else
#include <GL/glu.h>
#endif

/** Constructrors */
Mesh::Mesh(string filename, bool ccw,  bool vt):
    localRot(0,0,0),
    localTranslation(0,0,0),
    mAABB((2<<BVL)-1),
    mAABBTriangles((2<<BVL)-1)
{
    clock_t t = clock();
    loadTrianglesFromOBJ(filename, mVertices, mTriangles, ccw, vt);
    createTriangleLists();
    updateTriangleData();
    createBoundingBox();
    alignLocalCenter();
    calculateVolume();
    printf ("Mesh loading took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}

Mesh::Mesh(const Mesh &m1, const Mesh &m2, bool both):
    localRot(0,0,0),
    localTranslation(0,0,0),
    mAABB((2<<BVL)-1),
    mAABBTriangles((2<<BVL)-1)
{
    clock_t t = clock();
    findCollisions(m1, m2, mVertices, mTriangles, both);
    createTriangleLists();
    updateTriangleData();
    createBoundingBox();
    printf ("Mesh collision took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}

Mesh::Mesh(const Mesh &original):
    mVertices (original.mVertices),
    mTriangles (original.mTriangles),
    mVertexTriangles (original.mVertexTriangles),
    mAABB (original.mAABB),
    mAABBTriangles (original.mAABBTriangles),
    localRot (original.localRot),
    localTranslation (original.localTranslation)
{
    vector<Triangle>::iterator ti;
    for (ti=mTriangles.begin(); ti!= mTriangles.end(); ++ti)
        ti->vecList = &mVertices;
}

Mesh::~Mesh()
{

}

void Mesh::createBoundingBox()
{
    /* Big Outer Box */
    Point boxMin,boxMax;
    boxMin.x = boxMin.y = boxMin.z = FLT_MAX;
    boxMax.x = boxMax.y = boxMax.z = FLT_MIN;

    vector<Point>::const_iterator vi;
    for (vi=mVertices.begin(); vi!=mVertices.end(); ++vi) {
        if (vi->x > boxMax.x) boxMax.x = vi->x;
        else if (vi->x < boxMin.x) boxMin.x = vi->x;
        if (vi->y > boxMax.y) boxMax.y = vi->y;
        else if (vi->y < boxMin.y) boxMin.y = vi->y;
        if (vi->z > boxMax.z) boxMax.z = vi->z;
        else if (vi->z < boxMin.z) boxMin.z = vi->z;
    }

    mAABB[0] = Box(boxMin, boxMax);

    /* Levels of Hierrarchy */
    for (int ti=0; ti < mTriangles.size(); ++ti)
        mAABBTriangles[0].push_back(ti);

    /* For each level of hierarchy,
     * for each node of this level,
     * split the node to half.
     */
    for (int bvlevel=0; bvlevel<BVL; ++bvlevel) {
        for (int div=0; div < (1<<bvlevel); ++div) {

            int parent = (1<<bvlevel) -1+div;
            int ch1 = 2*parent+1;
            int ch2 = 2*parent+2;
            Point &min = mAABB[parent].min;
            Point &max = mAABB[parent].max;
            float limX = (max.x + min.x)/2;
            Box box1 (min, Point(limX, max.y, max.z));
            Box box2 (Point(limX, min.y, min.z), max);
            Point boxMin1,boxMax1,boxMin2,boxMax2;
            boxMin1.x = boxMin1.y = boxMin1.z = FLT_MAX;
            boxMax1.x = boxMax1.y = boxMax1.z = FLT_MIN;
            boxMin2.x = boxMin2.y = boxMin2.z = FLT_MAX;
            boxMax2.x = boxMax2.y = boxMax2.z = FLT_MIN;

            list<int>::const_iterator bvi;
            for (bvi=mAABBTriangles[parent].begin(); bvi!=mAABBTriangles[parent].end(); ++bvi) {
                Triangle &t = mTriangles[*bvi];
                bool b1 = Geom::intersects(box1, t.getBox());
                mAABBTriangles[b1?ch1:ch2].push_back(*bvi);
                Point &bmin = b1? boxMin1: boxMin2;
                Point &bmax = b1? boxMax1: boxMax2;
                for (int vi=0; vi<3; ++vi) {
                    Point &v = mVertices[t.v[vi]];
                    if (v.x > bmax.x) bmax.x = v.x;
                    else if (v.x < bmin.x) bmin.x = v.x;
                    if (v.y > bmax.y) bmax.y = v.y;
                    else if (v.y < bmin.y) bmin.y = v.y;
                    if (v.z > bmax.z) bmax.z = v.z;
                    else if (v.z < bmin.z) bmin.z = v.z;
                }
            }

            if (boxMin1.x<box1.min.x) boxMin1.x = box1.min.x;
            if (boxMin1.y<box1.min.y) boxMin1.y = box1.min.y;
            if (boxMin1.z<box1.min.z) boxMin1.z = box1.min.z;

            if (boxMax1.x>box1.max.x) boxMax1.x = box1.max.x;
            if (boxMax1.y>box1.max.y) boxMax1.y = box1.max.y;
            if (boxMax1.z>box1.max.z) boxMax1.z = box1.max.z;

            if (boxMin2.x<box2.min.x) boxMin2.x = box2.min.x;
            if (boxMin2.y<box2.min.y) boxMin2.y = box2.min.y;
            if (boxMin2.z<box2.min.z) boxMin2.z = box2.min.z;

            if (boxMax2.x>box2.max.x) boxMax2.x = box2.max.x;
            if (boxMax2.y>box2.max.y) boxMax2.y = box2.max.y;
            if (boxMax2.z>box2.max.z) boxMax2.z = box2.max.z;

            mAABB[ch1] = Box(boxMin1, boxMax1);
            mAABB[ch2] = Box(boxMin2, boxMax2);
        }
    }
}

void Mesh::calculateVolume()
{
    const float dl = mAABB[0].getXSize()/VDIV;
    if (dl<0.01) return;

    unsigned long int voxelCount=0;
    unsigned long int voxelTotal=0;
    int xi=0;
    for (float x=mAABB[0].min.x+dl/2; x<mAABB[0].max.x; x+=dl) {
        printf("%c   %-2d%%", "|/-\\"[xi++%4], (int)(100*((x-mAABB[0].min.x)/mAABB[0].getXSize())));
        fflush(stdout);
        for (float y=mAABB[0].min.y+dl/2; y<mAABB[0].max.y; y+=dl) {
            for (float z=mAABB[0].min.z+dl/2; z<mAABB[0].max.z; z+=dl)
            {
                int intersectionsCount=0;
                Point ray0(x,y,z);
                Point rayFar(x*(x>0?200:-200),y*(y>0?200:-200),z*(z>0?200:-200));
                Line ray (ray0, rayFar);
                vector<Triangle>::const_iterator ti;
                for (ti = mTriangles.begin(); ti!=mTriangles.end(); ++ti) {
                    if ((Geom::mkcode(ray.start, ti->getBox()) & Geom::mkcode(ray.end, ti->getBox())) != 0) continue;
                    if ( Geom::intersects(*ti, ray)) ++intersectionsCount;
                }
                if (intersectionsCount%2 == 1){
                    mVoxels.push_back( Box (Point(x-dl/2.2, y-dl/2.2, z-dl/2.2), Point(x+dl/2.2, y+dl/2.2, z+dl/2.2)));
                     ++voxelCount;
                 }
                ++voxelTotal;
            }
        }
        printf ("\r");
    }
    printf("         \r");

    /* Calculate the coverage for every level */
    float cover = ((float)voxelCount)/voxelTotal;
    float fullVol = mAABB[0].getVolume();
    coverage[0] = cover;

    for (int bvlevel=1; bvlevel<=BVL; ++bvlevel) {
        float boundingVol=0;
        for (int bi=(1<<bvlevel) -1; bi< (2<<bvlevel) -1; ++bi)
		    boundingVol += mAABB[bi].getVolume();
        coverage[bvlevel] = cover*fullVol/boundingVol;
    }
}

void Mesh::createTriangleLists()
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

void Mesh::updateTriangleData()
{
    vector<Triangle>::iterator ti;
    for (ti=mTriangles.begin(); ti!= mTriangles.end(); ++ti)
        ti->update();
}

void Mesh::loadTrianglesFromOBJ(string filename, vector<Point> &vertices, vector<Triangle> &triangles, bool ccw, bool vt)
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

void Mesh::findCollisions(const Mesh &m1, const Mesh &m2, vector<Point> &vertices, vector<Triangle> &triangles, bool both)
{
    if (!Geom::intersects (m1.getBox(), m2.getBox())) return;

    //TODO Eliminate vertex repetition
    vector<Triangle> const &m1t = m1.mTriangles;
    vector<Triangle> const &m2t = m2.mTriangles;

    int mti1, mti2, count=0;
    bool mt1Collided;            // Flag indicating the collision of a triangle in the outer loop
    vector<char> mt2Collided;    // Keep track of allready collided triangles
    if (both)
        mt2Collided.resize(m2t.size(), 0);

    vector<Triangle>::const_iterator mt1, mt2;
    for (mti1=0; mti1<m1t.size(); ++mti1) {
        mt1Collided=false;
        for (mti2=0; mti2<m2t.size(); ++mti2) {
            if (mt1Collided && mt2Collided[mti2]) continue;
            if (!Geom::intersects(m1t[mti1], m2t[mti2])) continue;

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


/** Editing */
void Mesh::setSize(float size)
{
    float s = size / (max(mAABB[0].max.x-mAABB[0].min.x,
                      max(mAABB[0].max.y-mAABB[0].min.y,
                      mAABB[0].max.z-mAABB[0].min.z)));

    vector<Point>::iterator vi;
    for (vi=mVertices.begin(); vi!= mVertices.end(); ++vi)
        vi->scale(s);

    vector<Box>::iterator pi;
    for(pi=mVoxels.begin(); pi!=mVoxels.end(); ++pi)
        pi->scale(s);

    for (int bi=0; bi<(2<<BVL)-1; ++bi)
        mAABB[bi].scale(s);

    updateTriangleData();
}

void Mesh::translate(const Point &p)
{
    vector<Point>::iterator vi;
    for (vi=mVertices.begin(); vi!= mVertices.end(); ++vi)
        vi->add(p);

    for (int bi=0; bi<(2<<BVL)-1; ++bi)
        mAABB[bi].add(p);

    vector<Box>::iterator pi;
    for(pi=mVoxels.begin(); pi!=mVoxels.end(); ++pi)
        pi->add(p);

    updateTriangleData();
}

void Mesh::alignLocalCorner()
{
    vector<Point>::iterator vi;
    for (vi=mVertices.begin(); vi!= mVertices.end(); ++vi)
        vi->sub(mAABB[0].min);

    vector<Box>::iterator pi;
    for(pi=mVoxels.begin(); pi!=mVoxels.end(); ++pi)
        pi->sub(mAABB[0].min);

    Point dl(mAABB[0].min);
    for (int bi=0; bi<(2<<BVL)-1; ++bi)
        mAABB[bi].sub(dl);

    updateTriangleData();
}

void Mesh::alignLocalCenter()
{
    Point c2(mAABB[0].max);
    Point c1(mAABB[0].min);
    c2.sub(c1);
    c2.scale(0.5);
    c1.add(c2);

    vector<Point>::iterator vi;
    for (vi=mVertices.begin(); vi!= mVertices.end(); ++vi)
        vi->sub(c1);

    vector<Box>::iterator pi;
    for(pi=mVoxels.begin(); pi!=mVoxels.end(); ++pi)
        pi->sub(c1);

    for (int bi=0; bi<(2<<BVL)-1; ++bi)
        mAABB[bi].sub(c1);

    updateTriangleData();
}

void Mesh::reduce(int LoD)
{
    clock_t t = clock();
    set<int> procList;      // List with triangles to process
    set<int>::iterator pli; // Iterator to procList
    int ti, tx;             // Indices of triangles to delete

    /* Populate triangle list with all the triangles */
    pli = procList.begin();
    for (ti=0; ti < mTriangles.size(); ++ti)
        procList.insert(pli, ti);

    /* Do the proccessing */
    for (pli = procList.begin(); pli != procList.end(); ++pli) {
        ti = *pli;

        if (mTriangles[ti].deleted)
            continue;

        /*1. Pick two vertices that will form the collapsing edge */
        int vk = mTriangles[ti].vi1;                // Vertex we keep of the collapsing edge
        int vx = mTriangles[ti].vi2;                // Vertex we discard of the collapsing edge
        set<int> &vkList = mVertexTriangles[vk];    // Reference to vertex's Vk triangle list
        set<int> &vxList = mVertexTriangles[vx];    // Reference to vertex's Vx triangle list
        set<int>::iterator vkLi, vxLi;              // Iterators for vertex triangle lists

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
    printf ("Mesh reduction took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}


/** Drawing */
void Mesh::drawVoxels(const Colour &col)
{
    vector<Box>::const_iterator pi;
    for(pi=mVoxels.begin(); pi!=mVoxels.end(); ++pi) {
        pi->draw(col, 0x30);
    }

}

void Mesh::drawTriangles(const Colour &col, bool wire)
{
    glPolygonMode(GL_FRONT_AND_BACK, wire? GL_LINE: GL_FILL);
    glBegin(GL_TRIANGLES);
    glColor3ubv(col.data);
    vector<Triangle>::const_iterator ti;
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti) {
        glNormal3fv(ti->getNormal().data);
        glVertex3fv(ti->v1().data);
        glVertex3fv(ti->v2().data);
        glVertex3fv(ti->v3().data);
    }
    glEnd();
}

void Mesh::drawTriangleBoxes(const Colour &col)
{
    vector<Triangle>::const_iterator ti;
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti)
        ti->getBox().draw(col, 0);
}

void Mesh::drawNormals(const Colour &col)
{
    glBegin(GL_LINES);
    vector<Triangle>::const_iterator ti;
    glColor3ubv(col.data);
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti) {
        glVertex3fv(ti->getCenter().data);
        glVertex3fv(ti->getNormal2().data);
    }

    glEnd();
    return;
}

void Mesh::drawAABB(const Colour &col)
{
    /* Draw only the main box and the
     * leaves of the tree (last level of hierarchy */
    mAABB[0].draw(col, 0);
    for (int bi=(1<<BVL)-1; bi<(2<<BVL)-1; ++bi) {
        mAABB[bi].draw(col, 0);
        mAABB[bi].draw(col, 0x80);
    }
}

void Mesh::draw(const Colour &col, int x)
{
    glPushMatrix();
    glTranslatef(localTranslation.x, localTranslation.y, localTranslation.z);
    glRotatef(localRot.x, 1, 0, 0);
    glRotatef(localRot.y, 0, 1, 0);
    glRotatef(localRot.z, 0, 0, 1);
    if (x & VOXELS) drawVoxels(Colour(0,0xFF,0));
    if (x & SOLID) drawTriangles(col, false);
    if (x & WIRE) drawTriangles(Colour(0,0,0), true);
    if (x & NORMALS) drawNormals(Colour(0xFF,0,0));
    if (x & AABB) drawAABB(Colour(0xA5, 0x2A, 0x2A));
    if (x & TBOXES) drawTriangleBoxes(Colour(0xFF,0,0));
    glPopMatrix();
}
