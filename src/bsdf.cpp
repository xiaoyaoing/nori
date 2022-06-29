//
// Created by 打工人 on 2022/6/27.
//

#include <nori/bsdf.h>
#include <nori/emitter.h>
NORI_NAMESPACE_BEGIN
void BSDF::addChild(NoriObject *obj){
if(obj->getClassType()== EEmitter)
{
    Emitter *  emitter= static_cast<Emitter *>(obj);
    m_emitter = emitter;
}
}

NORI_NAMESPACE_END
