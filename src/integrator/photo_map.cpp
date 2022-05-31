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
#include "nori/kdTree.h"
NORI_NAMESPACE_BEGIN


static void tracePhoton(const Scene * scene,Photon photon,std::vector<Photon> & photons,Sampler * sampler){
    Ray3f  ray(photon.position,photon.direction);
    std::cout<<photon.position<<" "<<photon.direction<<endl;
    while(true){
        Intersection its;
        if(!scene->rayIntersect(ray,its))
            break; //hit nothing;
//        if(its.shFrame.n.dot(ray.d)>0 && its.mesh->getBSDF());
        auto bsdf=its.mesh->getBSDF();
        if(!bsdf->isDiffuse())
            break;
        if(random()<0.95l)
            break;

        BSDFQueryRecord bRec(its.toLocal(-ray.d));
        its.mesh->getBSDF()->sample(bRec,sampler->next2D());
        auto outdir=its.toWorld(bRec.wo);


//        Photon photon(its.p+Epsilon*outdir,outdir,photon.power);
        photons.push_back(photon);

        photon.position=its.p+Epsilon*outdir;
        photon.direction=outdir;
        photon.power *=  bsdf->eval(bRec) * abs(Frame::cosTheta(bRec.wi)) / bsdf->pdf(bRec) /0.95;

        ray.o=its.p;
        ray.d=outdir;
    }



}


    class PhotonMapper : public Integrator {

        int nEmittedPhotons=10000;

    public:
        PhotonMapper(const PropertyList &props) {
              usePhotoMap= true;
//            this->energy=props.getColor("energy");
//            this->pos=props.getPoint("position");
//            /* No parameters this time */
        }

        void preprocess(const Scene *scene) override {
            std::vector<Photon> photons;
            auto canvasSize=scene->getCamera()->getOutputSize();
            std::unique_ptr<Sampler > sampler=scene->getSampler()->clone();


            for(int i=0;i<nEmittedPhotons;i++){
                    for(auto light:scene->lights){

                            Ray3f photonRay;
                            Color3f power;
                            Photon photon=  light->samplePhoton(sampler.get());
                            tracePhoton(scene,photon,photons,sampler.get());
//                          light->getShape()->sample(record,sampler.get(),1);
//                          auto dir=Warp::squareToUniformSphere(sampler->next2D());
//                          Photon photon(record.pos,dir,light->)
                    }
            }
            std::cout<<photons.size();
            kdTree =new KdTree();
            kdTree->build(photons);

//            kdTree=new KdTree(photons);/
        }


        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,int depth=0) const {

        }

        std::string toString() const {
            return "SimpleIntegrator[]";
        }
    private:
        KdTree   * kdTree;

    };
    NORI_REGISTER_CLASS(PhotonMapper, "photo_mapper");
NORI_NAMESPACE_END
