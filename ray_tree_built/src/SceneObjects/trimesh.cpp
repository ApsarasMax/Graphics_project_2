#include <cmath>
#include <float.h>
#include <algorithm>
#include <assert.h>
#include "trimesh.h"
#include "../ui/TraceUI.h"
#include "../ui/GraphicalUI.h"

extern TraceUI* traceUI;
extern GraphicalUI* graphicalUI;

using namespace std;

Trimesh::~Trimesh()
{
	for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
		delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const Vec3d &v )
{
    vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
    materials.push_back( m );
}

void Trimesh::addNormal( const Vec3d &n )
{
    normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
    int vcnt = vertices.size();

    if( a >= vcnt || b >= vcnt || c >= vcnt ) return false;

    TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material),  this, a, b, c );
    newFace->setTransform(this->transform);
    if (!newFace->degen) faces.push_back( newFace );


    // Don't add faces to the scene's object list so we can cull by bounding box
    // scene->add(newFace);
    return true;
}

char* Trimesh::doubleCheck()
// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
{
    if( !materials.empty() && materials.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of materials.";
    if( !normals.empty() && normals.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

bool Trimesh::intersectLocal(ray& r, isect& i) const
{
	double tmin = 0.0;
	double tmax = 0.0;
	typedef Faces::const_iterator iter;
	bool have_one = false;

    if(!kdTreeBuilt){
    	for( iter j = faces.begin(); j != faces.end(); ++j )
    	  {
    	    isect cur;
    	    if( (*j)->intersectLocal( r, cur ) )
    	      {
        		if( !have_one || (cur.t < i.t) )
        		  {
        		    i = cur;
        		    have_one = true;
        		  }
    	      }
    	  }
    }else{
        have_one = kdtree->intersect(r, i);
    }
	if( !have_one ) i.setT(1000.0);
	return have_one;
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray& r, isect& i) const
{

    const Vec3d& a = parent->vertices[ids[0]];
    const Vec3d& b = parent->vertices[ids[1]];
    const Vec3d& c = parent->vertices[ids[2]];

    // YOUR CODE HERE
    Vec3d u = b-a;
    Vec3d v = c-a;

    //calculating normal
    Vec3d n = u^v;
    n.normalize();
    //calculating count
    double d = 0-(n*a);

    //calculating t=-(d(num)+n(vec)*p(vec))/(n(vec)*d(vec))
    double t = -(d+n*(r.p))/(n*(r.d));
    if(t<RAY_EPSILON) return false;
    //calculating intersect
    Vec3d p = r.at(t);

    //judge if intersect
    Vec3d vec_ab = b-a;
    Vec3d vec_bc = c-b;
    Vec3d vec_ca = a-c;

    Vec3d vec_ap = p-a;
    Vec3d vec_bp = p-b;
    Vec3d vec_cp = p-c;

    if((vec_ab^vec_ap) * (vec_bc^vec_bp)>=0&&
        (vec_ab^vec_ap) * (vec_ca^vec_cp)>=0&&
        (vec_bc^vec_bp) * (vec_ca^vec_cp)>=0){

        double alpha, beta, gamma;
        alpha = (vec_bc^vec_bp).length()/(vec_ab^vec_bc).length();
        beta = (vec_ca^vec_cp).length()/(vec_ab^vec_bc).length();
        gamma = (vec_ab^vec_ap).length()/(vec_ab^vec_bc).length();

        i.setObject(this);
        i.setT(t);

        if(parent -> normals.empty()){
            i.setN(n);
        }else if(traceUI->smShadSw()){
                Vec3d n_a = parent->normals[ids[0]];
                Vec3d n_b = parent->normals[ids[1]];
                Vec3d n_c = parent->normals[ids[2]];
                i.setN(alpha * n_a + beta * n_b + gamma * n_c);
        }else{
                i.setN(n);
            
        }

        if(parent -> materials.empty()){
            i.setMaterial(i.getMaterial());
        }else{
            Material material;
            material += (alpha * (*(parent->materials[ids[0]])));
            material += (beta * (*(parent->materials[ids[1]])));
            material += (gamma * (*(parent->materials[ids[2]])));
            i.setMaterial(material);
        }

        i.setBary(alpha, beta, gamma);

        return true;
    }

    return false;
}

void Trimesh::generateNormals()
// Once you've loaded all the verts and faces, we can generate per
// vertex normals by averaging the normals of the neighboring faces.
{
    int cnt = vertices.size();
    normals.resize( cnt );
    int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
    memset( numFaces, 0, sizeof(int)*cnt );
    
    for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
    {
		Vec3d faceNormal = (**fi).getNormal();
        
        for( int i = 0; i < 3; ++i )
        {
            normals[(**fi)[i]] += faceNormal;
            ++numFaces[(**fi)[i]];
        }
    }

    for( int i = 0; i < cnt; ++i )
    {
        if( numFaces[i] )
            normals[i]  /= numFaces[i];
    }

    delete [] numFaces;
    vertNorms = true;
}

void Trimesh::buildKdTree(){
    if(!kdTreeBuilt){
        std::vector<Geometry*> objects(faces.size());
        std::copy(faces.begin(), faces.end(), objects.begin());
        kdtree = new KdTree(graphicalUI->m_nKdtreeMaxDepth, localBounds, graphicalUI->m_nKdtreeLeafSize);
        kdtree->addObjects(objects);
        kdTreeBuilt = true;
    }
}
