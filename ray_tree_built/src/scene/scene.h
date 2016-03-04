//
// scene.h
//
// The Scene class and the geometric types that it can contain.
//

#pragma warning (disable: 4786)


#ifndef __SCENE_H__
#define __SCENE_H__

#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <memory>
#include <stack>

#include "ray.h"
#include "material.h"
#include "camera.h"
#include "bbox.h"

#include "../vecmath/vec.h"
#include "../vecmath/mat.h"

using namespace std;

class Light;
class Scene;

class SceneElement {

public:
  virtual ~SceneElement() {}

  Scene *getScene() const { return scene; }

  // For debugging purposes, draws using OpenGL
  virtual void glDraw(int quality, bool actualMaterials, bool actualTextures) const  { }

 protected:
 SceneElement( Scene *s )
   : scene( s ) {}

  Scene *scene;
};

class TransformNode {

protected:

  // information about this node's transformation
  Mat4d    xform;
  Mat4d    inverse;
  Mat3d    normi;

  // information about parent & children
  TransformNode *parent;
  std::vector<TransformNode*> children;
    
 public:
  typedef std::vector<TransformNode*>::iterator          child_iter;
  typedef std::vector<TransformNode*>::const_iterator    child_citer;

  ~TransformNode() {
      for(child_iter c = children.begin(); c != children.end(); ++c ) delete (*c);
    }

  TransformNode *createChild(const Mat4d& xform) {
    TransformNode *child = new TransformNode(this, xform);
    children.push_back(child);
    return child;
  }
    
  // Coordinate-Space transformation
  Vec3d globalToLocalCoords(const Vec3d &v) { return inverse * v; }

  Vec3d localToGlobalCoords(const Vec3d &v) { return xform * v; }

  Vec4d localToGlobalCoords(const Vec4d &v) { return xform * v; }

  Vec3d localToGlobalCoordsNormal(const Vec3d &v) {
    Vec3d ret = normi * v;
    ret.normalize();
    return ret;
  }

  const Mat4d& transform() const		{ return xform; }

protected:
  // protected so that users can't directly construct one of these...
  // force them to use the createChild() method.  Note that they CAN
  // directly create a TransformRoot object.
 TransformNode(TransformNode *parent, const Mat4d& xform ) : children() {
      this->parent = parent;
      if (parent == NULL) this->xform = xform;
      else this->xform = parent->xform * xform;  
      inverse = this->xform.inverse();
      normi = this->xform.upper33().inverse().transpose();
    }
};

class TransformRoot : public TransformNode {
 public:
 TransformRoot() : TransformNode(NULL, Mat4d()) {}
};

// A Geometry object is anything that has extent in three dimensions.
// It may not be an actual visible scene object.  For example, hierarchical
// spatial subdivision could be expressed in terms of Geometry instances.
class Geometry : public SceneElement {

//protected:
  public://zyc
  // intersections performed in the object's local coordinate space
  // do not call directly - this should only be called by intersect()
  virtual bool intersectLocal(ray& r, isect& i ) const = 0;

public:
  // intersections performed in the global coordinate space.
  bool intersect(ray& r, isect& i) const;


  virtual bool hasBoundingBoxCapability() const;
  const BoundingBox& getBoundingBox() const { return bounds; }
  Vec3d getNormal() { return Vec3d(1.0, 0.0, 0.0); }

  virtual void ComputeBoundingBox() {
    // take the object's local bounding box, transform all 8 points on it,
    // and use those to find a new bounding box.

    BoundingBox localBounds = ComputeLocalBoundingBox();
        
    Vec3d min = localBounds.getMin();
    Vec3d max = localBounds.getMax();

    Vec4d v, newMax, newMin;

    v = transform->localToGlobalCoords( Vec4d(min[0], min[1], min[2], 1) );
    newMax = v;
    newMin = v;
    v = transform->localToGlobalCoords( Vec4d(max[0], min[1], min[2], 1) );
    newMax = maximum(newMax, v);
    newMin = minimum(newMin, v);
    v = transform->localToGlobalCoords( Vec4d(min[0], max[1], min[2], 1) );
    newMax = maximum(newMax, v);
    newMin = minimum(newMin, v);
    v = transform->localToGlobalCoords( Vec4d(max[0], max[1], min[2], 1) );
    newMax = maximum(newMax, v);
    newMin = minimum(newMin, v);
    v = transform->localToGlobalCoords( Vec4d(min[0], min[1], max[2], 1) );
    newMax = maximum(newMax, v);
    newMin = minimum(newMin, v);
    v = transform->localToGlobalCoords( Vec4d(max[0], min[1], max[2], 1) );
    newMax = maximum(newMax, v);
    newMin = minimum(newMin, v);
    v = transform->localToGlobalCoords( Vec4d(min[0], max[1], max[2], 1) );
    newMax = maximum(newMax, v);
    newMin = minimum(newMin, v);
    v = transform->localToGlobalCoords( Vec4d(max[0], max[1], max[2], 1) );
    newMax = maximum(newMax, v);
    newMin = minimum(newMin, v);
		
    bounds.setMax(Vec3d(newMax));
    bounds.setMin(Vec3d(newMin));
  }

  // default method for ComputeLocalBoundingBox returns a bogus bounding box;
  // this should be overridden if hasBoundingBoxCapability() is true.
  virtual BoundingBox ComputeLocalBoundingBox() { return BoundingBox(); }

  void setTransform(TransformNode *transform) { this->transform = transform; };
    
 Geometry(Scene *scene) : SceneElement( scene ) {}

  // For debugging purposes, draws using OpenGL
  void glDraw(int quality, bool actualMaterials, bool actualTextures) const;

  // The defult does nothing; this is here because it is not required
  // that you implement this function if you create your own scene objects.
  virtual void glDrawLocal(int quality, bool actualMaterials, bool actualTextures) const { }
  virtual void buildKdTree() {}


  
 protected:
  BoundingBox bounds;
  TransformNode *transform;
};

// A SceneObject is a real actual thing that we want to model in the 
// world.  It has extent (its Geometry heritage) and surface properties
// (its material binding).  The decision of how to store that material
// is left up to the subclass.
class SceneObject : public Geometry {

 public:
  virtual const Material& getMaterial() const = 0;
  virtual void setMaterial(Material *m) = 0;

  void glDraw(int quality, bool actualMaterials, bool actualTextures) const;

 protected:
 SceneObject( Scene *scene )
   : Geometry( scene ) {}
};

// A simple extension of SceneObject that adds an instance of Material
// for simple material bindings.
class MaterialSceneObject : public SceneObject {

public:
  virtual ~MaterialSceneObject() { delete material; }

  virtual const Material& getMaterial() const { return *material; }
  virtual void setMaterial(Material* m)	{ delete material; material = m; }

protected:
 MaterialSceneObject(Scene *scene, Material *mat) 
   : SceneObject(scene), material(mat) {}

  Material* material;
};


//template <typename Obj>
//class KdTree;
class KdTree{
    private:
        int currentAxis;
        int depth;
        BoundingBox bbox;
        std::vector <Geometry*> objects;
        double split;
        int size;
        
    public:
//TransformNode *transform;

        KdTree* left;
        KdTree* right;
        KdTree (int depth, BoundingBox bbox, int size) {
          this->bbox = bbox;
          this->depth = depth;
          currentAxis = 0;
          this->size = size;
          left = nullptr;
          right = nullptr;
        }

        ~KdTree() {
          if (!objects.empty())
            objects.clear();
          if (left != nullptr)
            delete left;
          if (right != nullptr)
            delete right;
        }
        std::vector<Geometry*> getObjects() {
          return objects;
        }

        void setCurrentAxis(int currentAxis) {
          this->currentAxis = currentAxis;
        }

        void addObjects (std::vector<Geometry*> &objects) {
            if (objects.size() < size || depth <= 0) {
              this->objects = objects;
              return;
            }

            //obtain the split point (in one specified axis)
            split = 0.0;
            std::vector<Geometry*>::iterator obj;
            for( obj = objects.begin(); obj != objects.end(); ++obj ){
              BoundingBox tmpBox = (*obj) -> ComputeLocalBoundingBox();
              //split += (tmpBox.getMin()[currentAxis] + tmpBox.getMax()[currentAxis]) / 2;
              split += (max(tmpBox.getMin()[currentAxis], bbox.getMin()[currentAxis]) + min(tmpBox.getMax()[currentAxis], bbox.getMax()[currentAxis])) / 2;
            }
            split /= objects.size();
           
            std::vector<Geometry*> left_objects;
            std::vector<Geometry*> right_objects;
            int countLeft = 0;

            for( obj = objects.begin(); obj != objects.end(); ++obj ){
              BoundingBox tmpBox = (*obj) -> ComputeLocalBoundingBox();
              if((tmpBox.getMin()[currentAxis] + tmpBox.getMax()[currentAxis]) / 2<=split){

                left_objects.push_back((*obj));
        		  if(tmpBox.getMax()[currentAxis]>=split){
        			  right_objects.push_back((*obj));	
		          }
                countLeft ++;
              }else{
                right_objects.push_back((*obj));
		            if(tmpBox.getMin()[currentAxis]<=split){
			           left_objects.push_back((*obj));
		            }
              }
            }

            //trim leaves
            if (countLeft == 0 || countLeft == objects.size()) {//mark
              this->objects = objects;
              return;
            }

            Vec3d split_min = bbox.getMin();
            Vec3d split_max = bbox.getMax();
            split_min[currentAxis] = split;
            split_max[currentAxis] = split;
            
            // recursively build tree
            left = new KdTree(depth - 1, BoundingBox(bbox.getMin(), split_max), size);
            left->setCurrentAxis((currentAxis + 1) % 3);
            left->addObjects(left_objects);
            right = new KdTree(depth - 1, BoundingBox(split_min, bbox.getMax()), size);
            right->setCurrentAxis((currentAxis + 1) % 3);
            right->addObjects(right_objects);
      }


    bool intersect(ray& r, isect& i) {
        double tmin = 0.0;
        double tmax = 0.0;
        // get intersection time
        if(!bbox.intersect(r, tmin, tmax)){
          return false;
        }

        bool have_one = false;

        // only leaves have objects
        std::vector<Geometry*>::iterator obj;
        for( obj = objects.begin(); obj != objects.end(); ++obj ){
            isect cur;
            if ((*obj)->intersect(r, cur)) {//Local
                if (!have_one || (cur.t < i.t)) {
                    i = cur;
                    have_one = true;
                }
            }
        }

        //terminate
        if (objects.size() != 0) {
            if (!have_one)
              i.setT(1000.0);
            return have_one;
        }

        // get intersection point
        double min = r.at(tmin)[currentAxis];
        double max = r.at(tmax)[currentAxis];

        // traversal
        if (min <= max + RAY_EPSILON && max - RAY_EPSILON <= split) { 
          return left->intersect(r, i); 
        }

        if (min <= max + RAY_EPSILON && min + RAY_EPSILON >= split) { 
          return right->intersect(r, i); 
        }
        if (min <= max + RAY_EPSILON && min - RAY_EPSILON <= split && split <= max + RAY_EPSILON) {
            if (left->intersect(r, i)) 
              return true;
            if (right->intersect(r, i)) 
              return true;
        } 

        if (min + RAY_EPSILON >= max && max + RAY_EPSILON >= split) { 
          return right->intersect(r, i); 
        }

        if (min + RAY_EPSILON >= max && min - RAY_EPSILON <= split) { 
          return left->intersect(r, i); 
        }

        if (min + RAY_EPSILON >= max && max -RAY_EPSILON <= split && split <= min + RAY_EPSILON) { 
            if (right->intersect(r, i))
                return true;
            if (left->intersect(r, i)) 
                return true;
        }
        return false;
    }
};


class Scene {

public:
  typedef std::vector<Light*>::iterator	liter;
  typedef std::vector<Light*>::const_iterator cliter;
  typedef std::vector<Geometry*>::iterator giter;
  typedef std::vector<Geometry*>::const_iterator cgiter;

  TransformRoot transformRoot;

  Scene() : transformRoot(), objects(), lights() {}
  virtual ~Scene();

  void add( Geometry* obj ) {
    obj->ComputeBoundingBox();
	sceneBounds.merge(obj->getBoundingBox());
    objects.push_back(obj);
  }
  void add(Light* light) { lights.push_back(light); }

  bool intersect(ray& r, isect& i) const;

  std::vector<Light*>::const_iterator beginLights() const { return lights.begin(); }
  std::vector<Light*>::const_iterator endLights() const { return lights.end(); }

  std::vector<Geometry*>::const_iterator beginObjects() const { return objects.begin(); }
  std::vector<Geometry*>::const_iterator endObjects() const { return objects.end(); }
        
  const Camera& getCamera() const { return camera; }
  Camera& getCamera() { return camera; }

  // For efficiency reasons, we'll store texture maps in a cache
  // in the Scene.  This makes sure they get deleted when the scene
  // is destroyed.
  TextureMap* getTexture( string name );

  // These two functions are for handling ambient light; in the Phong model,
  // the "ambient" light is considered a property of the _scene_ as a whole
  // and hence should be set here.
  Vec3d ambient() const	{ return ambientIntensity; }
  void addAmbient( const Vec3d& ambient ) { ambientIntensity += ambient; }

  void glDraw(int quality, bool actualMaterials, bool actualTextures) const;

  const BoundingBox& bounds() const { return sceneBounds; }

  void buildKdTree(int depth, int size){
    giter g;
    for( g = objects.begin(); g != objects.end(); ++g ){
      (*g)->buildKdTree();
    }
    kdtree = new KdTree(depth, sceneBounds, size);
    kdtree->addObjects(objects);
  }


 private:
  std::vector<Geometry*> objects;
  std::vector<Geometry*> nonboundedobjects;
  std::vector<Geometry*> boundedobjects;
  std::vector<Light*> lights;
  Camera camera;

  // This is the total amount of ambient light in the scene
  // (used as the I_a in the Phong shading model)
  Vec3d ambientIntensity;

  typedef std::map< std::string, TextureMap* > tmap;
  tmap textureCache;
	
  // Each object in the scene, provided that it has hasBoundingBoxCapability(),
  // must fall within this bounding box.  Objects that don't have hasBoundingBoxCapability()
  // are exempt from this requirement.
  BoundingBox sceneBounds;
  
  //KdTree<Geometry>* kdtree;
  KdTree* kdtree;



 public:
  // This is used for debugging purposes only.
  mutable std::vector<std::pair<ray*, isect*> > intersectCache;
};

#endif // __SCENE_H__
