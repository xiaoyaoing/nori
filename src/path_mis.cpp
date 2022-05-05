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

    class PathMisIntegrator : public Integrator {

    public:
        PathMisIntegrator(const PropertyList &props) {

            /* No parameters this time */
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,int depth=0) const {
            return Li(scene,sampler,ray, NAN);
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,float pbsdf) const {
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
            // 打中光源
            /**********************************************************/
            if (its.mesh->isEmitter() ) {
                float weight;
                if(isnan(pbsdf)){
                    weight=1.0f;
                }
                else {
                    float pLight=scene->EmitterPdf();
                    pLight/=std::max(Frame::cosTheta(wiLocal), 0.0f) / (its.t*its.t);
                    weight=pbsdf/(pbsdf+pLight);
                }
                L += its.mesh->getEmitter()->eval(its)*weight;
            }

            /***********************************************************/
            //间接光照
            /**********************************************************/

            if(sampler->next1D()<RussianRoulette){
                BSDFQueryRecord queryRecord(its.toLocal(-ray.d));
                Color3f xcolor = its.mesh->getBSDF()->sample(queryRecord, sampler->next2D());

                if(!xcolor.isZero()) {
                    Color3f reLix = Li(scene, sampler, Ray3f(its.p, its.toWorld(queryRecord.wo)),
                                       bsdf->isDiffuse()?bsdf->pdf(queryRecord):NAN);
                    L+= reLix *  xcolor   /RussianRoulette;
                }
            }

            /***********************************************************/
            // 采样直接光
            /**********************************************************/
            if(bsdf->isDiffuse()){
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
                    Color3f radiance(eRec.emi->eval(eRec));
                    Color3f f=its.mesh->getBSDF()->eval({its.toLocal(-ray.d),its.toLocal(lightDir),ESolidAngle});
                    float cosTheta1=abs(its.shFrame.n.cwiseAbs().dot(lightDir));
                    float cosTheta2=std::max(eRec.normal.dot(-lightDir),0.f);
                    float lightPdf=scene->EmitterPdf();
                    float bsdfPdf=bsdf->pdf({its.toLocal(-ray.d),its.toLocal(lightDir),ESolidAngle});
                    lightPdf/=std::max(eRec.normal.dot(-lightDir), 0.0f) / (distance*distance); //面积分到立体角积分转换
                    if(!isinf(lightPdf))
                    {L+= cosTheta1* cosTheta2 * radiance * f
                        /(distance* distance)
                        /eRec.pdfVal *
                            (lightPdf/(lightPdf+bsdfPdf));}

                }
            }
            return L;
        }



        std::string toString() const {
            return "PathMisIntegrator[]";
        }
    private:
        Vector3f pos;
        Color3f energy;
        float RussianRoulette=0.95;
    };


    NORI_REGISTER_CLASS(PathMisIntegrator, "path_mis");
NORI_NAMESPACE_END

