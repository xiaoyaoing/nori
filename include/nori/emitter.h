/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/object.h>
#include "mesh.h"
#include "bsdf.h"

NORI_NAMESPACE_BEGIN


    struct emitterRecord{
        Vector3f wi;
        Vector3f  normal;
        Vector3f  pos;
        float pdfVal;
        Emitter * emi;
        Intersection * its;
    };

    struct Photon{  //光子
        Vector3f position;
        Vector3f  direction;
        Color3f   power;
        int idx=0;
        Photon(Vector3f position,Vector3f direction,Color3f power)
        {
            this->position=position;
            this->direction=direction;
            this->power=power;
        }

        Photon(){}



    };


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

    Mesh * getShape(){
        return mesh;
    }

    void setMesh(Mesh * mesh){
        this->mesh=mesh;
    }


    //sample ray from light
    virtual Ray3f  sampleRay(Sampler * sampler,Color3f & power){

    }

    virtual Photon samplePhoton(Sampler *pSampler) {
           throw NoriException("Abstract sample Photon is not yet implemented!");
    }

    protected:
    Mesh * mesh;
    };


//struct EmitterQueryRecord{struct Photon{
//
//};
//
//    Vector3f wi;
//
//};

NORI_NAMESPACE_END
