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


            Intersection its;
            Ray3f _ray(ray);
            if (!scene->rayIntersect(_ray, its))
                return {0.0f};
            Color3f L;
            Point3f shadingPoint = its.p;
            Normal3f shadingPointNormal = its.shFrame.n;
            const BSDF* bsdf = its.mesh->getBSDF();
            BSDFQueryRecord bsdfQ = BSDFQueryRecord(its.toLocal(-ray.d));
            if(!bsdf->isDiffuse()){
                if(sampler->next1D()>RussianRoulette)
                    return {0.0f};
                Color3f albedo=its.mesh->getBSDF()->sample(bsdfQ, Point2f(drand48(), drand48()));
                return albedo * Li(scene,sampler, Ray3f(its.p, its.toWorld(bsdfQ.wo))) / RussianRoulette;
            }

            if(its.mesh->isEmitter()) {
                if(depth == 0)
                    return its.mesh->getEmitter()->eval(its);
                else
                    return {0.f};
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
                    if(L.x()>0.9 && L.y()>0.9 && L.z()>0.9){
                        auto s1= tinyformat::format("cosTheta1%f  ",cosTheta1);
                        auto s2=tinyformat::format("cosTheta2%f  ",cosTheta2);
                        auto s3=tinyformat::format("PdfVal%f"  ,eRec.pdfVal);
                        auto s4=tinyformat::format("distance%f  ",distance);
                        auto s5=tinyformat::format("radiance%f  ",radiance);
                        auto s6=tinyformat::format("  f%f  ",f);
                        std::cout<<(s1+s2+s3+s4+s5+s6)<<endl;
                    }
                }
            }
            Color3f albedo = its.mesh->getBSDF()->sample(bsdfQ, Point2f(drand48(), drand48()));
            if(sampler->next1D()<RussianRoulette)
            {
//                L+=albedo * Li(scene, sampler, Ray3f(its.p, its.toWorld((bsdfQ.wo))),depth + 1);
                return L/RussianRoulette;
            }
            return {0.0f};
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

