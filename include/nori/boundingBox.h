//
// Created by 打工人 on 2022/5/29.
//



#include <nori/object.h>


NORI_NAMESPACE_BEGIN

class BoundingBox{
    Vector3f min;
    Vector3f max;
    void enclose(Vector3f point) {
        min = hmin(min, point);
        max = hmax(max, point);
    }
    void enclose(BoundingBox & box) {
        min = hmin(min, box.min);
        max = hmax(max, box.max);
    }



};
NORI_NAMESPACE_END

