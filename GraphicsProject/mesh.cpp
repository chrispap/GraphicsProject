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
    mRot(0,0,0),
    mPos(0,0,0),
    mAABB((2<<BVL)-1),
    mAABBTriangles((2<<BVL)-1)
{
    clock_t t = clock();
    loadTrianglesFromOBJ(filename, mVertices, mTriangles, ccw, vt);
    createTriangleLists();
    updateTriangleData();
    createBoundingBoxHierarchy();
    centerAlign();
    calculateVolume();
    printf ("Mesh loading took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}

Mesh::Mesh(const Mesh &m1, const Mesh &m2, bool both):
    mRot(0,0,0),
    mPos(0,0,0),
    mAABB((2<<BVL)-1),
    mAABBTriangles((2<<BVL)-1)
{
    clock_t t = clock();
    findCollisions(m1, m2, mVertices, mTriangles, both);
    createTriangleLists();
    updateTriangleData();
    createBoundingBoxHierarchy();
    printf ("Mesh collision took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}

Mesh::Mesh(const Mesh &original):
    mVertices (original.mVertices),
    mTriangles (original.mTriangles),
    mVertexTriangles (original.mVertexTriangles),
    mAABBTriangles (original.mAABBTriangles),
    mAABB (original.mAABB),
    mRot (original.mRot),
    mPos (original.mPos)
{
    vector<Triangle>::iterator ti;
    for (ti=mTriangles.begin(); ti!= mTriangles.end(); ++ti)
        ti->vecList = &mVertices;
}

Mesh::~Mesh()
{

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

void Mesh::createBoundingBoxHierarchy()
{
    /* Big Outer Box */
    Point min,max;
    min.x = min.y = min.z = FLT_MAX;
    max.x = max.y = max.z = FLT_MIN;
    vector<Point>::const_iterator vi;
    for (vi=mVertices.begin(); vi!=mVertices.end(); ++vi) {
        if (vi->x > max.x) max.x = vi->x;
        else if (vi->x < min.x) min.x = vi->x;
        if (vi->y > max.y) max.y = vi->y;
        else if (vi->y < min.y) min.y = vi->y;
        if (vi->z > max.z) max.z = vi->z;
        else if (vi->z < min.z) min.z = vi->z;
    }
    mAABB[0] = Box(min, max);

    /* Levels of Hierrarchy */

    /*Construct a triangle list with all the triangles */
    for (int ti=0; ti < mTriangles.size(); ++ti) mAABBTriangles[0].push_back(ti);

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
            Point min1,max1,min2,max2;
            min1.x = min1.y = min1.z = FLT_MAX;
            max1.x = max1.y = max1.z = FLT_MIN;
            min2.x = min2.y = min2.z = FLT_MAX;
            max2.x = max2.y = max2.z = FLT_MIN;

            list<int>::const_iterator bvi;
            for (bvi=mAABBTriangles[parent].begin(); bvi!=mAABBTriangles[parent].end(); ++bvi) {
                Triangle &t = mTriangles[*bvi];
                bool left = Geom::intersects(box1, t.getBox());

                mAABBTriangles[left?ch1:ch2].push_back(*bvi);
                Point &bmin = left? min1: min2;
                Point &bmax = left? max1: max2;
                
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

            Box::saturate(box1, min1, max1);
            Box::saturate(box2, min2, max2);
            mAABB[ch1] = Box(min1, max1);
            mAABB[ch2] = Box(min2, max2);
        }
    }
}

void Mesh::calculateVolume()
{
    const float dl = mAABB[0].getXSize()/VDIV;
    if (dl<0.00001) return;

    unsigned long int voxelCount=0, voxelTotal=0, intersectionsCount=0, xi=0;

    for (float x=mAABB[0].min.x+dl/2; x<mAABB[0].max.x; x+=dl) {
        printf("[%c] [%-2d%%]", "|/-\\"[xi++%4], (int)(100*((x-mAABB[0].min.x)/mAABB[0].getXSize())));fflush(stdout);
        for (float y=mAABB[0].min.y+dl/2; y<mAABB[0].max.y; y+=dl) {
            for (float z=mAABB[0].min.z+dl/2; z<mAABB[0].max.z; z+=dl)
            {
                /* Construct ray */
                Point ray0(x,y,z);
                Point rayFar(x*20,y*20,z*20);
                Line ray (ray0, rayFar);

                /* Count intersecting triangles */
                intersectionsCount=0;
                list<int>::const_iterator ti;
                for (int bi=(1<<BVL)-1; bi<(2<<BVL)-1; ++bi) {
                    if (!Geom::intersects(mAABB[bi], ray)) continue;
                    for (ti = mAABBTriangles[bi].begin(); ti!=mAABBTriangles[bi].end(); ++ti) {
                        Triangle &t = mTriangles[*ti];
                        if ((Geom::mkcode(ray.start, t.getBox()) & Geom::mkcode(ray.end, t.getBox()))) continue;
                        if ( Geom::intersects(t, ray)) ++intersectionsCount;
                    }
                }

                /* For odd number of triangles count this voxel to the total volume */
                if (intersectionsCount%2 == 1){
                    mVoxels.push_back( Box (Point(x-dl/2.2, y-dl/2.2, z-dl/2.2), Point(x+dl/2.2, y+dl/2.2, z+dl/2.2)));
                    ++voxelCount;
                }
                ++voxelTotal;
            }
        }
        printf ("\r");
    }
    printf("             \r");
    printf("Voxel total: %ld", voxelTotal);

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
void Mesh::setMaxSize(float size)
{
    float s = size / mAABB[0].getMaxSize();

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

void Mesh::cornerAlign()
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

void Mesh::centerAlign()
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
    glTranslatef(mPos.x, mPos.y, mPos.z);
    glRotatef(mRot.x, 1, 0, 0);
    glRotatef(mRot.y, 0, 1, 0);
    glRotatef(mRot.z, 0, 0, 1);
    if (x & VOXELS) drawVoxels(Colour(0,0xFF,0));
    if (x & SOLID) drawTriangles(col, false);
    if (x & WIRE) drawTriangles(Colour(0,0,0), true);
    if (x & NORMALS) drawNormals(Colour(0xFF,0,0));
    if (x & AABB) drawAABB(Colour(0xA5, 0x2A, 0x2A));
    if (x & TBOXES) drawTriangleBoxes(Colour(0xFF,0,0));
    glPopMatrix();
}
