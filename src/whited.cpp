//
// Created by 打工人 on 2022/4/30.
//
//
// Created by 打工人 on 2022/4/28.
//

#include <nori/integrator.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN

    class WhittedIntegrator : public Integrator {

    public:
        WhittedIntegrator(const PropertyList &props) {

            /* No parameters this time */
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
            /* Find the surface that is visible in the requested direction */
            Intersection its;
            if (!scene->rayIntersect(ray, its))
                return Color3f(0.0f);

            auto distance=(its.p-pos).norm()-Epsilon*100;
            auto dir=(pos-its.p).normalized();
            Ray3f shadowRay(its.p,dir);
            shadowRay.maxt=distance;
            if(scene->rayIntersect(shadowRay)){
                return {.0,.0,.0};
            }
            float costheta=its.shFrame.n.cwiseAbs().dot(dir);
            auto mol=energy * std::max(0.f,costheta);
            auto den=4 * M_PI * M_PI *(distance* distance);
            return mol/den;
        }

        std::string toString() const {
            return "SimpleIntegrator[]";
        }
    private:
        Vector3f pos;
        Color3f energy;
    };


    NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END
