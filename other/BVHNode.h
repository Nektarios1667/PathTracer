#include "AABB.h"
#include "Hittable.h"
#include <memory>
#include <vector>

struct BVHNode {
    AABB bounds;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    std::vector<unique_ptr<Hittable>> objects; // For leaf

    BVHNode(vector<unique_ptr<Hittable>>& objects, size_t start, size_t end);

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }

    AABB surroundingBox(const AABB& box1, const AABB& box2) const;
    AABB getBoundingBox(const vector<unique_ptr<Hittable>>& objects, size_t start, size_t end) const;
    auto compareAlongLongestAxis(const AABB& bounds) const;
};