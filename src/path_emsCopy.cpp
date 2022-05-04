//
// Created by 打工人 on 2022/5/2.
//
//
// Created by 打工人 on 2022/5/2.
//

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



/***********************************************************/
// 采样直接光的积分器
/**********************************************************/

    class PathEmsIntegrator : public Integrator {

    public:
        PathEmsIntegrator(const PropertyList &props) {
            /* No parameters this time */
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,int depth=0) const {
            return  Li(scene,sampler,ray, false);
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,bool diffuseBounce) const {
            Intersection its;
            Ray3f _ray(ray);
            if (!scene->rayIntersect(_ray, its))
                return {0.0f};
            Color3f L;
            Point3f shadingPoint = its.p;
            Normal3f shadingPointNormal = its.shFrame.n;
            const BSDF* bsdf = its.mesh->getBSDF();
            Vector3f wiLocal = its.shFrame.toLocal(-ray.d);
            BSDFQueryRecord bsdfQueryRecord { wiLocal };

            /***********************************************************/
            //打中光源并且需要当前是镜面反射才可以加上这项（漫反射在采样光源的时候已经计算过了)
            /**********************************************************/
            if (its.mesh->isEmitter() && !diffuseBounce) {
                
                L += its.mesh->getEmitter()->eval(its);
            }
            
            /***********************************************************/
            //间接光照
            /**********************************************************/
            if(sampler->next1D()<RussianRoulette){
                BSDFQueryRecord queryRecord(its.toLocal(-ray.d));
                Color3f xcolor = its.mesh->getBSDF()->sample(queryRecord, sampler->next2D());
                if(!xcolor.isZero()) {
                    Color3f reLix = Li(scene, sampler, Ray3f(its.p, its.toWorld(queryRecord.wo)), bsdf->isDiffuse());
                    L+= reLix *  xcolor /RussianRoulette;
                }
            }

            /***********************************************************/
            // 采样直接光
            /**********************************************************/
            if(bsdf->isDiffuse()){
                emitterRecord eRec;
                eRec.wi=-ray.d;
                eRec.its=&its;
                scene->sampleEmitter(eRec,sampler);
                Vector3f lightDir=(eRec.pos-its.p);
                float distance=lightDir.norm();
                lightDir.normalize();
                Ray3f  shadowRay(its.p,lightDir,0+Epsilon,distance-Epsilon);
                if(scene->rayIntersect(shadowRay)){
                    //do nothing
                }
                else {
                    Color3f radiance(eRec.emi->eval(eRec));
                    Color3f f=its.mesh->getBSDF()->eval({its.toLocal(-ray.d),its.toLocal(lightDir),ESolidAngle});
                    float cosTheta1=abs(its.shFrame.n.cwiseAbs().dot(lightDir));
                    float cosTheta2=abs(eRec.normal.dot(-lightDir));
                    L+= cosTheta1* cosTheta2 * radiance * f
                        /(distance* distance)
                        /eRec.pdfVal
                        ;
                }
            }
            return L;
        }



            std::string toString() const {
            return "PathEmsIntegrator[]";
        }
    private:
        Vector3f pos;
        Color3f energy;
        float RussianRoulette=0.95;
    };


    NORI_REGISTER_CLASS(PathEmsIntegrator, "path_ems");
NORI_NAMESPACE_END

