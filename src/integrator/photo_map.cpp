//
// Created by 打工人 on 2022/4/29.
//
//
// Created by 打工人 on 2022/4/28.
//

#include "nori/integrator.h"
#include "nori/scene.h"
#include "nori/sampler.h"
#include "nori/warp.h"
#include "nori/camera.h"
NORI_NAMESPACE_BEGIN



    class PhotoMapper : public Integrator {

        int nEmittedPhotons;

    public:
        PhotoMapper(const PropertyList &props) {
              usePhotoMap= true;
//            this->energy=props.getColor("energy");
//            this->pos=props.getPoint("position");
//            /* No parameters this time */
        }

        void preprocess(const Scene *scene) override {
            Integrator::preprocess(scene);
            auto canvasSize=scene->getCamera()->getOutputSize();
            for(int i=0;i<nEmittedPhotons;i++){

            }
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,int depth=0) const {


        }

        std::string toString() const {
            return "SimpleIntegrator[]";
        }
    private:

    };
    NORI_REGISTER_CLASS(PhotoMapper, "photo_mapper");
NORI_NAMESPACE_END
