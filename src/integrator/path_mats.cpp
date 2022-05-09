//
// Created by 打工人 on 2022/5/2.
//

//
// Created by 打工人 on 2022/4/30.
//
//
// Created by 打工人 on 2022/4/28.
//

#include "nori/integrator.h"
#include "nori/scene.h"
#include "nori/bsdf.h"
#include "nori/emitter.h"
#include "nori/sampler.h"
NORI_NAMESPACE_BEGIN

    class PathBruteIntegrator : public Integrator {

    public:
        PathBruteIntegrator(const PropertyList &props) {

            /* No parameters this time */
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,int depth=0) const {
            /* Find the surface that is visible in the requested direction */
            Intersection its;
            Ray3f _ray(ray);
            if (!scene->rayIntersect(_ray, its))
                return (0.0f);
            Color3f L;
            if(its.mesh->isEmitter())
            L=its.mesh->getEmitter()->eval(its);
            if(sampler->next1D()<RussianRoulette){
                BSDFQueryRecord queryRecord(its.toLocal(-ray.d));
                Color3f xcolor = its.mesh->getBSDF()->sample(queryRecord, sampler->next2D());
                Color3f reLix = Li(scene, sampler, Ray3f(its.p, its.shFrame.toWorld(queryRecord.wo)));
                L += reLix * xcolor ;
                return L/RussianRoulette;
            }
            return {0.0f};
        }



        std::string toString() const {
            return "PathMatsIntegrator[]";
        }
    private:
        Vector3f pos;
        Color3f energy;
        float RussianRoulette=0.99;
    };


    NORI_REGISTER_CLASS(PathBruteIntegrator, "path_mats");
NORI_NAMESPACE_END

