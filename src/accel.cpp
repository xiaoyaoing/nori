/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/accel.h>
#include <Eigen/Geometry>

NORI_NAMESPACE_BEGIN

    int searchCount=0;
    void Accel::addMesh(Mesh *mesh) {
        m_MeshSet->addMesh(mesh);
        m_bbox.expandBy(mesh->getBoundingBox());
    }

    void Accel::build() {
        searchCount=0;
        std::vector<uint32_t> indexBuf;
        for (uint32_t i = (uint32_t) 0; i < m_MeshSet->getSize(); ++i)
            indexBuf.emplace_back(i);
        std::cout << "Start build OcTree" << std::endl;
        m_root = buildOcTree(m_bbox, indexBuf);
        std::cout << "End build OcTree" << std::endl;
        /* Nothing to do here for now */

    }

    bool Accel::rayIntersect(const Ray3f &ray_, Intersection &its, bool shadowRay) const {
//    m_root->rayTraversal(ray_,its,shadowRay,m_mesh);
        searchCount=0;

        Ray3f ray(ray_); /// Make a copy of the ray (we will need to update its '.maxt' value)

        bool foundIntersection=false;
        /* Brute force search through all triangles */
//    for (uint32_t idx = 0; idx < m_mesh->getTriangleCount(); ++idx) {
//        float u, v, t;
//        if (m_mesh->rayIntersect(idx, ray, u, v, t)) {
//            /* An intersection was found! Can terminate
//               immediately if this is a shadow ray query */
//            if (shadowRay)
//                return true;
//            ray.maxt = its.t = t;
//            its.uv = Point2f(u, v);
//            its.mesh = m_mesh;
//            f = idx;
//            foundIntersection = true;
//        }
//    }
        foundIntersection = m_root->rayTraversal(ray, its, shadowRay, m_MeshSet);
        auto  f=m_MeshSet->getTri(its.f).second;
        if (shadowRay && foundIntersection)  return true;
        if (foundIntersection) {
            /* At this point, we now know that there is an intersection,
               and we know the triangle index of the closest such intersection.

               The following computes a number of additional properties which
               characterize the intersection (normals, texture coordinates, etc..)
            */
            /* Find the barycentric coordinates */
            Vector3f bary;
            bary << 1-its.uv.sum(), its.uv;
            /* References to all relevant mesh buffers */
            const Mesh *mesh   = its.mesh;
            const MatrixXf &V  = mesh->getVertexPositions();
            const MatrixXf &N  = mesh->getVertexNormals();
            const MatrixXf &UV = mesh->getVertexTexCoords();
            const MatrixXu &F  = mesh->getIndices();

            /* Vertex indices of the triangle */
            uint32_t idx0 = F(0, f), idx1 = F(1, f), idx2 = F(2, f);

            Point3f p0 = V.col(idx0), p1 = V.col(idx1), p2 = V.col(idx2);

            /* Compute the intersection positon accurately
               using barycentric coordinates */
            its.p = bary.x() * p0 + bary.y() * p1 + bary.z() * p2;

            /* Compute proper texture coordinates if provided by the mesh */
            if (UV.size() > 0)
                its.uv = bary.x() * UV.col(idx0) +
                         bary.y() * UV.col(idx1) +
                         bary.z() * UV.col(idx2);

            /* Compute the geometry frame */
            its.geoFrame = Frame((p1-p0).cross(p2-p0).normalized());

            if (N.size() > 0) {
                /* Compute the shading frame. Note that for simplicity,
                   the current implementation doesn't attempt to provide
                   tangents that are continuous across the surface. That
                   means that this code will need to be modified to be able
                   use anisotropic BRDFs, which need tangent continuity */

                its.shFrame = Frame(
                        (bary.x() * N.col(idx0) +
                         bary.y() * N.col(idx1) +
                         bary.z() * N.col(idx2)).normalized());
            } else {
                its.shFrame = its.geoFrame;
            }
        }
        return foundIntersection;
    }
    std::vector<BoundingBox3f> OcNode::getSubBBoxes() {
        Point3f center {nodeBox.getCenter()};
        Vector3f halfEdge {center - nodeBox.min};

        std::vector<BoundingBox3f> subBoxes;

        subBoxes.emplace_back(
                BoundingBox3f {
                        nodeBox.min,
                        center
                }
        );
        subBoxes.emplace_back(
                BoundingBox3f {
                        nodeBox.min + Vector3f{halfEdge[0], .0f, .0f},
                        center + Vector3f{halfEdge[0], .0f, .0f}
                }
        );
        subBoxes.emplace_back(
                BoundingBox3f {
                        nodeBox.min + Vector3f{halfEdge[0], halfEdge[1], .0f},
                        center + Vector3f{halfEdge[0], halfEdge[1], .0f}
                }
        );
        subBoxes.emplace_back(
                BoundingBox3f {
                        nodeBox.min + Vector3f{.0f, halfEdge[1], .0f},
                        center + Vector3f{.0f, halfEdge[1], .0f}
                }
        );
        subBoxes.emplace_back(
                BoundingBox3f {
                        nodeBox.min + Vector3f{.0f, .0f, halfEdge[2]},
                        center + Vector3f{.0f, .0f, halfEdge[2]}
                }
        );
        subBoxes.emplace_back(
                BoundingBox3f {
                        nodeBox.min + Vector3f{halfEdge[0], .0f, halfEdge[2]},
                        center + Vector3f{halfEdge[0], .0f, halfEdge[2]}
                }
        );
        subBoxes.emplace_back(
                BoundingBox3f {
                        nodeBox.min + Vector3f{halfEdge[0], halfEdge[1], halfEdge[2]},
                        center + Vector3f{halfEdge[0], halfEdge[1], halfEdge[2]}
                }
        );
        subBoxes.emplace_back(
                BoundingBox3f {
                        nodeBox.min + Vector3f{.0f, halfEdge[1], halfEdge[2]},
                        center + Vector3f{.0f, halfEdge[1], halfEdge[2]}
                }
        );
        return subBoxes;
    }

    bool OcNode::rayTraversal( Ray3f &ray_, Intersection &its, bool shadowRay, const MeshSet *mesh) const {

        if(!nodeBox.rayIntersect(ray_))
            return false;
        searchCount++;
        if(indexBufPtr){
            bool foundIntersetcion= false;
            for (const auto &index : *indexBufPtr) {
                float u, v, t;
                if (mesh->rayIntersect(index, ray_, u, v, t)) {
                    if (shadowRay)
                        return true;
                    ray_.maxt = its.t = t;
                    its.uv = Point2f(u, v);
                    its.mesh = mesh->getMesh(index);
                    its.f=index;
                    foundIntersetcion= true;
//                    its.f = mesh.getTri(index).first;
                }
            }

            return foundIntersetcion;
        }
        else{
            bool foundIntersection = false;
            // recursive
            for (auto subNode : subNodes) {
                if (subNode) {
                    if (subNode->rayTraversal(ray_, its, shadowRay, mesh))
                        foundIntersection = true;
                }
            }
            return foundIntersection;
        }

        return false;
    }

    OcNode* Accel::buildOcTree(const BoundingBox3f &box, const std::vector<uint32_t> &indexBuf) {
        // no triangles
        if (indexBuf.size() == 0)
            return nullptr;

        // only a few triangles
        if (indexBuf.size() < ocLeafMaxSize) {
            OcNode *node = new OcNode {box, indexBuf};
            return node;
        }
        // split the current node
        std::vector<uint32_t> triangle_list[nSubs];
        OcNode *node = new OcNode {box};
        const auto& subBoxes = node->getSubBBoxes();

        for (const auto &index : indexBuf) {
            //const auto &bbox = m_mesh->getBBoxByTriIndex(index);
            const auto &bbox = m_MeshSet->getBoundingBox(index);
            for (int i = 0; i < nSubs; ++i) {
                // if triangle overlaps the sub-node i
                // add the triangle index into the list[i]
                if (bbox.overlaps(subBoxes[i])) {
                    // overlaps
                    triangle_list[i].emplace_back(index);
                }
                //if (m_mesh->isInBox(index, bbox)) {
                //    triangle_list[i].emplace_back(index);
                //}
            }
        }

        for (int i = 0; i < nSubs; ++i) {
            if(triangle_list[i].size()==indexBuf.size())
            {node = new OcNode {box, indexBuf};
            return node;}
            node->subNodes[i] = buildOcTree(subBoxes[i], triangle_list[i]);
        }
        return node;
    }

NORI_NAMESPACE_END

