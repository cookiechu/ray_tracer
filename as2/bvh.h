#ifndef BVH
#define BVH
#include "aabb.h"
#include "intersection.h"

class bvh {
public:

  aabb* get_root();

  aabb* recursive_split(const std::vector<primitive*> &primitives, unsigned int max_leaf);

  bool intersect(ray& r, aabb* node);

  bool intersect(ray& r, aabb* node, intersection *i);

private:
  aabb* root;
  /* data */
};

#endif