#include "myVertex.h"
#include "myVector3D.h"
#include "myHalfedge.h"
#include "myFace.h"

myVertex::myVertex(void)
{
	point = NULL;
	originof = NULL;
	normal = new myVector3D(1.0,1.0,1.0);
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}


void myVertex::computeNormal()
{
    double nx = 0, ny = 0, nz = 0;
    myHalfedge *h = originof;
    do {
        if (h->adjacent_face && h->adjacent_face->normal)
        {
            nx += h->adjacent_face->normal->dX;
            ny += h->adjacent_face->normal->dY;
            nz += h->adjacent_face->normal->dZ;
        }
        if (!h->twin) break;
        h = h->twin->next;
    } while (h != originof);

    normal->dX = nx;
    normal->dY = ny;
    normal->dZ = nz;
    normal->normalize();
}

