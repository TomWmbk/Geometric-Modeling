#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myVector3D.h"

using namespace std;

myMesh::myMesh(void)
{
	/**** TODO ****/
}

myMesh::~myMesh(void)
{
	/**** TODO ****/
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++)
		if (vertices[i])
			delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++)
		if (halfedges[i])
			delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++)
		if (faces[i])
			delete faces[i];

	vector<myVertex *> empty_vertices;
	vertices.swap(empty_vertices);
	vector<myHalfedge *> empty_halfedges;
	halfedges.swap(empty_halfedges);
	vector<myFace *> empty_faces;
	faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	vector<myHalfedge *>::iterator it;
	for (it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			break;
	}
	if (it != halfedges.end())
		cout << "Error! Not all edges have their twins!\n";
	else
		cout << "Each edge has a twin!\n";
}

bool myMesh::readFile(std::string filename)
{
	string s, t, u;

	ifstream fin(filename);
	if (!fin.is_open())
	{
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge *> twin_map;

	while (getline(fin, s))
	{
		stringstream myline(s);
		myline >> t;
		if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;
			myPoint3D *p = new myPoint3D(x, y, z);
			myVertex *v = new myVertex();
			v->point = p;
			vertices.push_back(v);
		}
		else if (t == "f")
		{
			vector<int> faceids;
			while (myline >> u)
			{
				int vid = atoi((u.substr(0, u.find("/"))).c_str());
				if (vid < 0)
					vid = (int)vertices.size() + vid;
				else
					vid--; // OBJ indices are 1-based
				faceids.push_back(vid);
			}

			if ((int)faceids.size() < 3)
				continue;

			myFace *f = new myFace();
			faces.push_back(f);

			int n = (int)faceids.size();
			vector<myHalfedge *> face_hedges(n);

			for (int i = 0; i < n; i++)
			{
				myHalfedge *he = new myHalfedge();
				he->source = vertices[faceids[i]];
				he->adjacent_face = f;
				if (vertices[faceids[i]]->originof == NULL)
					vertices[faceids[i]]->originof = he;
				halfedges.push_back(he);
				face_hedges[i] = he;
			}

			f->adjacent_halfedge = face_hedges[0];

			for (int i = 0; i < n; i++)
			{
				face_hedges[i]->next = face_hedges[(i + 1) % n];
				face_hedges[i]->prev = face_hedges[(i - 1 + n) % n];

				int src = faceids[i];
				int dst = faceids[(i + 1) % n];
				twin_map[{src, dst}] = face_hedges[i];
			}
		}
	}

	// Link twins
	for (auto &kv : twin_map)
	{
		int src = kv.first.first;
		int dst = kv.first.second;
		auto it = twin_map.find({dst, src});
		if (it != twin_map.end())
		{
			kv.second->twin = it->second;
			it->second->twin = kv.second;
		}
	}

	checkMesh();
	normalize();

	return true;
}

void myMesh::computeNormals()
{
	// Compute face normals
	for (unsigned int i = 0; i < faces.size(); i++)
		faces[i]->computeNormal();

	// Compute vertex normals as average of adjacent face normals
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		myVertex *v = vertices[i];
		myHalfedge *start = v->originof;
		if (!start)
			continue;

		double nx = 0, ny = 0, nz = 0;
		myHalfedge *h = start;
		do
		{
			if (h->adjacent_face && h->adjacent_face->normal)
			{
				nx += h->adjacent_face->normal->dX;
				ny += h->adjacent_face->normal->dY;
				nz += h->adjacent_face->normal->dZ;
			}
			if (!h->twin)
				break;
			h = h->twin->next;
		} while (h != start);

		v->normal->dX = nx;
		v->normal->dY = ny;
		v->normal->dZ = nz;
		v->normal->normalize();
	}
}

void myMesh::normalize()
{
	if (vertices.size() < 1)
		return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X)
			tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X)
			tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y)
			tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y)
			tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z)
			tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z)
			tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		   ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		   zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}

void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{

	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::subdivisionCatmullClark()
{
	/**** TODO ****/
}

void myMesh::simplify()
{
	/**** TODO ****/
}

void myMesh::simplify(myVertex *)
{
	/**** TODO ****/
}

void myMesh::triangulate()
{
	/**** TODO ****/
}

// return false if already triangle, true othewise.
bool myMesh::triangulate(myFace *f)
{
	int n = 0;
	myHalfedge *he = f->adjacent_halfedge;
	do { n++; he = he->next; } while ( he != f->adjacent_halfedge );
	if (n<=3){
		return false;
	}
	vector<myHalfedge *> border;
	he = f->adjacent_halfedge;
	do {border.push_back(he); he = he->next; } while (he != f->adjacent_halfedge);

	myPoint3D *center = new myPoint3D(0,0,0);
	for(int i = 0; i<n; i++){
		center->X += border[i]->source->point->X;
		center->Y += border[i]->source->point->Y;
		center->Z += border[i]->source->point->Z;
	}

	return false;
}
