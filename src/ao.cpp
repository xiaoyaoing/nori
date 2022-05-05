//
// Created by 打工人 on 2022/4/29.
//
//
// Created by 打工人 on 2022/4/28.
//

#include <nori/integrator.h>
#include <nori/scene.h>
#include "../include/nori/sampler.h"
#include "../include/nori/warp.h"
NORI_NAMESPACE_BEGIN

    class AoIntegrator : public Integrator {

    public:
        AoIntegrator(const PropertyList &props) {
//            this->energy=props.getColor("energy");
//            this->pos=props.getPoint("position");
//            /* No parameters this time */
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,int depth=0) const {
            /* Find the surface that is visible in the requested direction */
            Intersection its;
            if (!scene->rayIntersect(ray, its))
                return Color3f(0.0f);
            Color3f color;
            int nSample=10;


            auto normal=its.shFrame.n.cwiseAbs();
            Frame frame(normal);

            for(int i=0;i<nSample;i++){
               Point2f p= sampler->next2D();
               auto cosHem= Warp::squareToCosineHemisphere(p);
               auto worldShadowDir=frame.toWorld(cosHem).normalized();
               Ray3f shadowRay(its.p,worldShadowDir);
               if(scene->rayIntersect(shadowRay))
                   continue;
               color+= normal.dot(worldShadowDir) * INV_PI/Warp::squareToCosineHemispherePdf(cosHem);
            }
            return  color/nSample;
        }

        std::string toString() const {
            return "SimpleIntegrator[]";
        }
    private:

    };
    NORI_REGISTER_CLASS(AoIntegrator, "ao");
NORI_NAMESPACE_END
