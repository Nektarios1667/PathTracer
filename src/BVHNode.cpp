#include "BVHNode.h"
#include "AABB.h"
#include <memory>
#include <algorithm>


std::vector<std::unique_ptr<Hittable>> BVHNode::scene;

AABB BVHNode::getBoundingBox(const vector<Hittable*>& objects, size_t start, size_t end) const {
    if (start >= end) return {}; // Return empty

    AABB bounds = objects[start]->getBoundingBox(); // Start with first

    for (size_t i = start + 1; i < end; ++i) {
        bounds = AABB::combine(bounds, objects[i]->getBoundingBox()); // Expand
    }

    return bounds;
}

auto BVHNode::compareAlongLongestAxis(const AABB& bounds) const {
    int axis;
    Vector3 size = bounds.upper - bounds.lower;

    // Get longest axis
    if (size.x > size.y && size.x > size.z) axis = 0;
    else if (size.y > size.x && size.y > size.z) axis = 1;
    else axis = 2;

    // Lambda to sort the triangles by axis
    return [axis](const Hittable* a, const Hittable* b) {
        float ca = a->getBoundingBox().center()[axis];
        float cb = b->getBoundingBox().center()[axis];
        return ca < cb;
    };
}

BVHNode::BVHNode(vector<Hittable*>& objects, size_t start, size_t end) {
    size_t count = end - start;
    bounds = BVHNode::getBoundingBox(objects, start, end);

    if (count == 1) {
        // Leaf
        object = move(objects[start]);
    } else if (count == 2) {
        // Left/right with one child each
        left = make_unique<BVHNode>(objects, start, start + 1);
        right = make_unique<BVHNode>(objects, start + 1, end);
    } else {
        // Sort by axis
        sort(objects.begin() + start, objects.begin() + end, compareAlongLongestAxis(bounds));

        size_t halfway = start + count / 2;

        // Build left/right
        left = make_unique<BVHNode>(objects, start, halfway);
        right = make_unique<BVHNode>(objects, halfway, end);
    }
}
