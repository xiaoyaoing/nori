//
// Created by 打工人 on 2022/4/30.
//
//
// Created by 打工人 on 2022/4/28.
//

#include <nori/integrator.h>
#include <nori/scene.h>
#include "../include/nori/bsdf.h"
#include "../include/nori/emitter.h"
#include "../include/nori/sampler.h"
NORI_NAMESPACE_BEGIN

    class WhittedIntegrator : public Integrator {

    public:
        WhittedIntegrator(const PropertyList &props) {

            /* No parameters this time */
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,int depth=0) const {
            /* Find the surface that is visible in the requested direction */
            Intersection its;
            Ray3f _ray(ray);
            if (!scene->rayIntersect(_ray, its))
                return Color3f(0.0f);
            Color3f L;

            if(its.mesh->isEmitter())
             L= its.mesh->getEmitter()->eval(its);

            if(!its.mesh->getBSDF()->isDiffuse()) {
                if(drand48() > RussianRoulette)
                    return {0.f};
                else {
                    BSDFQueryRecord bsdfQ = BSDFQueryRecord(its.toLocal(-ray.d));
                    Color3f albedo = its.mesh->getBSDF()->sample(bsdfQ, Point2f(drand48(), drand48()));
                    return   albedo * Li(scene, sampler, Ray3f(its.p, its.toWorld(bsdfQ.wo)))/ RussianRoulette;
                }
            }

            /***********************************************************/
            // diffuse
            /**********************************************************/
//                if(its.mesh->getBSDF()->isDiffuse())
            {
            emitterRecord eRec;
            scene->sampleEmitter(eRec,sampler);
            Vector3f lightDir=(eRec.pos-its.p);
            float distance=lightDir.norm();
            lightDir.normalize();
            Ray3f  shadowRay(its.p,lightDir,0+Epsilon,distance-Epsilon);
            if(scene->rayIntersect(shadowRay)){
                //do nothing
            }
            else {
                eRec.emi->toString();
                Color3f radiance(eRec.emi->eval(eRec));

                Color3f f=its.mesh->getBSDF()->eval({its.toLocal(-ray.d),its.toLocal(lightDir),ESolidAngle});
                float cosTheta1=abs(its.shFrame.n.cwiseAbs().dot(lightDir));
                float cosTheta2=abs(eRec.normal.dot(-lightDir));
                L+= cosTheta1* cosTheta2 * radiance * f
                        /(distance* distance)
                        /eRec.pdfVal ;
            }
//            if(sampler->next1D()<RussianRoulette){
//                BSDFQueryRecord queryRecord(its.shFrame.toLocal(-ray.d));
//                Color3f xcolor = its.mesh->getBSDF()->sample(queryRecord, sampler->next2D());
//                if(!xcolor.isZero())
//                {
////                    Color3f reLix = Li(scene, sampler, Ray3f(its.p, its.shFrame.toWorld(queryRecord.wo)));
//                    L += reLix * xcolor ;
//                }
//        }
        }
        return L;
        }



        std::string toString() const {
            return "SimpleIntegrator[]";
        }
    private:
        Vector3f pos;
        Color3f energy;
        float RussianRoulette=0.95;
    };


    NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END
