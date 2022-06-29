//
// Created by 打工人 on 2022/5/30.
//
#include <gtest/gtest.h>

#include "nori/kdTree.h"

NORI_NAMESPACE_BEGIN


int static  randomInt(int a,int b){
    return  a + rand()%b;
}

TEST(kdTree,test1){
    std::vector<Photon > photons;
    auto * kdTree =new KdTree();
    for(int i=0;i<1000;i++){
        Photon * photon=new Photon;
        photon->position=Vector3f (randomInt(0,1000), randomInt(0,1000), randomInt(0,1000));
        photon->idx=i;
        photons.push_back(*photon);
    }
    kdTree->build(photons);

    Vector3f  pos(randomInt(0,1000), randomInt(0,1000), randomInt(0,1000));

    int k=20;
    auto neigh=  kdTree->kNN(pos,k);

    float minD = -1e10f;
    for(auto & i:neigh){
        auto d=(i->position-pos).squaredNorm();
        minD= std::max(minD,(i->position-pos).squaredNorm() ) ;
    }

    int count =0;
    for(auto i:photons){
        int d=(int)(i.position-pos).squaredNorm();
        std::cout<<(i.position-pos).squaredNorm();
        if( (i.position-pos).squaredNorm() <= minD  ){
            count ++;
        }
    }

    ASSERT_EQ(count,k)<< tinyformat::format("k %d count %d",k,count);
}




// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}


NORI_NAMESPACE_END





