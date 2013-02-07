/** @file mesh.cpp
 * Implementation of class Mesh
 */

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
    createBoundingVolHierarchy();
    centerAlign();
    createNormals();
    calculateVolume();
    printf ("Mesh loading took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
    for (int blevel=0; blevel<=BVL; blevel++)
        printf("Coverage Level %d: AABB %4.2f%%, Sphere %4.2f%% \n", blevel, 100*AABBCover[blevel], 100*sphereCover[blevel]);

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
    intersect(m1, m2, mVertices, mTriangles, both);
    if ( mTriangles.size())
        printf ("Mesh intersection took:\t%4.2f sec | %d triangles \n", ((float)clock()-t)/CLOCKS_PER_SEC, mTriangles.size());
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

void Mesh::createBoundingVolHierarchy()
{
    createBoundingBoxHierarchy();
    createBoundingSphereHierarchy();
}

void Mesh::createBoundingBoxHierarchy()
{
    /* The main box */
    mAABB[0] = Box(mVertices);

    /*Construct a triangle list with all the triangles */
    mAABBTriangles[0].clear();
    for (int ti=0; ti < mTriangles.size(); ++ti)
        mAABBTriangles[0].push_back(ti);

    /* Do BVL subdivisions */
    for (int bvlevel=0; bvlevel<BVL; ++bvlevel) {
        for (int div=0; div < (1<<bvlevel); ++div) {
            /* Find parent's, children's indices */
            int parent = (1<<bvlevel) -1+div;
            int chL = 2*parent+1;
            int chR = 2*parent+2;
            mAABBTriangles[chL].clear();
            mAABBTriangles[chR].clear();
            /* Find the largest of the 3 dimensions (xyz) and divide the box */
            Point &min = mAABB[parent].min;
            Point &max = mAABB[parent].max;
            int dim=0;
            Box boxL, boxR;
            if (mAABB[parent].getXSize() > mAABB[parent].getYSize())
                dim = (mAABB[parent].getXSize() > mAABB[parent].getZSize())? 0 : 1;
            else dim = (mAABB[parent].getYSize() > mAABB[parent].getZSize())? 1 : 2;
            float lim = (max.data[dim] + min.data[dim])/2;
            if (dim==0) {
                boxL = Box(min, Point(lim, max.y, max.z));
                boxR = Box(Point(lim, min.y, min.z), max);
            } else if (dim==1) {
                boxL = Box(min, Point(max.x, lim, max.z));
                boxR = Box(Point(min.x, lim, min.z), max);
            } else {
                boxL = Box(min, Point(max.x, max.y, lim));
                boxR = Box(Point(min.x, min.y, lim), max);
            }
            /* Find the triangles that belong to each subdivision*/
            Point minL,maxL,minR,maxR;
            minL.x = minL.y = minL.z = FLT_MAX;
            maxL.x = maxL.y = maxL.z = FLT_MIN;
            minR.x = minR.y = minR.z = FLT_MAX;
            maxR.x = maxR.y = maxR.z = FLT_MIN;

            list<int>::const_iterator bvi;
            for (bvi=mAABBTriangles[parent].begin(); bvi!=mAABBTriangles[parent].end(); ++bvi) {
                Triangle &t = mTriangles[*bvi];
                /* Check both boxes */
                if (Geom::intersects(boxL, t.getBox())) {
                    mAABBTriangles[chL].push_back(*bvi);
                    for (int vi=0; vi<3; ++vi) {
                        Point &v = mVertices[t.v[vi]];
                        if      (v.x > maxL.x) maxL.x = v.x;
                        else if (v.x < minL.x) minL.x = v.x;
                        if      (v.y > maxL.y) maxL.y = v.y;
                        else if (v.y < minL.y) minL.y = v.y;
                        if      (v.z > maxL.z) maxL.z = v.z;
                        else if (v.z < minL.z) minL.z = v.z;
                    }
                }
                if (Geom::intersects(boxR, t.getBox())) {
                    mAABBTriangles[chR].push_back(*bvi);
                    for (int vi=0; vi<3; ++vi) {
                        Point &v = mVertices[t.v[vi]];
                        if      (v.x > maxR.x) maxR.x = v.x;
                        else if (v.x < minR.x) minR.x = v.x;
                        if      (v.y > maxR.y) maxR.y = v.y;
                        else if (v.y < minR.y) minR.y = v.y;
                        if      (v.z > maxR.z) maxR.z = v.z;
                        else if (v.z < minR.z) minR.z = v.z;
                    }
                }
            }
            mAABB[chL] = Box(minL, maxL).forceMax(boxL);
            mAABB[chR] = Box(minR, maxR).forceMax(boxR);
        }
    }
}

void Mesh::createBoundingSphereHierarchy()
{
    set<int> setL, setR;
    mSphere[0] = Sphere(mVertices, setL);

    /*Construct a triangle list with all the triangles */
    mSphereTriangles[0].clear();
    for (int ti=0; ti < mTriangles.size(); ++ti)
        mSphereTriangles[0].push_back(ti);

    for (int bvlevel=0; bvlevel<BVL; ++bvlevel) {
        int dim=0;
        for (int div=0; div < (1<<bvlevel); ++div) {
            /* Find parent's, children's indices */
            int parent = (1<<bvlevel) -1+div;
            int chL = 2*parent+1;
            int chR = 2*parent+2;
            setL.clear();
            setR.clear();
            mSphereTriangles[chL].clear();
            mSphereTriangles[chR].clear();
            float lim = mSphere[parent].center.data[dim];

            list<int>::const_iterator bvi;
            for (bvi=mSphereTriangles[parent].begin(); bvi!=mSphereTriangles[parent].end(); ++bvi) {
                Triangle &t = mTriangles[*bvi];
                if (mVertices[t.vi1].data[dim] < lim || mVertices[t.vi2].data[dim] < lim || mVertices[t.vi3].data[dim] < lim) {
                    setL.insert(t.vi1);setL.insert(t.vi2);setL.insert(t.vi3);
                    mSphereTriangles[chL].push_back(*bvi);
                }
                else {
                    setR.insert(t.vi1);setR.insert(t.vi2);setR.insert(t.vi3);
                    mSphereTriangles[chR].push_back(*bvi);
                }
            }

            if (setL.size()) mSphere[chL] = Sphere(mVertices, setL);
            else mSphere[chL] = Sphere();
            if (setR.size()) mSphere[chR] = Sphere(mVertices, setR);
            else mSphere[chR] = Sphere();
        }
        dim = (dim+1)%3;
    }
}

void Mesh::calculateVolume()
{
    const float dl = mAABB[0].getXSize()/VDIV;
    if (dl<0.00001) return;

    unsigned long int voxelInside=0, voxelTotal=0, intersectionsCount=0, xi=0;

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
                set<int>alreadyIntersected;
                alreadyIntersected.clear();
                intersectionsCount=0;
                list<int>::const_iterator ti;
                int bvl = BVL;
                for (int bi=BVL_SIZE(bvl-1); bi<BVL_SIZE(bvl); ++bi) {
                    if (!Geom::intersects(mAABB[bi], ray)) continue;
                    for (ti = mAABBTriangles[bi].begin(); ti!=mAABBTriangles[bi].end(); ++ti) {
                        Triangle &t = mTriangles[*ti];
                        if ((Geom::mkcode(ray.start, t.getBox()) & Geom::mkcode(ray.end, t.getBox()))) continue;
                        if ( Geom::intersects(t, ray) && (alreadyIntersected.find(*ti)==alreadyIntersected.end())) {
                            ++intersectionsCount;
                            alreadyIntersected.insert(*ti);
                        }
                    }
                }

                /* For odd number of triangles count this voxel to the total volume */
                if (intersectionsCount%2 == 1){
                    mVoxels.push_back( Box (Point(x-dl/2.2, y-dl/2.2, z-dl/2.2), Point(x+dl/2.2, y+dl/2.2, z+dl/2.2)));
                    ++voxelInside;
                }
                ++voxelTotal;
            }
        }
        printf ("\r");
    }
    printf("             \r");

    /* Calculate the coverage for every level */
    float objVol = (mAABB[0].getVolume()*voxelInside)/voxelTotal;
    float bVol, sVol;

    for (int bvlevel=0; bvlevel<=BVL; ++bvlevel) {
        bVol=sVol=0;
        for (int bi=BVL_SIZE(bvlevel-1); bi< BVL_SIZE(bvlevel); ++bi) {
            bVol += mAABB[bi].getVolume();
            sVol += mSphere[bi].getVolume();
            for (int bii=bi+1; bii< BVL_SIZE(bvlevel); ++bii)
                sVol -= Geom::intersectionVolume(mSphere[bi], mSphere[bii]);
        }

        AABBCover[bvlevel] = objVol/bVol;
        sphereCover[bvlevel] = objVol/sVol;
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

void Mesh::intersect( Mesh &m1,  Mesh &m2, vector<Point> &vertices, vector<Triangle> &triangles, bool both)
{
    //TODO: Eliminate vertex repetition

    /* Trivial check */
    Box bbox1 = Box(m1.getBox()).add(m1.mPos);
    Box bbox2 = Box(m2.getBox()).add(m2.mPos);
    if (!Geom::intersects (bbox1, bbox2)) return;

    vector<Triangle> const &mt1 = m1.mTriangles;    // Just for a shorter name
    vector<Triangle> const &mt2 = m2.mTriangles;    // Just for a shorter name
    int bi1, bi2;                                   // Indices to models' bounding boxes
    list<int>::const_iterator mti1, mti2;           // Iterators for trianges of boxes'
    vector<bool> mtCol1, mtCol2;                    // Flags indicating that a triangle has already collided
    unsigned int count=0;                           // Count intersecting triangles

    /* Bring objects to theis actual positions */
    m1.hardTranslate(m1.mPos);
    m2.hardTranslate(m2.mPos);

    mtCol1.resize(mt1.size(), 0);
    if (both) mtCol2.resize(mt2.size(), 0);

    for (bi1=BVL_SIZE(BVL-1); bi1<BVL_SIZE(BVL); ++bi1) {
        for (bi2=BVL_SIZE(BVL-1); bi2<BVL_SIZE(BVL); ++bi2) {
            if (Geom::intersects(m1.mAABB[bi1], m2.mAABB[bi2])) {
                for (mti1 = m1.mAABBTriangles[bi1].begin(); mti1!=m1.mAABBTriangles[bi1].end(); ++mti1) {
                    if (Geom::intersects(mt1[*mti1].box, m2.mAABB[bi2])) {
                        for (mti2 = m2.mAABBTriangles[bi2].begin(); mti2!=m2.mAABBTriangles[bi2].end(); ++mti2) {
                            if ( Geom::intersects(mt1[*mti1], mt2[*mti2]) ) {
                                /* Add the colliding triangle of the first model. */
                                if (!mtCol1[*mti1]) {
                                    vertices.push_back(mt1[*mti1].v1());
                                    vertices.push_back(mt1[*mti1].v2());
                                    vertices.push_back(mt1[*mti1].v3());
                                    triangles.push_back(Triangle(&vertices, 3*count, 3*count+1, 3*count+2));
                                    mtCol1[*mti1]=true;
                                    ++count;
                                }
                                /* Add the colliding triangle of the second model. (Only if both is set) */
                                if (both && !mtCol2[*mti2]) {
                                    vertices.push_back(mt2[*mti2].v1());
                                    vertices.push_back(mt2[*mti2].v2());
                                    vertices.push_back(mt2[*mti2].v3());
                                    triangles.push_back(Triangle(&vertices, 3*count, 3*count+1, 3*count+2));
                                    mtCol2[*mti2]=true;
                                    ++count;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* Send models back to their origin */
    Point restore = Point(m1.mPos).scale(-1);
    m1.hardTranslate(restore);
    restore = Point(m2.mPos).scale(-1);;
    m2.hardTranslate(restore);
}


/* Editing */
struct TriangleCost
{
    int index;
    float cost;

    static vector<Triangle>  * tVec;
    static vector<Point>     * nVec;
    static vector<set<int> > * sVec;

    TriangleCost (int _index, bool calcCost=false) :
        index(_index)
    {
        if (calcCost)
            calculateCost();
    }

    TriangleCost (const TriangleCost &copyfrom)
    {
        index = copyfrom.index;
        cost = copyfrom.cost;
    }

    void calculateCost()
    {
        float sum;                  // sum the dot products
        Point n1,n2;                // Temp for normals
        set<int>::iterator tli;     // iterator for vertex's triangles

        vector<Triangle> &trian = *tVec;
        vector<set<int> > &vtl = *sVec;

        tli = vtl[trian[index].vi1].begin();
        n2  = trian[*tli].getNormal();
        ++tli;
        sum = 0;

        while (tli != vtl[trian[index].vi1].end()) {
            n1 = n2;
            n2 = trian[*tli].getNormal();
            sum += Geom::dotprod(n1, n2);
            ++tli;
        }

        cost = sum / (vtl[trian[index].vi1].size()-1);
    }

    bool operator<(TriangleCost rhs) { return cost < rhs.cost; }

    bool operator== (const TriangleCost &t) { return (index == t.index); }

    bool operator!= (const TriangleCost &t) { return !(*this == t); }

};

vector<Triangle>  * TriangleCost::tVec;
vector<Point>     * TriangleCost::nVec;
vector<set<int> > * TriangleCost::sVec;

void Mesh::simplify(int percent)
{
    /* Set these pointers */
    TriangleCost::tVec = &mTriangles;
    TriangleCost::nVec = &mVertexNormals;
    TriangleCost::sVec = &mVertexTriangles;

    clock_t t = clock();
    list<TriangleCost> procList;            // List of candidate triangles for collapse
    list<TriangleCost>::iterator pli;       // Iterator for the list above
    int ti, tx;                             // Indices of current triangles proccessed

    /* Populate triangle list with all the triangles and sort it */
    pli = procList.begin();
    for (ti=0; ti < mTriangles.size(); ++ti)
        pli = procList.insert(pli, TriangleCost(ti, true));

    procList.sort();

    int desiredRemovals = mTriangles.size()*(100-percent)/100;
    int removals = 0;

    /* Do the proccessing */
    while (procList.size() > 10 && removals < desiredRemovals) {

        /*0. Pick the next triangle for removal */
        ti = procList.begin()->index;
        if (mTriangles[ti].deleted){
            procList.erase(procList.begin());
            continue;
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

        if (tx==-1 || mTriangles[tx].deleted) {
            procList.erase(procList.begin());
            continue;
        }

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

        /* Place the new vertex in the middle of the collapsed edge */
        mVertices[vk] = Point(mVertices[vk]).add(mVertices[vx]).scale(0.5);

        /*6. Move the triangle list of the discarded vertex to the one we keeped */
        vkList.insert(vxList.begin(), vxList.end());
        vxList.clear();
        vkList.erase(ti);
        vkList.erase(tx);

        /* 7. Remove all the triangles of this area of the process list */
        procList.erase(procList.begin()); // Faster way for: procList.remove(ti);
        procList.remove(tx);
        for (vkLi = vkList.begin(); vkLi != vkList.end(); ++vkLi) {
            procList.remove(*vkLi);
        }

        //procList.sort(); // Won't improve quality, so dont do it.

        removals += 2;
    }

    /* Clean up the data structures holding the model data */
    int from, to;
    for (from=0; from < mTriangles.size();   ) {
        if (mTriangles[from].deleted) {
            for (to=from; to+1<mTriangles.size() && mTriangles[to+1].deleted ; ++to);
            mTriangles.erase(mTriangles.begin()+from, mTriangles.begin()+to+1);
        } else ++from;
    }

    createTriangleLists();
    updateTriangleData();
    createNormals();
    createBoundingVolHierarchy();
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

    for (int bi=0; bi<BVL_SIZE(BVL); ++bi){
        mAABB[bi].scale(s);
        mSphere[bi].scale(s);
    }

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
    for (int bi=0; bi<BVL_SIZE(BVL); ++bi) {
        mAABB[bi].sub(dl);
        mSphere[bi].sub(dl);
    }

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

    for (int bi=0; bi<BVL_SIZE(BVL); ++bi){
        mAABB[bi].sub(c1);
        mSphere[bi].sub(c1);
    }

    updateTriangleData();
}


/* Drawing */
void Mesh::drawVoxels(Colour col)
{
    vector<Box>::const_iterator pi;
    for(pi=mVoxels.begin(); pi!=mVoxels.end(); ++pi) {
        pi->draw(col, 0x30);
    }

}

void Mesh::drawTriangles(Colour col, bool wire)
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

void Mesh::drawTriangleBoxes(Colour col)
{
    vector<Triangle>::const_iterator ti;
    for(ti=mTriangles.begin(); ti!=mTriangles.end(); ++ti)
        ti->getBox().draw(col, 0);
}

void Mesh::drawNormals(Colour col)
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

void Mesh::drawSphere(Colour col, bool hier)
{
    /* Draw only the main box and the
     * leaves of the tree (last level of hierarchy */
    if (!hier) {
        mSphere[0].draw(col, 0);
    }
    else {
        unsigned char A=0;
        for (int bi=BVL_SIZE(BVL-1); bi<BVL_SIZE(BVL); ++bi) {
            mSphere[bi].draw(col, A);
        }
    }
}

void Mesh::drawAABB(Colour col, bool hier)
{
    /* Draw only the main box and the
     * leaves of the tree (last level of hierarchy */
    if (!hier) {
        mAABB[0].draw(col, 0);
    }
    else {
        for (int bi=BVL_SIZE(BVL-1); bi<BVL_SIZE(BVL); ++bi) {
            mAABB[bi].draw(col, 0);
            mAABB[bi].draw(col, 0x50);
        }
    }
}

void Mesh::draw(Colour col, int x)
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
    if (x & AABB) drawAABB(Colour(0xA5, 0x2A, 0x2A), x&HIER);
    if (x & SPHERE) drawSphere(Colour(0xA5, 0x2A, 0x2A), x&HIER);
    if (x & TBOXES) drawTriangleBoxes(Colour(0xFF,0,0));
    glPopMatrix();
}
