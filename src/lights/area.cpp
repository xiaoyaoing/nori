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


    Ray3f sampleRay(Sampler *sampler, Color3f &power) override {
        emitterRecord eRec;
        mesh->sample(eRec,sampler,1.f);
        float lightPosPdf=eRec.pdfVal;

        Vector3f  localDir=Warp::squareToCosineHemisphere(sampler->next2D());
        Vector3f  dir= Frame(eRec.normal).toWorld(localDir).normalized();
        float lightDirPdf=Warp::squareToCosineHemispherePdf(localDir);

        float cos =eRec.normal .dot(dir);

        power=radiance*cos/(lightPosPdf*lightDirPdf);
        return {eRec.pos,dir};
    }

    Photon samplePhoton(Sampler * sampler) override {


         emitterRecord eRec;
         mesh->sample(eRec,sampler,1.f);
         float lightPosPdf=eRec.pdfVal;

         Vector3f  dir= Frame(eRec.normal).toWorld(Warp::squareToCosineHemisphere(sampler->next2D())).normalized();
         float lightDirPdf=Warp::squareToCosineHemispherePdf(dir);

         float cos =eRec.normal .dot(dir);

         Photon photon(eRec.pos,
                       dir,abs(radiance
                       * cos / (lightPosPdf * lightDirPdf)
                       ));

        return photon;

    }



//    float pdf(BSDFQueryRecord record, float d) override {
//
//    }

private:
    Color3f radiance;
};

NORI_REGISTER_CLASS(AreaLight, "area");


NORI_NAMESPACE_END