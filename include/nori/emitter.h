/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/object.h>
#include "mesh.h"

NORI_NAMESPACE_BEGIN

/**
 * \brief Superclass of all emitters
 */
class Emitter : public NoriObject {
public:

    /**
     * \brief Return the type of object (i.e. Mesh/Emitter/etc.) 
     * provided by this instance
     * */
    EClassType getClassType() const { return EEmitter; }

    virtual Color3f eval(emitterRecord eRec) const=0;
    virtual Color3f eval(Intersection its) const=0;
    void setMesh(Mesh * mesh){
        this->mesh=mesh;
    }

protected:
    Mesh * mesh;
};

struct emitterRecord{
    Vector3f  normal;
    Vector3f  pos;
    float pdfVal;
    Emitter * emi;
};

NORI_NAMESPACE_END
