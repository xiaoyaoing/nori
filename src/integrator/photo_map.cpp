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


static void tracePhoton(const Scene * scene,Ray3f & ray,Color3f & throughput,std::vector<Photon> * photons,Sampler * sampler){


//    auto throughput=photon.power;
    int depth=0;
    while( depth < 10){
        Intersection its;
        if(!scene->rayIntersect(ray,its))
            break;

        auto bsdf=its.mesh->getBSDF();
        if(bsdf->isDiffuse()){
            photons->emplace_back(Photon(ray.o,-ray.d,throughput));
        }

        const float russian_roulette_prob=std::min(std::max(throughput.x(),std::max(throughput.y(),throughput.z())),1.0f);
        if(sampler->next1D()>russian_roulette_prob){
            return ;
        }
        throughput/=russian_roulette_prob;

        BSDFQueryRecord bRec(its.toLocal(-ray.d));
        auto f=its.mesh->getBSDF()->sample(bRec,sampler->next2D());
        auto outdir=its.toWorld(bRec.wo);

        throughput*=f;
        ray=Ray3f(its.p,outdir);
        depth++;
    }
}


    class PhotonMapper : public Integrator {

        int nEmittedPhotons=100000;

    public:
        PhotonMapper(const PropertyList &props) {
        }

        void preprocess(const Scene *scene) override {


            std::cout<<"constructed photon map"<<endl;

            auto  photons =new std::vector<Photon>() ;
            auto canvasSize=scene->getCamera()->getOutputSize();
            std::unique_ptr<Sampler > sampler=scene->getSampler()->clone();


            for(int i=0;i<nEmittedPhotons;i++){
                    for(auto light:scene->lights){

                        Color3f power;
                        Ray3f  ray =light->sampleRay(sampler.get(),power);
                        tracePhoton(scene,ray,power,photons,sampler.get());
                    }
            }

            kdTree =new KdTree();
            std::cout << "counstructing KdTree"<<endl;
            kdTree->build(*photons);
            std::cout << "constructed completed photons:"<<photons->size()<<endl;
        }


        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray,int depth=0) const {

            Intersection its;
            if(!scene->rayIntersect(ray,its)){
                return {0.0f};
            }


            Color3f L;
            Point3f shadingPoint = its.p;
            Normal3f shadingPointNormal = its.shFrame.n;
            const BSDF* bsdf = its.mesh->getBSDF();

            if(its.mesh->isEmitter()){
                L+=its.mesh->getEmitter()->eval(its);
            }

            {
                if(bsdf->isDiffuse()){
                    Color3f flux{0};
                   auto nearestPhotons=  kdTree->kNN(its.p,100);
                   auto radius2=(nearestPhotons[0]->position  - its.p).squaredNorm();
                   BSDFQueryRecord queryRecord(its.toLocal(-ray.d));
                   queryRecord.measure=ESolidAngle;
                   for(Photon * photon : nearestPhotons){
                       queryRecord.wo=its.toLocal(photon->direction);
                       auto f =bsdf->eval(queryRecord);
                       flux += f * photon->power;
                   }

                   L += flux * INV_PI  / (radius2 * float(nEmittedPhotons));
                }

                else{
                    if(sampler->next1D()<RussianRoulette)
                        return {0.0f};
                    BSDFQueryRecord bsdfQ = BSDFQueryRecord(its.toLocal(-ray.d));
                    Color3f albedo = its.mesh->getBSDF()->sample(bsdfQ, Point2f(drand48(), drand48()));
                    L+=  albedo * Li(scene, sampler, Ray3f(its.p, its.toWorld(bsdfQ.wo)))/ RussianRoulette;
                }
            }

            return L;
        }

        Color3f getIrradiance(Vector3f pos,Vector3f normal){
            Vector3f  ret(0,0,0);
        }

        std::string toString() const {
            return "SimpleIntegrator[]";
        }
    private:
        KdTree   * kdTree;
        float RussianRoulette = 0.80;
    };
    NORI_REGISTER_CLASS(PhotonMapper, "photon_mapper");
NORI_NAMESPACE_END
