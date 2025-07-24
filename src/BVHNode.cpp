#include "BVHNode.h"
#include "AABB.h"
#include <memory>
#include <algorithm>


AABB BVHNode::getBoundingBox(const vector<unique_ptr<Hittable>>& objects) const {
    if (objects.empty()) return {}; // Return empty

    AABB bounds = objects[0]->getBoundingBox(); // Start with first

    for (size_t i = 1; i < objects.size(); ++i) {
        bounds = BVHNode::surroundingBox(bounds, objects[i]->getBoundingBox()); // Expand
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
    return [axis](Hittable* a, Hittable* b) {
        float ca = a->getBoundingBox().center()[axis];
        float cb = b->getBoundingBox().center()[axis];
        return ca < cb;
    };
}

BVHNode::BVHNode(vector<unique_ptr<Hittable>>&& objects, size_t start, size_t end) {
    size_t count = end - start;
    bounds = BVHNode::getBoundingBox(objects);

    if (count == 1) {
        BVHNode::objects.emplace_back(move(objects[start]));
    } else if (count == 2) {
        left = make_unique<BVHNode>(vector<unique_ptr<Hittable>>{ move(objects[start]) }, 0, 1);
        right = make_unique<BVHNode>(vector<unique_ptr<Hittable>>{ move(objects[start + 1]) }, 0, 1);
    } else {
        sort(objects.begin(), objects.end(), compareAlongLongestAxis(bounds));

        size_t halfway = objects.size() / 2;
        vector<unique_ptr<Hittable>> leftObjs(objects.begin(), objects.begin() + halfway);
        vector<unique_ptr<Hittable>> rightObjs(objects.begin() + halfway, objects.end());

        left = std::make_unique<BVHNode>(leftObjs);
        right = std::make_unique<BVHNode>(rightObjs);
    }
}
