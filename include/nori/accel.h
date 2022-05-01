/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/mesh.h>

NORI_NAMESPACE_BEGIN



    struct AccelNode {
        virtual ~AccelNode() = default;
        virtual bool rayTraversal(Ray3f &ray_, Intersection &its, bool shadowRay, const MeshSet * meshset) const = 0;
    };

/**
 * @brief
 * The node for an Octree
 */
    constexpr int nSubs = 8;
    constexpr int ocLeafMaxSize = 10;
    struct OcNode : public AccelNode {
        // the cube of the node
        BoundingBox3f nodeBox;

        // alway nullptr when interior
        std::unique_ptr<std::vector<uint32_t>> indexBufPtr {nullptr};

        // alway nullptr when leaf
        OcNode *subNodes[nSubs] {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

        OcNode(const BoundingBox3f &_nodeBox) : nodeBox(_nodeBox) { }

        OcNode(const BoundingBox3f &_nodeBox, const std::vector<uint32_t> &indexBuf)
                :nodeBox(_nodeBox), indexBufPtr(std::make_unique<std::vector<uint32_t>>(indexBuf)) { }

        virtual ~OcNode() {
            for (int i = 0; i < nSubs; ++i)
                delete subNodes[i];
        }

        std::vector<BoundingBox3f> getSubBBoxes();

        virtual bool rayTraversal( Ray3f &ray_, Intersection &its, bool shadowRay, const MeshSet * mesh ) const ;
    };

/**
 * \brief Acceleration data structure for ray intersection queries
 *
 * The current implementation falls back to a brute force loop
 * through the geometry.
 */
class Accel {
public:

    /**
     * \brief Register a triangle mesh for inclusion in the acceleration
     * data structure
     *
     * This function can only be used before \ref build() is called
     */
    void addMesh(Mesh *mesh);

    /// Build the acceleration data structure (currently a no-op)
    void build();

    /// Return an axis-aligned box that bounds the scene
    const BoundingBox3f &getBoundingBox() const { return m_bbox; }

    /**
     * \brief Intersect a ray against all triangles stored in the scene and
     * return detailed intersection information
     *
     * \param ray
     *    A 3-dimensional ray data structure with minimum/maximum extent
     *    information
     *
     * \param its
     *    A detailed intersection record, which will be filled by the
     *    intersection query
     *
     * \param shadowRay
     *    \c true if this is a shadow ray query, i.e. a query that only aims to
     *    find out whether the ray is blocked or not without returning detailed
     *    intersection information.
     *
     * \return \c true if an intersection was found
     */
    bool rayIntersect(const Ray3f &ray, Intersection &its, bool shadowRay) const;

    MeshSet * m_MeshSet;
private:
    ///< Mesh (only a single one for now)
    BoundingBox3f m_bbox;           ///< Bounding box of the entire scene
    OcNode *buildOcTree(const BoundingBox3f &box, const std::vector<uint32_t> &indexBuf);
    OcNode * m_root;
};


class Node{

};
NORI_NAMESPACE_END
