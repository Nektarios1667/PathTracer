#pragma once
#include "AABB.h"
#include "Hittable.h"
#include "BVHStats.h"
#include <memory>
#include <vector>

struct BVHNode {
    static std::vector<std::unique_ptr<Hittable>> scene;
    AABB bounds;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    Hittable* object = nullptr; // For leaf

    BVHNode(vector<Hittable*>& objects, size_t start, size_t end);

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }

    AABB getBoundingBox(const vector<Hittable*>& objects, size_t start, size_t end) const;
    auto compareAlongLongestAxis(const AABB& bounds) const;
    static void getNodeDebugInfo(const BVHNode* node, int currentDepth, BVHStats& stats) {
        if (!node) return;

        stats.totalNodes++;
        if (currentDepth > stats.maxDepth) stats.maxDepth = currentDepth;

        if (node->isLeaf()) {
            stats.totalLeafNodes++;
            return;
        }

        getNodeDebugInfo(node->left.get(), currentDepth + 1, stats);
        getNodeDebugInfo(node->right.get(), currentDepth + 1, stats);
    }
};