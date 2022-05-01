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

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
            /* Find the surface that is visible in the requested direction */
            Intersection its;
            Ray3f _ray(ray);
            if (!scene->rayIntersect(_ray, its))
                return Color3f(0.0f);
//            return Color3f(0.25,0.5,0.6);
            int depth=0;
            Color3f throughput(1.f);
            Color3f L;

            while(sampler->next1D()<RussianRoulette){
                if(its.mesh->isEmitter())
                  return  L+ throughput * its.mesh->getEmitter()->eval(its);

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
                    L+= cosTheta1* cosTheta2 * radiance * f *throughput
                            /(distance* distance)
                            /eRec.pdfVal ;
//                    return L/0.95;
                }}
                if(sampler->next1D()<RussianRoulette){
                    BSDFQueryRecord queryRecord(its.toLocal(-ray.d));
                    Color3f xcolor = its.mesh->getBSDF()->sample(queryRecord, sampler->next2D());
                    Color3f reLix = Li(scene, sampler, Ray3f(its.p, its.shFrame.toWorld(queryRecord.wo)));
                    L += reLix * xcolor ;
                }
                break;
                /***********************************************************/
                //specular
                /**********************************************************/
//                else
//               {
//                   BSDFQueryRecord queryRecord(its.toLocal(-ray.d));
//                  Color3f bsdfVal=  its.mesh->getBSDF()->sample(queryRecord,sampler->next2D());
////                    Color3f  bsdfVal=its.mesh->getBSDF()->eval(queryRecord);
//                    if(bsdfVal.isZero())
//                        break;
//                    throughput*=bsdfVal;
//                    _ray.o=its.p;
//                    _ray.d=its.toWorld(queryRecord.wi);
//                    scene->rayIntersect(_ray,its);
//                }

//                return L;
            }
        return L/RussianRoulette;
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
