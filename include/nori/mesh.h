/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/object.h>
#include <nori/frame.h>
#include <nori/bbox.h>
#include "dpdf.h"
#include "emitter.h"

NORI_NAMESPACE_BEGIN

/**
 * \brief Intersection data structure
 *
 * This data structure records local information about a ray-triangle intersection.
 * This includes the position, traveled ray distance, uv coordinates, as well
 * as well as two local coordinate frames (one that corresponds to the true
 * geometry, and one that is used for shading computations).
 */
struct Intersection {
    /// Position of the surface intersection
    Point3f p;
    /// Unoccluded distance along the ray
    float t;
    /// UV coordinates, if any
    Point2f uv;
    /// Shading frame (based on the shading normal)
    Frame shFrame;
    /// Geometric frame (based on the true geometry)
    Frame geoFrame;
    /// Pointer to the associated mesh
    const Mesh *mesh;

    /// Create an uninitialized intersection record
    Intersection() : mesh(nullptr) { }

    /// Transform a direction vector into the local shading frame
    Vector3f toLocal(const Vector3f &d) const {
        return shFrame.toLocal(d);
    }

    /// Transform a direction vector from local to world coordinates
    Vector3f toWorld(const Vector3f &d) const {
        return shFrame.toWorld(d);
    }

    /// Return a human-readable summary of the intersection record
    std::string toString() const;

    uint32_t f;
};

/**
 * \brief Triangle mesh
 *
 * This class stores a triangle mesh object and provides numerous functions
 * for querying the individual triangles. Subclasses of \c Mesh implement
 * the specifics of how to create its contents (e.g. by loading from an
 * external file)
 */
class Mesh : public NoriObject {
public:

    void sample(emitterRecord & rec, Sampler * sampler,float weight);

    /// Release all memory
    virtual ~Mesh();

    /// Initialize internal data structures (called once by the XML parser)
    virtual void activate();

    /// Return the total number of triangles in this shape
    uint32_t getTriangleCount() const { return (uint32_t) m_F.cols(); }

    /// Return the total number of vertices in this shape
    uint32_t getVertexCount() const { return (uint32_t) m_V.cols(); }

    /// Return the surface area of the given triangle
    float surfaceArea(uint32_t index) const;

    //// Return an axis-aligned bounding box of the entire mesh
    const BoundingBox3f &getBoundingBox() const { return m_bbox; }

    //// Return an axis-aligned bounding box containing the given triangle
    BoundingBox3f getBoundingBox(uint32_t index) const;

    //// Return the centroid of the given triangle
    Point3f getCentroid(uint32_t index) const;

    /** \brief Ray-triangle intersection test
     *
     * Uses the algorithm by Moeller and Trumbore discussed at
     * <tt>http://www.acm.org/jgt/papers/MollerTrumbore97/code.html</tt>.
     *
     * Note that the test only applies to a single triangle in the mesh.
     * An acceleration data structure like \ref BVH is needed to search
     * for intersections against many triangles.
     *
     * \param index
     *    Index of the triangle that should be intersected
     * \param ray
     *    The ray segment to be used for the intersection query
     * \param t
     *    Upon success, \a t contains the distance from the ray origin to the
     *    intersection point,
     * \param u
     *   Upon success, \c u will contain the 'U' component of the intersection
     *   in barycentric coordinates
     * \param v
     *   Upon success, \c v will contain the 'V' component of the intersection
     *   in barycentric coordinates
     * \return
     *   \c true if an intersection has been detected
     */
    bool rayIntersect(uint32_t index, const Ray3f &ray, float &u, float &v, float &t) const;

    /// Return a pointer to the vertex positions
    const MatrixXf &getVertexPositions() const { return m_V; }

    /// Return a pointer to the vertex normals (or \c nullptr if there are none)
    const MatrixXf &getVertexNormals() const { return m_N; }

    /// Return a pointer to the texture coordinates (or \c nullptr if there are none)
    const MatrixXf &getVertexTexCoords() const { return m_UV; }

    /// Return a pointer to the triangle vertex index list
    const MatrixXu &getIndices() const { return m_F; }

    /// Is this mesh an area emitter?
    bool isEmitter() const { return m_emitter != nullptr; }

    /// Return a pointer to an attached area emitter instance
    Emitter *getEmitter() { return m_emitter; }

    /// Return a pointer to an attached area emitter instance (const version)
    const Emitter *getEmitter() const { return m_emitter; }

    /// Return a pointer to the BSDF associated with this mesh
    const BSDF *getBSDF() const { return m_bsdf; }

    /// Register a child object (e.g. a BSDF) with the mesh
    virtual void addChild(NoriObject *child);

    /// Return the name of this mesh
    const std::string &getName() const { return m_name; }

    /// Return a human-readable summary of this instance
    std::string toString() const;

    /**
     * \brief Return the type of object (i.e. Mesh/BSDF/etc.)
     * provided by this instance
     * */
    EClassType getClassType() const { return EMesh; }

    float getAllSurfaceArea() const {
        return allSurfaceArea;
    }

protected:
    /// Create an empty mesh
    Mesh();

protected:
    DiscretePDF  dPdf;                   ///Assignement 4.1
    float        allSurfaceArea;         /// surfaceArea
    std::string m_name;                  ///< Identifying name
    MatrixXf      m_V;                   ///< Vertex positions
    MatrixXf      m_N;                   ///< Vertex normals
    MatrixXf      m_UV;                  ///< Vertex texture coordinates
    MatrixXu      m_F;                   ///< Faces
    BSDF         *m_bsdf = nullptr;      ///< BSDF of the surface
    Emitter    *m_emitter = nullptr;     ///< Associated emitter, if any
    BoundingBox3f m_bbox;                ///< Bounding box of the mesh
};

    struct MeshSet {
        std::unique_ptr<std::vector<Mesh *>> meshesPtr {nullptr};
        std::unique_ptr<std::vector<Mesh *>> emitterPtr {nullptr};
        std::vector<uint32_t> count;
        DiscretePDF emitterPdf;

        MeshSet(): meshesPtr(std::make_unique<std::vector<Mesh *>>()),
                   emitterPtr(std::make_unique<std::vector<Mesh *>>())
        { }

        //MeshSet(std::vector<Mesh> &_meshes) {
        //    meshesPtr = std::make_unique<std::vector<Mesh>>(_meshes);
        //    for (const auto &mesh : *meshesPtr) {
        //        if (count.empty())
        //            count.emplace_back(mesh.getTriangleCount());
        //        else count.emplace_back(mesh.getTriangleCount() + count.back());
        //    }
        //}

        void addMesh(Mesh *mesh) ;

        uint32_t getSize() const {
            return count.back();
        }

        decltype(auto) getTri(uint32_t idx) const {
            auto cmp = [&idx](uint32_t f) {
                return f > idx;
            };
            const auto &entry = std::find_if(count.begin(), count.end(), cmp);
            uint32_t meshIdx = entry - count.begin();
            uint32_t triIdx = idx;
            if (meshIdx > 0)
                triIdx -= count[meshIdx - 1];
            return std::pair<uint32_t, uint32_t> {meshIdx, triIdx};
        }

        Mesh* getMesh(uint32_t idx) const {
            uint32_t meshIdx = getTri(idx).first;
            return (*meshesPtr)[meshIdx];
        }

        BoundingBox3f getBoundingBox(uint32_t idx) const;

        bool rayIntersect (uint32_t index, const Ray3f &ray, float &u, float &v, float &t) const;

        bool isInBox (uint32_t idx, const BoundingBox3f &box) const;

        void sampleLight(emitterRecord &eRec,Sampler * sampler) const;
    };

NORI_NAMESPACE_END
