/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2007                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
#ifndef DIRT_UTILS_H
#define DIRT_UTILS_H

//Include Files
#include <math.h>

#include <common/meshmodel.h>
#include <common/interfaces.h>
#include<vector>
#include<vcg/simplex/vertex/base.h>
#include<vcg/simplex/face/base.h>
#include<vcg/complex/trimesh/base.h>
#include <vcg/space/point3.h>
#include <vcg/space/intersection2.h>




/*
  @description Simulate the movement of a point, affected by a force "dir" on a face.

  @parameter CMeshO::CoordType p - coordinates of the point
  @parameter CMeshO::FaceType face - pointer to the face
  @parameter CmeshO;;CoordType dir
  @return new coordinates of the point
*/

CMeshO::CoordType StepForward(CMeshO::CoordType p, CMeshO::FacePointer &face, CMeshO::CoordType dir,float t=1){
    //int t=1;
    Point3<float> new_pos;
    Point3<float> n= face->N();
    float a=n[0]*dir[0]+n[1]*dir[1]+n[2]*dir[2];
    //if(a<0.01 && a>-0.01 ) return p;

    Point3<float> f;
    //Calcolo le componenti della forza lungo il piano
    f[0]=dir[0]-a*n[0];
    f[1]=dir[1]-a*n[1];
    f[2]=dir[2]-a*n[2];



    new_pos[0]=p[0]+0.5*f[0]*t*t;
    new_pos[1]=p[1]+0.5*f[1]*t*t;
    new_pos[2]=p[2]+0.5*f[2]*t*t;

    return new_pos;
};

void DrawDirt(MeshDocument &md,std::vector<Point3f> &dp){
//TODO
};


bool ComputeIntersection(CMeshO::CoordType p1,
                         CMeshO::CoordType p2,
                         CMeshO::FacePointer &f,
                         CMeshO::CoordType &int_point,
                         CMeshO::FacePointer &new_f){
    CMeshO::CoordType n=f->N();

    //(*f).C()=Color4b::Blue;
    int max_c;
    float n0=math::Abs(n[0]);
    float n1=math::Abs(n[1]);
    float n2=math::Abs(n[2]);


    if(n0>n1){
          if(n0>n2) max_c=0;
          else max_c=2;
    }else{
        if(n1>n2) max_c=1;
        else max_c=2;
              }

    /*
    if(n0>n1){
          if(n0>n2) max_c=0;
          else max_c=2;
    }else{
        if(n1>n2) max_c=1;
        else max_c=2;
              }
*/
 //   CMeshO::CoordType int_point;

    Point2f int_p;

    Segment2f seg;

    CMeshO::CoordType fv0 = (f->V(0))->P();
    CMeshO::CoordType fv1 = (f->V(1))->P();
    CMeshO::CoordType fv2 = (f->V(2))->P();

    Segment2f line0;
    Segment2f line1;
    Segment2f line2;

    Point2f p1_2d;
    Point2f p2_2d;

    switch(max_c){

    case 0:{
            line0=Segment2f(Point2f(fv0[1],fv0[2]),Point2f(fv1[1],fv1[2]));
            line1=Segment2f(Point2f(fv1[1],fv1[2]),Point2f(fv2[1],fv2[2]));
            line2=Segment2f(Point2f(fv2[1],fv2[2]),Point2f(fv0[1],fv0[2]));
            p1_2d=Point2f(p1[1],p1[2]);
            p2_2d=Point2f(p2[1],p2[2]);
            seg=Segment2f(p1_2d,p2_2d);

            break;
        }
    case 1:{
            line0=Segment2f(Point2f(fv0[0],fv0[2]),Point2f(fv1[0],fv1[2]));
            line1=Segment2f(Point2f(fv1[0],fv1[2]),Point2f(fv2[0],fv2[2]));
            line2=Segment2f(Point2f(fv2[0],fv2[2]),Point2f(fv0[0],fv0[2]));
            p1_2d=Point2f(p1[0],p1[2]);
            p2_2d=Point2f(p2[0],p2[2]);
            seg=Segment2f(p1_2d,p2_2d);
            break;
        }
    case 2:{
            line0=Segment2f(Point2f(fv0[0],fv0[1]),Point2f(fv1[0],fv1[1]));
            line1=Segment2f(Point2f(fv1[0],fv1[1]),Point2f(fv2[0],fv2[1]));
            line2=Segment2f(Point2f(fv2[0],fv2[1]),Point2f(fv0[0],fv0[1]));
            p1_2d=Point2f(p1[0],p1[1]);
            p2_2d=Point2f(p2[0],p2[1]);
            seg=Segment2f(p1_2d,p2_2d);
            break;
        }
    }

    bool int_found=false;
    int edge=-1;

    Point2f tmp;
    if(SegmentSegmentIntersection(line0,seg,tmp)){
       int_found=true;
       int_p=tmp;
       edge=0;
    }

    if(SegmentSegmentIntersection(line1,seg,tmp)){
        if(int_found){

            if(Segment2f(tmp,p2_2d).Length()<Segment2f(int_p,p2_2d).Length()){
            int_found=true;
            int_p=tmp;
            edge=1;
            }
        }else{
            int_found=true;
            int_p=tmp;
            edge=1;
        }
    }



    if(SegmentSegmentIntersection(line2,seg,tmp)){
        if(int_found){
            if(Segment2f(tmp,p2_2d).Length()<Segment2f(int_p,p2_2d).Length()){
            int_found=true;
            int_p=tmp;
            edge=2;
            }
        }else{
            int_found=true;
            int_p=tmp;
            edge=2;
        }
    }

/*
    if(SegmentSegmentIntersection(line0,seg,int_p)){
        int_found=true;
        edge=0;
    }else{
        if(SegmentSegmentIntersection(line1,seg,int_p)){
        int_found=true;
        edge=1;
        }else{
            if(SegmentSegmentIntersection(line2,seg,int_p)){
            int_found=true;
            edge=2;
            }

        }

    }

*/

   
    if(int_found){






          f->C()=Color4b::Red;//Debug
          new_f=f->FFp(edge);


        switch(max_c){

        case 0:{
                int_point[0]=(n[0]*fv0[0]-n[1]*(int_p[0]-fv0[1])-n[2]*(int_p[1]-fv0[2]))/n[0];
                int_point[1]=int_p[0];
                int_point[2]=int_p[1];
                break;
            }
        case 1:{
                int_point[0]=int_p[0];
                int_point[1]=(n[1]*fv0[1]-n[0]*(int_p[0]-fv0[0])-n[2]*(int_p[1]-fv0[2]))/n[1];
                int_point[2]=int_p[1];
                break;
            }
        case 2:{
                int_point[0]=int_p[0];
                int_point[1]=int_p[1];
                int_point[2]=(n[2]*fv0[2]-n[0]*(int_p[0]-fv0[0])-n[1]*(int_p[1]-fv0[1]))/n[2];
                break;
            }
        }



    }



    return int_found;


};

bool IsOnFace(CMeshO::CoordType &p, CMeshO::FacePointer &f){

    CMeshO::CoordType b;
    CMeshO::CoordType p0=f->P(0);
    CMeshO::CoordType p1=f->P(1);
    CMeshO::CoordType p2=f->P(2);
    //x
    float A=p0[0]-p2[0];
    float B=p1[0]-p2[0];
    float C=p2[0]-p[0];
    //y
    float D=p0[1]-p2[1];
    float E=p1[1]-p2[1];
    float F=p2[1]-p[1];
    //z
    float G=p0[1]-p2[1];
    float H=p1[1]-p2[1];
    float I=p2[1]-p[1];

    b[0]=( B*(F+I)-C*(E+H))/(  A*(E+H)-B*(D+G) );
    b[1]=( A*(F+I)-C*(D+G))/( B*(D+G)- A*(E+H) );
    b[2]=1.0-b[0]-b[1];

        if(b[0]<0 || b[1]<0 || b[2]<0) return false;

        return true;
};

#endif // DIRT_UTILS_H