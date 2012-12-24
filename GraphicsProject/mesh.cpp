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
Mesh::Mesh(string filename, bool ccw):
    mRot(0,0,0),
    mPos(0,0,0),
    mAABB(BVL_SIZE(BVL)),
    mAABBTriangles(BVL_SIZE(BVL)),
    mSphere(BVL_SIZE(BVL)),
    mSphereTriangles(BVL_SIZE(BVL))
{
    clock_t t = clock();
    loadObj(filename, mVertices, mTriangles, ccw);
    createTriangleLists();
    createBoundingBoxHierarchy();
    centerAlign();
    createNormals();
    calculateVolume();
    for (int blevel=0; blevel<=BVL; blevel++)
        printf("Mesh coverage level:\t%d: %4.2f%% \n", blevel, 100*coverage[blevel]);
    printf ("Mesh loading took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}

Mesh::Mesh( Mesh &m1,  Mesh &m2, bool both):
    mRot(0,0,0),
    mPos(0,0,0),
    mAABB(BVL_SIZE(BVL)),
    mAABBTriangles(BVL_SIZE(BVL)),
    mSphere(BVL_SIZE(BVL)),
    mSphereTriangles(BVL_SIZE(BVL))
{
    clock_t t = clock();
    findCollisions(m1, m2, mVertices, mTriangles, both);
    printf ("Mesh collision took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}

Mesh::Mesh(const Mesh &copyfrom):
    mVertices (copyfrom.mVertices),
    mTriangles (copyfrom.mTriangles),
    mVertexNormals (copyfrom.mVertexNormals),
    mVertexTriangles (copyfrom.mVertexTriangles),
    mAABBTriangles (copyfrom.mAABBTriangles),
    mSphere(copyfrom.mSphere),
    mSphereTriangles(copyfrom.mSphereTriangles),
    mAABB (copyfrom.mAABB),
    mRot (copyfrom.mRot),
    mPos (copyfrom.mPos)
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

void Mesh::createNormals()
{
    mVertexNormals.clear();
    mVertexNormals.resize(mVertices.size());
    Point normSum;
    for (int vi=0; vi< mVertices.size(); ++vi) {
        normSum.x=0; normSum.y=0; normSum.z=0;
        set<int>::const_iterator _ti;
        for (_ti=mVertexTriangles[vi].begin(); _ti!=mVertexTriangles[vi].end(); ++_ti)
            normSum.add(mTriangles[*_ti].getNormal());
        mVertexNormals[vi] = normSum.scale(1.0f/3);
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
        if      (vi->x > max.x) max.x = vi->x;
        else if (vi->x < min.x) min.x = vi->x;
        if      (vi->y > max.y) max.y = vi->y;
        else if (vi->y < min.y) min.y = vi->y;
        if      (vi->z > max.z) max.z = vi->z;
        else if (vi->z < min.z) min.z = vi->z;
    }
    mAABB[0] = Box(min, max);

    /* Levels of Hierrarchy */

    /*Construct a triangle list with all the triangles */
    mAABBTriangles[0].clear();
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
            mAABBTriangles[ch1].clear();
            mAABBTriangles[ch2].clear();
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
                bool right = Geom::intersects(box2, t.getBox());

                if (left) {
                    mAABBTriangles[ch1].push_back(*bvi);
                    for (int vi=0; vi<3; ++vi) {
                        Point &v = mVertices[t.v[vi]];
                        if      (v.x > max1.x) max1.x = v.x;
                        else if (v.x < min1.x) min1.x = v.x;
                        if      (v.y > max1.y) max1.y = v.y;
                        else if (v.y < min1.y) min1.y = v.y;
                        if      (v.z > max1.z) max1.z = v.z;
                        else if (v.z < min1.z) min1.z = v.z;
                    }
                }
                if (right) {
                    mAABBTriangles[ch2].push_back(*bvi);
                    for (int vi=0; vi<3; ++vi) {
                        Point &v = mVertices[t.v[vi]];
                        if      (v.x > max2.x) max2.x = v.x;
                        else if (v.x < min2.x) min2.x = v.x;
                        if      (v.y > max2.y) max2.y = v.y;
                        else if (v.y < min2.y) min2.y = v.y;
                        if      (v.z > max2.z) max2.z = v.z;
                        else if (v.z < min2.z) min2.z = v.z;
                    }
                }
            }

            mAABB[ch1] = Box(min1, max1).forceMax(box1);
            mAABB[ch2] = Box(min2, max2).forceMax(box2);
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
                for (int bi=BVL_SIZE(BVL-1); bi<BVL_SIZE(BVL); ++bi) {
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

    /* Calculate the coverage for every level */
    float cover = ((float)voxelCount)/voxelTotal;
    float fullVol = mAABB[0].getVolume();
    coverage[0] = cover;

    for (int bvlevel=1; bvlevel<=BVL; ++bvlevel) {
        float boundingVol=0;
        for (int bi=(1<<bvlevel) -1; bi< (2<<bvlevel) -1; ++bi)
            boundingVol += mAABB[bi].getVolume();
        coverage[bvlevel] = (cover*fullVol)/boundingVol;
    }
}

void Mesh::loadObj(string filename, vector<Point> &vertices, vector<Triangle> &triangles, bool ccw)
{
    Point v;
    int f1, f2, f3;
    char line[128];
    FILE *objfile;

    if (!(objfile = fopen(filename.c_str(), "rt"))) return;

    while (fgets(line, 128, objfile)) {
        switch (line[0]) {
        case 'v':
            sscanf(&line[1],"%f %f %f", &v.x, &v.y, &v.z);
            vertices.push_back(v);
            break;
        case 'f':
            sscanf(&line[1],"%d%d%d", &f1, &f2, &f3);
            if (ccw) triangles.push_back(Triangle(&vertices, --f1, --f3, --f2));
            else triangles.push_back(Triangle(&vertices, --f1, --f2, --f3));
            break;
        default:
            continue;
        };
    }

    fclose(objfile);
}

void Mesh::findCollisions( Mesh &m1,  Mesh &m2, vector<Point> &vertices, vector<Triangle> &triangles, bool both)
{
    Box bbox1 = Box(m1.getBox()).add(m1.mPos);
    Box bbox2 = Box(m2.getBox()).add(m2.mPos);

    if (Geom::intersects (bbox1, bbox2)) // trivial check
    {
        //TODO Eliminate vertex repetition

        m1.hardTranslate(m1.mPos);
        m2.hardTranslate(m2.mPos);

        vector<Triangle> const &m1t = m1.mTriangles;
        vector<Triangle> const &m2t = m2.mTriangles;

        unsigned int count=0;       // count intersecting triangles
        unsigned int m1ti;          // index to model's 1 triangles
        bool m1tCollided;           // Flag indicating the collision of a triangle in the outer loop
        vector<char> m2tCollided;   // Keep track of allready collided triangles
        if (both)                   // We need it only if we are looking for triangles from both models
            m2tCollided.resize(m2t.size(), 0);

        for (m1ti=0; m1ti<m1t.size(); ++m1ti) { // for all triangles of model 1
            m1tCollided=false;

            for (int bi=BVL_SIZE(BVL-1); bi<BVL_SIZE(BVL); ++bi) {  // for all bounding boxes of last level of model 2
                if (!Geom::intersects(m2.mAABB[bi], m1t[m1ti].box)) continue;

                list<int>::const_iterator m2ti;
                for (m2ti = m2.mAABBTriangles[bi].begin(); m2ti!=m2.mAABBTriangles[bi].end(); ++m2ti) {
                    if (m1tCollided && m2tCollided[*m2ti]) continue;
                    if (!Geom::intersects(m1t[m1ti], m2t[*m2ti])) continue;

                    /* Add the colliding triangle of the first model. */
                    if (!m1tCollided) {
                        vertices.push_back(m1t[m1ti].v1());
                        vertices.push_back(m1t[m1ti].v2());
                        vertices.push_back(m1t[m1ti].v3());
                        triangles.push_back(Triangle(&vertices, 3*count, 3*count+1, 3*count+2));
                        count++;
                        m1tCollided=true;
                    }
                    /* Add the colliding triangle of the second model if this is requested, otherwise break. */
                    if (!both)
                        break;
                    else if (!m2tCollided[*m2ti]) {
                        vertices.push_back(m2t[*m2ti].v1());
                        vertices.push_back(m2t[*m2ti].v2());
                        vertices.push_back(m2t[*m2ti].v3());
                        triangles.push_back(Triangle(&vertices, 3*count, 3*count+1, 3*count+2));
                        count++;
                        m2tCollided[*m2ti]=true;
                    }
                }
            }
        }

        Point restore = Point(m1.mPos).scale(-1);
        m1.hardTranslate(restore);
        restore = Point(m2.mPos).scale(-1);;
        m2.hardTranslate(restore);
    }
}


/** Editing */

/** 
 * Comparator in order to sort the triangle list
 * based on how parallel the normals in two vertices are 
 */ 
static vector<Triangle>* tVec;  // ptr to triangle list
static vector<Point>* nVec;     // ptr to vertex normal list

static bool NormalComparator (const int& l, const int& r)
{
    float nl = Geom::dotprod((*nVec)[(*tVec)[l].vi1], (*nVec)[(*tVec)[l].vi2]);
    float nr = Geom::dotprod((*nVec)[(*tVec)[r].vi1], (*nVec)[(*tVec)[r].vi2]);
    return nl > nr;
}

void Mesh::simplify(int percent)
{
    clock_t t = clock();
    list<int> procList;          // List of candidate triangles for collapse
    list<int>::iterator pli;     // Iterator for the list above
    int ti, tx;                 // Indices of current triangles proccessed

    /* Populate triangle list with all the triangles and sort it */
    pli = procList.begin();
    for (ti=0; ti < mTriangles.size(); ++ti)
        procList.insert(pli, ti);
    
    tVec = &mTriangles; nVec = &mVertexNormals;
    procList.sort(NormalComparator);

    int desiredRemovals = mTriangles.size()*(100-percent)/100;
    int removals = 0;

    /* Do the proccessing */
    for (pli = procList.begin(); removals < desiredRemovals && pli != procList.end(); ++pli) {
        ti = *pli;

        if (mTriangles[ti].deleted){
            procList.remove(ti); continue;
        }

        /*1. Pick two vertices that will form the collapsing edge */
        int vk = mTriangles[ti].vi1;                // Vertex we keep of the collapsing edge
        int vx = mTriangles[ti].vi2;                // Vertex we discard of the collapsing edge
        set<int> &vkList = mVertexTriangles[vk];    // Reference to vertex's Vk triangle list
        set<int> &vxList = mVertexTriangles[vx];    // Reference to vertex's Vx triangle list
        set<int>::iterator vkLi, vxLi;              // Iterators for vertex triangle lists

        /*2. Find the second triangle, apart ti, with edge [vk,vx]=tx */
        vxLi = vxList.begin();
        vkLi = vkList.begin();
        tx = -1;
        while (vxLi != vxList.end() && vkLi != vkList.end()) {
            if (*vxLi < *vkLi) ++vxLi;
            else if (*vxLi > *vkLi) ++vkLi;
            else { if (*vxLi == ti) { ++vxLi; ++vkLi; }
                else { tx = *vxLi; break; }}
        }

        if (tx==-1 || mTriangles[tx].deleted)
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

        /* Place the new vertex in the middle ob the collapsed edge */
        mVertices[vk] = Point(mVertices[vk]).add(mVertices[vx]).scale(0.5);

        /*6. Move the triangle list of the discarded vertex to the one we keeped */
        vkList.insert(vxList.begin(), vxList.end());
        vxList.clear();
        vkList.erase(ti);
        vkList.erase(tx);

        /* 7. Remove all the triangles of this area of the process list */
        procList.remove(tx);
        for (vkLi = vkList.begin(); vkLi != vkList.end(); ++vkLi)
            procList.remove(*vkLi);

        removals += 2;
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
    createNormals();
    createBoundingBoxHierarchy();
    printf ("Mesh reduction took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
}

void Mesh::hardTranslate(const Point &p)
{
    vector<Point>::iterator vi;
    for (vi=mVertices.begin(); vi!= mVertices.end(); ++vi)
        vi->add(p);

    for (int bi=0; bi<BVL_SIZE(BVL); ++bi)
        mAABB[bi].add(p);

    updateTriangleData();
}

void Mesh::setMaxSize(float size)
{
    float s = size / mAABB[0].getMaxSize();

    vector<Point>::iterator vi;
    for (vi=mVertices.begin(); vi!= mVertices.end(); ++vi)
        vi->scale(s);

    vector<Box>::iterator pi;
    for(pi=mVoxels.begin(); pi!=mVoxels.end(); ++pi)
        pi->scale(s);

    for (int bi=0; bi<BVL_SIZE(BVL); ++bi)
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
    for (int bi=0; bi<BVL_SIZE(BVL); ++bi)
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

    for (int bi=0; bi<BVL_SIZE(BVL); ++bi)
        mAABB[bi].sub(c1);

    updateTriangleData();
}


/** Drawing */
void Mesh::drawVoxels(Colour &col)
{
    vector<Box>::const_iterator pi;
    for(pi=mVoxels.begin(); pi!=mVoxels.end(); ++pi) {
        pi->draw(col, 0x30);
    }

}

void Mesh::drawTriangles(Colour &col, bool wire)
{
    glPolygonMode(GL_FRONT_AND_BACK, wire? GL_LINE: GL_FILL);
    glBegin(GL_TRIANGLES);
    glColor3ubv(col.data);
    bool normExist = mVertexNormals.size()>0;
    vector<Triangle>::const_iterator ti;
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti) {
        if (normExist) glNormal3fv(mVertexNormals[ti->vi1].data);
        glVertex3fv(ti->v1().data);
        if (normExist) glNormal3fv(mVertexNormals[ti->vi2].data);
        glVertex3fv(ti->v2().data);
        if (normExist) glNormal3fv(mVertexNormals[ti->vi3].data);
        glVertex3fv(ti->v3().data);
    }
    glEnd();
}

void Mesh::drawTriangleBoxes(Colour &col)
{
    vector<Triangle>::const_iterator ti;
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti)
        ti->getBox().draw(col, 0);
}

void Mesh::drawNormals(Colour &col)
{
    Point n;
    glBegin(GL_LINES);
    vector<Triangle>::const_iterator ti;
   
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti) {
        n = ti->getCenter();
        glColor3ubv(Colour(0x00,0,0).data);
        glVertex3fv(n.data);
        n.add(ti->getNormal());
        glColor3ubv(Colour(0xFF,0,0).data);
        glVertex3fv(n.data);
    }

    glEnd();
    return;
}

void Mesh::drawAABB(Colour &col, bool hier)
{
    /* Draw only the main box and the
     * leaves of the tree (last level of hierarchy */
    mAABB[0].draw(col, 0);

    if (hier) {
        for (int bi=BVL_SIZE(BVL-1); bi<BVL_SIZE(BVL); ++bi) {
            mAABB[bi].draw(col, 0);
            mAABB[bi].draw(col, 0x50);
        }
    }
}

void Mesh::draw(Colour &col, int x)
{
    glPushMatrix();
    glTranslatef(mPos.x, mPos.y, mPos.z);
    glRotatef(mRot.x, 1, 0, 0);
    glRotatef(mRot.y, 0, 1, 0);
    glRotatef(mRot.z, 0, 0, 1);
    if (x & VOXELS) drawVoxels(Colour(0,0xFF,0));
    if (x & SOLID) drawTriangles(col, false);
    if (x & WIRE) drawTriangles(Colour(0,0,0), true);
    if (x & NORMALS) drawNormals(col);
    if (x & AABB) drawAABB(Colour(0xA5, 0x2A, 0x2A), x&AABBH);
    if (x & TBOXES) drawTriangleBoxes(Colour(0xFF,0,0));
    glPopMatrix();
}
