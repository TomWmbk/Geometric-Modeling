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
    if (!originof) return;

    double nx = 0, ny = 0, nz = 0;
    const int maxSteps = 1000;
    int steps = 0;

    // Forward traversal: turn around vertex CCW via twin->next
    myHalfedge *h = originof;
    do {
        if (!h) break;
        if (h->adjacent_face && h->adjacent_face->normal)
        {
            nx += h->adjacent_face->normal->dX;
            ny += h->adjacent_face->normal->dY;
            nz += h->adjacent_face->normal->dZ;
        }
        if (!h->twin || !h->twin->next) break;
        h = h->twin->next;
        steps++;
    } while (h != originof && steps < maxSteps);

    // If we stopped early (open mesh boundary), traverse backwards too
    if (h != originof && steps < maxSteps)
    {
        h = originof;
        while (h && h->prev && h->prev->twin && steps < maxSteps)
        {
            h = h->prev->twin;
            if (h == originof) break;
            if (h->adjacent_face && h->adjacent_face->normal)
            {
                nx += h->adjacent_face->normal->dX;
                ny += h->adjacent_face->normal->dY;
                nz += h->adjacent_face->normal->dZ;
            }
        }
    }

    normal->dX = nx;
    normal->dY = ny;
    normal->dZ = nz;
    normal->normalize();
}

