/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include "nori/bsdf.h"
#include "nori/frame.h"

NORI_NAMESPACE_BEGIN

/// Ideal dielectric BSDF
class Dielectric : public BSDF {
public:
    Dielectric(const PropertyList &propList) {
        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);
    }

    Color3f eval(const BSDFQueryRecord &) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return Color3f(0.0f);
    }

    float pdf(const BSDFQueryRecord &) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return 0.0f;
    }

    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
       // throw NoriException("Unimplemented!");
        float cosThetaI = Frame::cosTheta(bRec.wi);
        float fresnelVal = fresnel(cosThetaI, m_extIOR, m_intIOR);
        auto cosTheta=Frame::cosTheta(bRec.wi);
        auto sinTheta=Frame::sinTheta(bRec.wi);

        auto eta = cosThetaI > 0 ? m_extIOR / m_intIOR : m_intIOR / m_extIOR;
        bool canRefract=(sinTheta*eta)<=1;
        if(fresnelVal>sample[0]){
               bRec.wo= reflect(bRec.wi);
               bRec.eta=1.f;
               return {1.f};
        }
        else{
            bRec.wo= refract(bRec.wi);
            return {1.f};
        }

//        if (sample[0] < fresnelVal) {
//            // reflect
//            bRec.wo = reflect(bRec.wi);
//            bRec.eta = 1.f;
//            return Color3f(1.f / fresnelVal);
//        } else {
//            // refract
//            bRec.wo = refract(bRec.wi);
//            // bRec.eta =
//            return Color3f(1.f / (1 - fresnelVal));
//        }
    }

    std::string toString() const {
        return tfm::format(
            "Dielectric[\n"
            "  intIOR = %f,\n"
            "  extIOR = %f\n"
            "]",
            m_intIOR, m_extIOR);
    }
private:
    static Vector3f reflect(const Vector3f &wi) {
        return Vector3f {
                -wi[0], -wi[1], wi[2]
        };
    }

    Vector3f refract(const Vector3f &wi) const {
        float cosThetaI = Frame::cosTheta(wi),
                eta = cosThetaI > 0 ? m_extIOR / m_intIOR : m_intIOR / m_extIOR,
                cosThetaT = std::sqrt(
                1 - eta*eta*(1-cosThetaI*cosThetaI)
        );
        return Vector3f {
                - wi[0] * eta,
                - wi[1] * eta,
                cosThetaI > 0 ? -cosThetaT : cosThetaT
        };
    }
    float m_intIOR, m_extIOR;
};

NORI_REGISTER_CLASS(Dielectric, "dielectric");
NORI_NAMESPACE_END
