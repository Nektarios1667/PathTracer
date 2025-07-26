#include "BVHNode.h"
#include "AABB.h"
#include <memory>
#include <algorithm>

AABB BVHNode::surroundingBox(const AABB& box1, const AABB& box2) const {
    Vector3 small(
        std::min(box1.lower.x, box2.lower.x),
        std::min(box1.lower.y, box2.lower.y),
        std::min(box1.lower.z, box2.lower.z)
    );
    Vector3 big(
        std::max(box1.upper.x, box2.upper.x),
        std::max(box1.upper.y, box2.upper.y),
        std::max(box1.upper.z, box2.upper.z)
    );
    return {small, big};
}

AABB BVHNode::getBoundingBox(const vector<unique_ptr<Hittable>>& objects, size_t start, size_t end) const {
    if (start >= end) return {}; // Return empty

    AABB bounds = objects[start]->getBoundingBox(); // Start with first

    for (size_t i = start + 1; i < end; ++i) {
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
    return [axis](const unique_ptr<Hittable>& a, const unique_ptr<Hittable>& b) {
        float ca = a->getBoundingBox().center()[axis];
        float cb = b->getBoundingBox().center()[axis];
        return ca < cb;
    };
}

BVHNode::BVHNode(vector<unique_ptr<Hittable>>& objects, size_t start, size_t end) {
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
