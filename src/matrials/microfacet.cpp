/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include "nori/bsdf.h"
#include "nori/frame.h"
#include "nori/warp.h"

NORI_NAMESPACE_BEGIN

static void  reflect(const Vector3f & wi,const Vector3f & n,Vector3f & wo){
        wo=wi-2*wi.dot(n)*n;
}

class Microfacet : public BSDF {
public:
    Microfacet(const PropertyList &propList) {
        name=propList.getString("mtlname","");

        /* RMS surface roughness */
        m_alpha = propList.getFloat("alpha", 0.1f);

        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);

        /* Albedo of the diffuse base material (a.k.a "kd") */
        m_kd = propList.getColor("kd", Color3f(0.5f));

        /* To ensure energy conservation, we must scale the 
           specular component by 1-kd. 

           While that is not a particularly realistic model of what 
           happens in reality, this will greatly simplify the 
           implementation. Please see the course staff if you're 
           interested in implementing a more realistic version 
           of this BRDF. */
        m_ks = 1 - m_kd.maxCoeff();
    }

    /// Evaluate the BRDF for the given pair of directions
    Color3f eval(const BSDFQueryRecord &bRec) const {

        Color3f kd=m_kd * INV_PI;

        Vector3f wi=bRec.wi;
        Vector3f wo=bRec.wo;
        Vector3f wh=(wi+wo).normalized();

        float F= fresnel(wh.dot(wi),m_extIOR,m_intIOR);

        auto G_1=[&](Vector3f & wv,Vector3f & wh){
            if(wv.dot(wh)/Frame::cosTheta(wv)<=0)
                return 0.f;
            float b=1.f/(m_alpha * Frame::tanTheta(wv));
            if(b>=1.6){
                return 1.f;
            }
            else
            {
              auto mol=3.535f*b+2.181f*b*b;
              auto den=1+2.276f*b+2.577f*b*b;
              return float(mol/den);
            }
        };
        auto G=G_1(wi,wh)*G_1(wo,wh);

        auto D=Warp::squareToBeckmannPdf(wh,m_alpha);

        Color3f ks=m_ks * D * F * G /
                (4 * Frame::cosTheta(wi) * Frame::cosTheta(wo) * Frame::cosTheta(wh));

        return kd + ks;


    }

    /// Evaluate the sampling density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {
        if (bRec.measure != ESolidAngle
            || Frame::cosTheta(bRec.wi) <= 0.0f
            || Frame::cosTheta(bRec.wo) <= 0.0f)
            return 0.0f;
        Vector3f wh = (bRec.wi + bRec.wo); wh.normalize();
        float jacobian = 1.f/(4*(wh.dot(bRec.wo)));
        float a = m_ks*Warp::squareToBeckmannPdf(wh, m_alpha) * jacobian;
        float b = (1 - m_ks) * bRec.wo.z() * INV_PI;
        return a + b;
//    	throw NoriException("MicrofacetBRDF::pdf(): not implemented!");
    }

    /// Sample the BRDF
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &_sample) const {
        if(_sample.x()<m_ks){
            ///specular case
            const Vector3f n = Warp::squareToBeckmann(Point2f((float)drand48(), (float)drand48()), m_alpha);
            Vector3f inDir=-bRec.wi;
            reflect(inDir, n, bRec.wo);
            bRec.measure = ESolidAngle;
        }
        else{
            ///diffuse case
            bRec.wo=Warp::squareToCosineHemisphere(Point2f((float)drand48(),(float)drand48()));
            bRec.measure=ESolidAngle;
        }
        Color3f m_eval = eval(bRec);
        if(m_eval.isZero())
            return {0.f};
        float pdfVal= pdf(bRec);
        if(pdfVal==0.f)
            return {0.f};
        return m_eval * Frame::cosTheta(bRec.wo) /pdfVal;
//    	throw NoriException("MicrofacetBRDF::sample(): not implemented!");

        // Note: Once you have implemented the part that computes the scattered
        // direction, the last part of this function should simply return the
        // BRDF value divided by the solid angle density and multiplied by the
        // cosine factor from the reflection equation, i.e.
        // return eval(bRec) * Frame::cosTheta(bRec.wo) / pdf(bRec);
    }

    bool isDiffuse() const {
        /* While microfacet BRDFs are not perfectly diffuse, they can be
           handled by sampling techniques for diffuse/non-specular materials,
           hence we return true here */
        return true;
    }

    std::string toString() const {
        return tfm::format(
            "Microfacet[\n"
            "  alpha = %f,\n"
            "  intIOR = %f,\n"
            "  extIOR = %f,\n"
            "  kd = %s,\n"
            "  ks = %f\n"
            "]",
            m_alpha,
            m_intIOR,
            m_extIOR,
            m_kd.toString(),
            m_ks
        );
    }
private:
    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks;
    Color3f m_kd;
};

NORI_REGISTER_CLASS(Microfacet, "microfacet");
NORI_NAMESPACE_END
