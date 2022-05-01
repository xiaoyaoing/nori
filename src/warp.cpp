/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/warp.h>
#include <nori/vector.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

Point2f Warp::squareToUniformSquare(const Point2f &sample) {
    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
    return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f : 0.0f;
}
static  float TentInverse(float x){
    if(x<=.5f)
        return std::sqrt(2*x)-1;
    return  1- std::sqrt(2-2*x);
}
Point2f Warp::squareToTent(const Point2f &sample) {
    Point2f  res(TentInverse(sample[0]), TentInverse(sample[1]));
    return  res;
//    throw NoriException("Warp::squareToTent() is not yet implemented!");
}

float Warp::squareToTentPdf(const Point2f &p) {
    return (1.f-abs(p[0])) * (1.f-abs(p[1]));
}


Point2f Warp::squareToUniformDisk(const Point2f &sample) {
    auto phi=2*sample.x()*M_PI;
    auto r=sqrt(sample.y());
    return {r*cos(phi),r*sin(phi)};
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
    return p.norm() < 1.f ? INV_PI : .0f;}

Vector3f Warp::squareToUniformSphere(const Point2f &sample) {
    float z = 1 - 2 * sample[0];
    float r = std::sqrt(std::max((float )0, (float)1 - z * z));
    float phi = 2 * M_PI * sample[1];
    return {r * std::cos(phi), r * std::sin(phi), z};
}

float Warp::squareToUniformSpherePdf(const Vector3f &v) {
//    throw NoriException("Warp::squareToUniformSpherePdf() is not yet implemented!");
      return 0.25f*INV_PI;
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {
    float z = 1 - 2 * sample[0];
    float r = std::sqrt(std::max((float )0, (float)1 - z * z));
    float phi = 2 * M_PI * sample[1];
    return {r * std::cos(phi), r * std::sin(phi), abs(z)};
    //throw NoriException("Warp::squareToUniformHemisphere() is not yet implemented!");
}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
    return v[2] >=0 ? 0.5f * INV_PI : .0f;
//    throw NoriException("Warp::squareToUniformHemispherePdf() is not yet implemented!");
}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
    float z=sqrt(1-sample.x());
    float phi=sample.y()*2*M_PI;

    return {sqrt(sample.x())* cos(phi),sqrt(sample.x())*sin(phi),z};
//    throw NoriException("Warp::squareToCosineHemisphere() is not yet implemented!");
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
    return v[2] >=0 ? v.z() * INV_PI : .0f;
    throw NoriException("Warp::squareToCosineHemispherePdf() is not yet implemented!");
}

Vector3f Warp::squareToBeckmann(const Point2f &sample, float alpha) {
    auto tan2theta= -alpha*alpha*log( sample.x() );
    auto cosTheta=sqrt(1/(1+tan2theta));
    auto sinTheta= sqrt(1-cosTheta*cosTheta);
    auto phi=sample.y() * 2 * M_PI;
    return (sinTheta*cos(phi), sinTheta*sin(phi),cosTheta);
//    throw NoriException("Warp::squareToBeckmann() is not yet implemented!");
}

float Warp::squareToBeckmannPdf(const Vector3f &m, float alpha) {
    if(m.z()<0)
        return 0.0f;
    auto cosTheta=m.z();
    auto sinTheta=sqrt(1-cosTheta*cosTheta);
    auto tan2Theta=(sinTheta* sinTheta)/(cosTheta*cosTheta);
    return INV_PI * exp(-tan2Theta/(alpha*alpha))  / (alpha*alpha*pow(cosTheta,3));

//    throw NoriException("Warp::squareToBeckmannPdf() is not yet implemented!");
}

NORI_NAMESPACE_END
