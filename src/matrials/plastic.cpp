//
// Created by 打工人 on 2022/6/12.
//

/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include "nori/bsdf.h"
#include "nori/frame.h"
#include "nori/warp.h"

NORI_NAMESPACE_BEGIN

/// Ideal Plastic BRDF
class Plastic : public BSDF {
public:
    Plastic(const PropertyList &propList) {
        m_albedo = propList.getColor("albedo", Color3f(0.5f));
    }

        Color3f eval(const BSDFQueryRecord & bRec) const {
            /* Discrete BRDFs always evaluate to zero in Nori */
            if(drand48()<0.5){
                if (bRec.measure != ESolidAngle
                    || Frame::cosTheta(bRec.wi) <= 0
                    || Frame::cosTheta(bRec.wo) <= 0)
                    return Color3f(0.0f);

                /* The BRDF is simply the albedo / pi */
                return m_albedo * INV_PI;
            }
            else {
                return {0.0} ;
            }
        }

        float pdf(const BSDFQueryRecord & bRec) const {
            if (bRec.measure != ESolidAngle
                || Frame::cosTheta(bRec.wi) <= 0
                || Frame::cosTheta(bRec.wo) <= 0)
                return 0.0f;

            return 0.5f* INV_PI * Frame::cosTheta(bRec.wo);
        }

        Color3f sample(BSDFQueryRecord &bRec, const Point2f & sample) const {

           if(sample.x()<0.5)
           {
               if (Frame::cosTheta(bRec.wi) <= 0)
                   return Color3f(0.0f);
               bRec.measure = ESolidAngle;
               bRec.wo = Warp::squareToCosineHemisphere(sample);
               bRec.eta = 1.0f;

               return m_albedo;
           }
           else {
               if (Frame::cosTheta(bRec.wi) <= 0)
                   return Color3f(0.0f);

               // Reflection in local coordinates
               bRec.wo = Vector3f(
                       -bRec.wi.x(),
                       -bRec.wi.y(),
                       bRec.wi.z()
               );
               bRec.measure = EDiscrete;

               /* Relative index of refraction: no change */
               bRec.eta = 1.0f;

               return Color3f(1.0f);
           }
        }


        bool isDiffuse() const {
            return true;
        }

        EClassType getClassType() const { return EBSDF; }

        std::string toString() const override {
            return std::string();
        }

    private:
        Color3f m_albedo;
    };

NORI_REGISTER_CLASS(Plastic,"plastic")
NORI_NAMESPACE_END
