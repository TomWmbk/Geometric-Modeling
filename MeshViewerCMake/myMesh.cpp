#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <algorithm>
#include <set>
#include <GL/glew.h>
#include "myVector3D.h"

using namespace std;

namespace
{
	int faceDegree(myFace *f)
	{
		if (!f || !f->adjacent_halfedge)
			return 0;

		int degree = 0;
		myHalfedge *start = f->adjacent_halfedge;
		myHalfedge *he = start;
		do
		{
			if (!he || !he->next)
				return 0;
			degree++;
			he = he->next;
		} while (he != start && degree <= 10000);

		return degree;
	}

	bool isInVector(const vector<myHalfedge *> &items, myHalfedge *item)
	{
		return find(items.begin(), items.end(), item) != items.end();
	}

	void removeHalfedge(vector<myHalfedge *> &halfedges, myHalfedge *he)
	{
		halfedges.erase(remove(halfedges.begin(), halfedges.end(), he), halfedges.end());
	}

	void removeFace(vector<myFace *> &faces, myFace *face)
	{
		faces.erase(remove(faces.begin(), faces.end(), face), faces.end());
	}
}

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
	if (faces.size() == 0)
		return;

	for (int attempt = 0; attempt < 2; attempt++)
	{
		unsigned int oldVertexCount = (unsigned int)vertices.size();
		for (unsigned int i = 0; i < halfedges.size(); i++)
		{
			myHalfedge *e = halfedges[i];
			if (!e || !e->twin || !e->next || !e->prev)
				continue;
			if (faceDegree(e->adjacent_face) != 3 || faceDegree(e->twin->adjacent_face) != 3)
				continue;

			e->source->originof = e;
			simplify(e->source);
			if (vertices.size() < oldVertexCount)
				return;
		}

		if (attempt == 0)
			triangulate();
	}

	cout << "No valid edge collapse found.\n";
}

void myMesh::simplify(myVertex *vx)
{
	if (!vx || !vx->originof)
		return;

	myHalfedge *e = vx->originof;
	if (!e->twin || !e->next || !e->prev || !e->twin->next || !e->twin->prev)
		return;

	myHalfedge *et = e->twin;
	myHalfedge *e_next = e->next;
	myHalfedge *e_prev = e->prev;
	myHalfedge *et_next = et->next;
	myHalfedge *et_prev = et->prev;
	myFace *f1 = e->adjacent_face;
	myFace *f2 = et->adjacent_face;

	if (!f1 || !f2 || f1 == f2 || faceDegree(f1) != 3 || faceDegree(f2) != 3)
		return;
	if (!e_next->twin || !e_prev->twin || !et_next->twin || !et_prev->twin)
		return;

	myVertex *v0 = e->source;
	myVertex *v1 = e_next->source;
	myVertex *v2 = e_prev->source;
	myVertex *v3 = et_prev->source;
	if (!v0 || !v1 || !v2 || !v3 || v0 == v1 || v2 == v3)
		return;

	vector<myHalfedge *> removedHalfedges;
	removedHalfedges.push_back(e);
	removedHalfedges.push_back(et);
	removedHalfedges.push_back(e_next);
	removedHalfedges.push_back(e_prev);
	removedHalfedges.push_back(et_next);
	removedHalfedges.push_back(et_prev);

	// Link condition: avoid creating a non-manifold vertex after the collapse.
	set<myVertex *> n0, n1;
	for (unsigned int i = 0; i < halfedges.size(); i++)
	{
		myHalfedge *he = halfedges[i];
		if (!he || !he->next)
			continue;
		if (he->source == v0)
			n0.insert(he->next->source);
		if (he->next->source == v0)
			n0.insert(he->source);
		if (he->source == v1)
			n1.insert(he->next->source);
		if (he->next->source == v1)
			n1.insert(he->source);
	}

	int commonNeighbours = 0;
	for (set<myVertex *>::iterator it = n0.begin(); it != n0.end(); ++it)
		if (n1.find(*it) != n1.end())
			commonNeighbours++;
	if (commonNeighbours != 2)
		return;

	// A remaining face containing both vertices would become degenerate.
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		myFace *f = faces[i];
		if (!f || f == f1 || f == f2)
			continue;

		bool hasV0 = false;
		bool hasV1 = false;
		myHalfedge *start = f->adjacent_halfedge;
		myHalfedge *he = start;
		do
		{
			if (!he)
				break;
			if (he->source == v0)
				hasV0 = true;
			if (he->source == v1)
				hasV1 = true;
			he = he->next;
		} while (he != start);

		if (hasV0 && hasV1)
			return;
	}

	v0->point->X = (v0->point->X + v1->point->X) / 2.0;
	v0->point->Y = (v0->point->Y + v1->point->Y) / 2.0;
	v0->point->Z = (v0->point->Z + v1->point->Z) / 2.0;

	// Every remaining halfedge that started from v1 now starts from v0.
	for (unsigned int i = 0; i < halfedges.size(); i++)
	{
		myHalfedge *he = halfedges[i];
		if (he && !isInVector(removedHalfedges, he) && he->source == v1)
			he->source = v0;
	}

	// The two deleted triangles leave two pairs of boundary halfedges to reconnect.
	e_prev->twin->twin = e_next->twin;
	e_next->twin->twin = e_prev->twin;
	et_prev->twin->twin = et_next->twin;
	et_next->twin->twin = et_prev->twin;

	for (unsigned int i = 0; i < removedHalfedges.size(); i++)
		removeHalfedge(halfedges, removedHalfedges[i]);
	removeFace(faces, f1);
	removeFace(faces, f2);
	vertices.erase(remove(vertices.begin(), vertices.end(), v1), vertices.end());

	for (unsigned int i = 0; i < vertices.size(); i++)
		vertices[i]->originof = NULL;
	for (unsigned int i = 0; i < halfedges.size(); i++)
		if (halfedges[i] && halfedges[i]->source && !halfedges[i]->source->originof)
			halfedges[i]->source->originof = halfedges[i];

	delete e;
	delete et;
	delete e_next;
	delete e_prev;
	delete et_next;
	delete et_prev;
	delete f1;
	delete f2;
	if (v1->point)
		delete v1->point;
	delete v1;
}

void myMesh::triangulate()
{
	vector<vector<myVertex *> > triangles;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		myFace *f = faces[i];
		if (!f || !f->adjacent_halfedge)
			continue;

		vector<myVertex *> faceVertices;
		myHalfedge *start = f->adjacent_halfedge;
		myHalfedge *he = start;
		do
		{
			if (!he || !he->source)
				break;
			faceVertices.push_back(he->source);
			he = he->next;
		} while (he && he != start && faceVertices.size() <= 10000);

		if (faceVertices.size() < 3)
			continue;

		for (unsigned int j = 1; j + 1 < faceVertices.size(); j++)
		{
			if (faceVertices[0] == faceVertices[j] ||
				faceVertices[j] == faceVertices[j + 1] ||
				faceVertices[j + 1] == faceVertices[0])
				continue;

			vector<myVertex *> tri;
			tri.push_back(faceVertices[0]);
			tri.push_back(faceVertices[j]);
			tri.push_back(faceVertices[j + 1]);
			triangles.push_back(tri);
		}
	}

	for (unsigned int i = 0; i < halfedges.size(); i++)
		delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++)
		delete faces[i];

	halfedges.clear();
	faces.clear();
	for (unsigned int i = 0; i < vertices.size(); i++)
		vertices[i]->originof = NULL;

	map<pair<myVertex *, myVertex *>, myHalfedge *> twinMap;

	for (unsigned int i = 0; i < triangles.size(); i++)
	{
		myFace *f = new myFace();
		myHalfedge *h0 = new myHalfedge();
		myHalfedge *h1 = new myHalfedge();
		myHalfedge *h2 = new myHalfedge();

		h0->source = triangles[i][0];
		h1->source = triangles[i][1];
		h2->source = triangles[i][2];

		h0->next = h1; h1->next = h2; h2->next = h0;
		h0->prev = h2; h1->prev = h0; h2->prev = h1;

		h0->adjacent_face = f;
		h1->adjacent_face = f;
		h2->adjacent_face = f;
		f->adjacent_halfedge = h0;

		faces.push_back(f);
		halfedges.push_back(h0);
		halfedges.push_back(h1);
		halfedges.push_back(h2);

		if (!h0->source->originof) h0->source->originof = h0;
		if (!h1->source->originof) h1->source->originof = h1;
		if (!h2->source->originof) h2->source->originof = h2;

		myHalfedge *hs[3] = { h0, h1, h2 };
		for (int k = 0; k < 3; k++)
		{
			myVertex *src = hs[k]->source;
			myVertex *dst = hs[k]->next->source;
			map<pair<myVertex *, myVertex *>, myHalfedge *>::iterator twinIt =
				twinMap.find(make_pair(dst, src));

			if (twinIt != twinMap.end())
			{
				hs[k]->twin = twinIt->second;
				twinIt->second->twin = hs[k];
			}
			else
			{
				twinMap[make_pair(src, dst)] = hs[k];
			}
		}
	}
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
