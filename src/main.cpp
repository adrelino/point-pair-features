//
//  main.cpp
//  PointPairFeatures
//
//  Created by Adrian Haarbach on 27.06.14.
//  Copyright (c) 2014 Adrian Haarbach. All rights reserved.
//
#include "Visualize.h"
#include "LoadingSaving.h"
#include "PointPairFeatures.h"
#include "PointCloudManipulation.h"
#include <iostream>
#include "Constants.h"

using namespace std;

int main(int argc, char * argv[])
{

    Visualize* inst = Visualize::getInstance();

    //PointCloud m=LoadingSaving::loadPointCloud("bunny/model.xyz"); //bunny/cloudXYZ_0.xyz");
    PointCloud m=LoadingSaving::loadPointCloud("bunny/bunny_smoothNormals.xyz");
    PointCloud mSmall=PointCloudManipulation::downSample(m,false);


    PointCloud s=LoadingSaving::loadPointCloud("bunny/bunny_smoothNormals.xyz",400);
    PointCloud sSmall=PointCloudManipulation::downSample(s,false);
    PointCloudManipulation::reestimateNormals(sSmall,0.015f);

    PointCloudManipulation::reestimateNormals(mSmall,0.015f);
    Translation3f traCentroid=PointCloudManipulation::getTranslationToCentroid(mSmall);
    mSmall=PointCloudManipulation::projectPointsAndNormals(Isometry3f(traCentroid),mSmall);
    
//    PointCloud s=LoadingSaving::loadPointCloud("bunny/scene.xyz");
//    PointCloud sSmall=PointCloudManipulation::downSample(s,false);
//    sSmall=PointCloudManipulation::projectPointsAndNormals(Isometry3f(traCentroid),sSmall);

    Quaternionf q=Quaternionf::Identity();
    q = q * AngleAxisf(deg2rad(-30), Vector3f::UnitX());
    q = q * AngleAxisf(deg2rad(60),Vector3f::UnitY());
    q = q * AngleAxisf(deg2rad(65),Vector3f::UnitZ());

    Vector4f rot(q.x(),q.y(),q.z(),q.w());
    //Vector4f rot(.2,.2,.2,.4);
    //Vector3f tra(0,0,0);//
    Vector3f tra(.04,0.09,-0.07);//,0.5,0.01);

    Isometry3f P = Translation3f(tra)*Quaternionf(rot);
    PointPairFeatures::printPose(P,"P_original:");
    sSmall=PointCloudManipulation::projectPointsAndNormals(P, sSmall);

    //Visualize::visualize();

    inst->model=mSmall;
    inst->scene=sSmall;

    cout<<"initial setup scene and model, press q to continue"<<endl;

    //Visualize::spin();


    Isometry3f Pest=PointPairFeatures::getTransformationBetweenPointClouds(mSmall,sSmall);

    cout<<"PPF coarse P "<<endl;
    PointPairFeatures::err(P,Pest);


    Isometry3f P_Iterative_ICP = Pest; //initialize with ppf coarse alignment
    PointCloud modelPoseEst;

    for (int i = 0; i < 100; ++i) {  //5 ICP steps
        modelPoseEst=PointCloudManipulation::projectPointsAndNormals(P_Iterative_ICP, mSmall);

        vector<Vector3f> src,dst;

        vector<int> idxx = PointCloudManipulation::getClosesPoints(modelPoseEst,sSmall,src,dst,0.03f);

        inst->modelT=modelPoseEst;
        inst->closestPtsSceneToModel = idxx;

        cout<<"Iteration "<<i<<endl;
        cout<<"# Scene to Model Correspondences: "<<idxx.size()<<"=="<<src.size()<<"=="<<dst.size()<<endl;

        Visualize::spin();



        Isometry3f P_incemental = ICP::computeStep(src,dst,false);

        P_Iterative_ICP = P_incemental * P_Iterative_ICP;

        cout<<"ICP Step "<<i<<endl;
        PointPairFeatures::err(P,P_Iterative_ICP);




    }

    while(Visualize::waitKey('Q')){
        glutPostRedisplay();
        glutMainLoopEvent();
    }

}
