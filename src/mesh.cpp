/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/mesh.h>
#include <nori/bbox.h>
#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <nori/warp.h>
#include <Eigen/Geometry>

NORI_NAMESPACE_BEGIN

Mesh::Mesh() { }

Mesh::~Mesh() {
    delete m_bsdf;
    delete m_emitter;
}

void Mesh::activate() {
    if (!m_bsdf) {
        /* If no material was assigned, instantiate a diffuse BRDF */
        m_bsdf = static_cast<BSDF *>(
            NoriObjectFactory::createInstance("diffuse", PropertyList()));
    }
    if(this->isEmitter())
    {
    for(size_t i=0;i<getTriangleCount();i++){
        dPdf.append(this->surfaceArea(i));
        }
    this->allSurfaceArea=dPdf.normalize();}
    if(this->isEmitter())
    std::cout<<"allSurfaceArea"<<allSurfaceArea<<endl;
}

float Mesh::surfaceArea(uint32_t index) const {
    uint32_t i0 = m_F(0, index), i1 = m_F(1, index), i2 = m_F(2, index);

    const Point3f p0 = m_V.col(i0), p1 = m_V.col(i1), p2 = m_V.col(i2);

    return 0.5f * Vector3f((p1 - p0).cross(p2 - p0)).norm();
}

bool Mesh::rayIntersect(uint32_t index, const Ray3f &ray, float &u, float &v, float &t) const {
    uint32_t i0 = m_F(0, index), i1 = m_F(1, index), i2 = m_F(2, index);
    const Point3f p0 = m_V.col(i0), p1 = m_V.col(i1), p2 = m_V.col(i2);

    /* Find vectors for two edges sharing v[0] */
    Vector3f edge1 = p1 - p0, edge2 = p2 - p0;

    /* Begin calculating determinant - also used to calculate U parameter */
    Vector3f pvec = ray.d.cross(edge2);

    /* If determinant is near zero, ray lies in plane of triangle */
    float det = edge1.dot(pvec);

    if (det > -1e-8f && det < 1e-8f)
        return false;
    float inv_det = 1.0f / det;

    /* Calculate distance from v[0] to ray origin */
    Vector3f tvec = ray.o - p0;

    /* Calculate U parameter and test bounds */
    u = tvec.dot(pvec) * inv_det;
    if (u < 0.0 || u > 1.0)
        return false;

    /* Prepare to test V parameter */
    Vector3f qvec = tvec.cross(edge1);

    /* Calculate V parameter and test bounds */
    v = ray.d.dot(qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0)
        return false;

    /* Ray intersects triangle -> compute t */
    t = edge2.dot(qvec) * inv_det;

    return t >= ray.mint && t <= ray.maxt;
}

BoundingBox3f Mesh::getBoundingBox(uint32_t index) const {
    BoundingBox3f result(m_V.col(m_F(0, index)));
    result.expandBy(m_V.col(m_F(1, index)));
    result.expandBy(m_V.col(m_F(2, index)));
    return result;
}

Point3f Mesh::getCentroid(uint32_t index) const {
    return (1.0f / 3.0f) *
        (m_V.col(m_F(0, index)) +
         m_V.col(m_F(1, index)) +
         m_V.col(m_F(2, index)));
}

void Mesh::addChild(NoriObject *obj) {
    switch (obj->getClassType()) {
        case EBSDF:
            if (m_bsdf)
                throw NoriException(
                    "Mesh: tried to register multiple BSDF instances!");
            m_bsdf = static_cast<BSDF *>(obj);
            break;

        case EEmitter: {
                Emitter *emitter = static_cast<Emitter *>(obj);
                if (m_emitter)
                    throw NoriException(
                        "Mesh: tried to register multiple Emitter instances!");
                m_emitter = emitter;
            }
            break;

        default:
            throw NoriException("Mesh::addChild(<%s>) is not supported!",
                                classTypeName(obj->getClassType()));
    }
}

std::string Mesh::toString() const {
    return tfm::format(
        "Mesh[\n"
        "  name = \"%s\",\n"
        "  vertexCount = %i,\n"
        "  triangleCount = %i,\n"
        "  bsdf = %s,\n"
        "  emitter = %s\n"
        "]",
        m_name,
        m_V.cols(),
        m_F.cols(),
        m_bsdf ? indent(m_bsdf->toString()) : std::string("null"),
        m_emitter ? indent(m_emitter->toString()) : std::string("null")
    );
}

    void Mesh::sample(emitterRecord & rec, Sampler * sampler,float weight) {
        uint32_t index=dPdf.sample(sampler->next1D());
        uint32_t i0 = m_F(0, index), i1 = m_F(1, index), i2 = m_F(2, index);
        const Point3f p0 = m_V.col(i0), p1 = m_V.col(i1), p2 = m_V.col(i2);

        auto bary=sampler->next2D();
        float u=1- sqrt(1-bary.x()),v=bary.y() * sqrt(1-bary.x());
        rec.pos=p0*(1-u-v) + p1 * u + p2 * v;

        if(m_N.size()>0){
            rec.normal=(1-u-v) * m_N.col(i0) + u* m_N.col(i1)+v* m_N.col(i2);

        }
        else{
            rec.normal=((p1-p0).cross(p2-p0)).normalized();
        }
//
//        BSDFQueryRecord bRec{rec.wi};
//        bRec.wo =rec.its->shFrame.toLocal((rec.pos-rec.its->p).normalized());
//        bRec.measure = ESolidAngle;

//        rec.pdfVal= getEmitter()->pdf(bRec,weight);

        rec.emi=m_emitter;
    }

    std::string Intersection::toString() const {
    if (!mesh)
        return "Intersection[invalid]";

    return tfm::format(
        "Intersection[\n"
        "  p = %s,\n"
        "  t = %f,\n"
        "  uv = %s,\n"
        "  shFrame = %s,\n"
        "  geoFrame = %s,\n"
        "  mesh = %s\n"
        "]",
        p.toString(),
        t,
        uv.toString(),
        indent(shFrame.toString()),
        indent(geoFrame.toString()),
        mesh ? mesh->toString() : std::string("null")
    );
}

bool MeshSet::rayIntersect(uint32_t index, const nori::Ray3f &ray, float &u, float &v, float &t) const {
    auto tri= getTri(index);
    return meshesPtr->operator[](tri.first)->rayIntersect(tri.second,ray,u,v,t);
}


BoundingBox3f MeshSet::getBoundingBox(uint32_t idx) const {
    auto tri= getTri(idx);
    return meshesPtr->operator[](tri.first)->getBoundingBox(tri.second);
}

    void MeshSet::addMesh(Mesh *mesh) {
        {
            meshesPtr->emplace_back(mesh);
            if (count.empty())
                count.emplace_back(mesh->getTriangleCount());
            else count.emplace_back(mesh->getTriangleCount() + count.back());
            if(mesh->isEmitter())
            {
                mesh->getEmitter()->setMesh(mesh);
                emitterPtr->emplace_back(mesh);
                emitterPdf.append(mesh->getAllSurfaceArea());
            }
        }
    }

void MeshSet::sampleLight(emitterRecord & eRec,Sampler * sampler) const {
    float pdfVal1;
    int idx=emitterPdf.sample(sampler->next1D(),pdfVal1);
    eRec.pdfVal=1/this->allSurfaceArea;
    (*emitterPtr)[idx]->sample(eRec,sampler,pdfVal1);

}

void MeshSet::EmitterNormalize() {
   this->allSurfaceArea= this->emitterPdf.normalize();
    }


NORI_NAMESPACE_END

