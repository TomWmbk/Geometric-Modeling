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
	for (unsigned int i = 0; i < faces.size(); i++)
		faces[i]->computeNormal();

	for (unsigned int i = 0; i < vertices.size(); i++)
		vertices[i]->computeNormal();
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
	if (vertices.size() >= 4){
		simplify(vertices[0]);
	}
}

void myMesh::simplify(myVertex *vx)
{
	myHalfedge *e = vx->originof;
	if(!e->twin){
		return;
	}
	myVertex *vn = e->next->source;

	vx->point->X = (vx->point->X + vn->point->X)/2.0f;
	vx->point->Y = (vx->point->Y + vn->point->Y)/2.0f;
	vx->point->Z = (vx->point->Z + vn->point->Z)/2.0f;

	myHalfedge *he = e->twin;
	do{
		he->source = vx;
		he = he->twin->next;
	}while(he != e->twin);

	e->prev->twin->twin = e->next->twin;
	e->next->twin->twin = e->prev->twin;

	e->twin->prev->twin->twin = e->twin->next->twin;
	e->twin->next->twin->twin = e->twin->prev->twin;

	vx->originof = e->twin->next->twin;

	
}

void myMesh::triangulate()
{
    vector<myFace *> todo;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        int n = 0;
        myHalfedge *he = faces[i]->adjacent_halfedge;
        do { n++; he = he->next; } while (he != faces[i]->adjacent_halfedge);
        if (n > 3) todo.push_back(faces[i]);
    }
    for (unsigned int i = 0; i < todo.size(); i++)
        triangulate(todo[i]);
}

bool myMesh::triangulate(myFace *f)
{
	const double eps = 1e-10;
	int n = 0;
	myHalfedge *e = f->adjacent_halfedge;
	do { n++; e = e->next; } while (e != f->adjacent_halfedge);
	if (n == 3) return false;

	vector<myHalfedge *> hedges(n);
	vector<myVertex *> verts(n);
	e = f->adjacent_halfedge;
	for (int i = 0; i < n; i++) { hedges[i] = e; verts[i] = e->source; e = e->next; }

	myVector3D faceN(0, 0, 0);
	for (int i = 0; i < n; i++)
	{
		myPoint3D *a = verts[i]->point, *b = verts[(i + 1) % n]->point;
		faceN.dX += (a->Y - b->Y) * (a->Z + b->Z);
		faceN.dY += (a->Z - b->Z) * (a->X + b->X);
		faceN.dZ += (a->X - b->X) * (a->Y + b->Y);
	}
	if (faceN.length() < eps) return false;

	vector<int> nxt(n), prv(n);
	for (int i = 0; i < n; i++) { nxt[i] = (i + 1) % n; prv[i] = (i - 1 + n) % n; }

	int remaining = n;
	int cur = 0;
	while (remaining > 3)
	{
		bool found = false;
		int startIdx = cur;
		do
		{
			int p = prv[cur];
			int nx = nxt[cur];

			myVector3D v1 = *verts[cur]->point - *verts[p]->point;
			myVector3D v2 = *verts[nx]->point - *verts[cur]->point;
			if (v1.crossproduct(v2) * faceN > 0)
			{
				bool isEar = true;
				int test = nxt[nx];
				while (test != p)
				{
					myVector3D c0 = (*verts[cur]->point - *verts[p]->point).crossproduct(*verts[test]->point - *verts[p]->point);
					myVector3D c1 = (*verts[nx]->point - *verts[cur]->point).crossproduct(*verts[test]->point - *verts[cur]->point);
					myVector3D c2 = (*verts[p]->point - *verts[nx]->point).crossproduct(*verts[test]->point - *verts[nx]->point);
					if (c0 * faceN > eps && c1 * faceN > eps && c2 * faceN > eps) { isEar = false; break; }
					test = nxt[test];
				}
				if (isEar)
				{
					myHalfedge *d_in = new myHalfedge();
					myHalfedge *d_out = new myHalfedge();
					d_in->source = verts[nx];
					d_out->source = verts[p];
					d_in->twin = d_out;
					d_out->twin = d_in;
					halfedges.push_back(d_in);
					halfedges.push_back(d_out);
					myFace *nf = new myFace();
					faces.push_back(nf);
					nf->adjacent_halfedge = hedges[p];
					hedges[p]->next = hedges[cur];
					hedges[cur]->next = d_in;
					d_in->next = hedges[p];
					hedges[p]->prev = d_in;
					hedges[cur]->prev = hedges[p];
					d_in->prev = hedges[cur];
					hedges[p]->adjacent_face = nf;
					hedges[cur]->adjacent_face = nf;
					d_in->adjacent_face = nf;
					hedges[p] = d_out;
					nxt[p] = nx;
					prv[nx] = p;
					remaining--;
					cur = nx;
					found = true;
					break;
				}
			}
			cur = nxt[cur];
		} while (cur != startIdx);
		if (!found) break;
	}

	int i0 = cur;
	int i1 = nxt[i0];
	int i2 = nxt[i1];
	f->adjacent_halfedge = hedges[i0];
	hedges[i0]->next = hedges[i1];
	hedges[i1]->next = hedges[i2];
	hedges[i2]->next = hedges[i0];
	hedges[i0]->prev = hedges[i2];
	hedges[i1]->prev = hedges[i0];
	hedges[i2]->prev = hedges[i1];
	hedges[i0]->adjacent_face = f;
	hedges[i1]->adjacent_face = f;
	hedges[i2]->adjacent_face = f;
	return true;
}

void myMesh::testTriangulation()
{
    unsigned int nontri = 0;
    for (unsigned int i = 0; i < faces.size(); ++i) {
        myFace *f = faces[i];
        if (!f || !f->adjacent_halfedge) { ++nontri; continue; }
        myHalfedge *h = f->adjacent_halfedge;
        unsigned int cnt = 0;
        myHalfedge *cur = h;
        do {
            if (!cur) break;
            ++cnt;
            cur = cur->next;
            if (cnt > 10000) break;
        } while (cur != h);
        if (cnt != 3) ++nontri;
    }
    cout << "[testTriangulation] nontriangular_faces=" << nontri << " total_faces=" << faces.size() << "\n";
}

void myMesh::testNormals()
{
    unsigned int faceBad = 0, vertBad = 0;
    for (unsigned int i = 0; i < faces.size(); ++i) {
        myFace *f = faces[i];
        if (!f || !f->normal) { ++faceBad; continue; }
        double L = f->normal->length();
        if (!(L > 1e-9)) { ++faceBad; }
    }
    for (unsigned int i = 0; i < vertices.size(); ++i) {
        myVertex *v = vertices[i];
        if (!v || !v->normal) { ++vertBad; continue; }
        double L = v->normal->length();
        if (!(L > 1e-9)) { ++vertBad; }
    }
    cout << "[testNormals] faces_bad=" << faceBad << " verts_bad=" << vertBad << "\n";
}

void myMesh::testHalfedges()
{
    unsigned int errors = 0;
    for (unsigned int i = 0; i < halfedges.size(); ++i) {
        myHalfedge *h = halfedges[i];
        if (!h) { ++errors; continue; }
        if (h->next == NULL || h->prev == NULL || h->source == NULL) {
            cout << "[testHalfedges] invalid halfedge at index " << i << "\n";
            ++errors; continue;
        }
        if (h->next->prev != h) {
            cout << "[testHalfedges] next->prev mismatch at halfedge " << i << "\n";
            ++errors;
        }
        if (h->prev->next != h) {
            cout << "[testHalfedges] prev->next mismatch at halfedge " << i << "\n";
            ++errors;
        }
        if (h->twin && h->twin->twin != h) {
            cout << "[testHalfedges] twin mismatch at halfedge " << i << "\n";
            ++errors;
        }
    }
    cout << "[testHalfedges] done. checked=" << halfedges.size() << " errors=" << errors << "\n";
}

void myMesh::surfaceOfRevolution() {
    vector<pair<float,float>> curve = {
        {0.0f, -1.0f},
        {0.5f, -0.5f},
        {0.8f,  0.0f},
        {0.5f,  0.5f},
        {0.0f,  1.0f}
    };

    int n = curve.size();
    int m = 20;

    vector<int> rowStart(n);

    for (int i = 0; i < n; i++) {
        float r = curve[i].first;
        float y = curve[i].second;
        rowStart[i] = (int)vertices.size();
        if (r == 0.0f) {
            myVertex *v = new myVertex();
            v->point = new myPoint3D(0.0f, y, 0.0f);
            vertices.push_back(v);
        } else {
            for (int j = 0; j < m; j++) {
                float theta = 2.0f * M_PI * j / m;
                myVertex *v = new myVertex();
                v->point = new myPoint3D(r * cos(theta), y, r * sin(theta));
                vertices.push_back(v);
            }
        }
    }

    map<pair<int,int>, myHalfedge*> twin_map;

    auto addTriangle = [&](int a, int b, int c) {
        if (a == b || b == c || a == c) return; 
        int ids[3] = {a, b, c};
        myFace *f = new myFace();
        faces.push_back(f);
        myHalfedge *hes[3];
        for (int k = 0; k < 3; k++) {
            myHalfedge *he = new myHalfedge();
            he->source = vertices[ids[k]];
            he->adjacent_face = f;
            if (vertices[ids[k]]->originof == NULL)
                vertices[ids[k]]->originof = he;
            halfedges.push_back(he);
            hes[k] = he;
        }
        f->adjacent_halfedge = hes[0];
        for (int k = 0; k < 3; k++) {
            hes[k]->next = hes[(k+1)%3];
            hes[k]->prev = hes[(k-1+3)%3];
            twin_map[{ids[k], ids[(k+1)%3]}] = hes[k];
        }
    };

    auto vertIdx = [&](int row, int j) -> int {
        if (curve[row].first == 0.0f) return rowStart[row]; 
        return rowStart[row] + j % m;
    };

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < m; j++) {
            int a = vertIdx(i,   j);
            int b = vertIdx(i,   j+1);
            int c = vertIdx(i+1, j);
            int d = vertIdx(i+1, j+1);
            addTriangle(a, d, b);
            addTriangle(a, c, d);
        }
    }

    for (auto &kv : twin_map) {
        auto it = twin_map.find({kv.first.second, kv.first.first});
        if (it != twin_map.end()) {
            kv.second->twin = it->second;
            it->second->twin = kv.second;
        }
    }

    normalize();
}