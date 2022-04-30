//
// Created by 打工人 on 2022/4/29.
//


#include "../include/nori/emitter.h"
NORI_NAMESPACE_BEGIN
class AreaLight : public Emitter{

public:
    std::string toString() const override {
        return "AreaLight";
    }



    AreaLight(const PropertyList list) {
        this->radiance=list.getColor("radiance");
    }

private:
    Color3f radiance;
};

NORI_REGISTER_CLASS(AreaLight, "area");


NORI_NAMESPACE_END