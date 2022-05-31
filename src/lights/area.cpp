//
// Created by 打工人 on 2022/4/29.
//


#include "nori/emitter.h"
#include "nori/warp.h"
NORI_NAMESPACE_BEGIN
class AreaLight : public Emitter{

public:
    std::string toString() const override {
        return "AreaLight";
    }



    Color3f eval(emitterRecord eRec) const override {
          return radiance;
//        return Emitter::eval(intersection);
    }

    Color3f eval(Intersection its)const override{
        return radiance;
    }
    AreaLight(const PropertyList list) {
        this->radiance=list.getColor("radiance");
    }


    Photon samplePhoton(Sampler * sampler) override {
         emitterRecord eRec;
         mesh->sample(eRec,sampler,1.f);

//         while(Warp::squareToUniformSphere(sampler->next2D()). )
        auto dir= Frame(eRec.normal).toWorld(Warp::squareToUniformSphere(sampler->next2D())).normalized();

        Photon photon(eRec.pos,
                       dir,radiance * eRec.normal.dot(dir));


        return photon;
//        auto
//        return Emitter::samplePhoton(pSampler);
    }



//    float pdf(BSDFQueryRecord record, float d) override {
//
//    }

private:
    Color3f radiance;
};

NORI_REGISTER_CLASS(AreaLight, "area");


NORI_NAMESPACE_END