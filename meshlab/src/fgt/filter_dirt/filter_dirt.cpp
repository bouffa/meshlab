/****************************************************************************
* MeshLab                                                           o o     *
* An extendible mesh processor                                    o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005, 2006                                          \/)\/    *
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

#include <Qt>
#include <QtGui>
#include "filter_dirt.h"
#include "dustparticle.h"
#include "dustsampler.h"
#include "dirt_utils.h"


#include <vcg/space/color4.h>
#include <vcg/math/random_generator.h>
#include <vcg/complex/trimesh/closest.h>
#include <vcg/space/index/spatial_hashing.h>
#include <vcg/complex/trimesh/stat.h>
#include <vcg/complex/trimesh/update/topology.h>
#include <vcg/space/box2.h>
#include <vcg/math/base.h>
#include <vcg/complex/trimesh/clean.h>
#include <vcg/complex/trimesh/stat.h>
#include <vcg/complex/trimesh/smooth.h>
#include <vcg/complex/trimesh/update/flag.h>
#include <vcg/complex/trimesh/update/selection.h> 
#include <vcg/complex/trimesh/update/color.h>
#include <vcg/complex/trimesh/update/flag.h>
#include <vcg/complex/trimesh/update/bounding.h>
#include <vcg/complex/trimesh/update/normal.h>
#include <vcg/complex/trimesh/point_sampling.h>
#include <vcg/space/triangle3.h>
#include <vcg/complex/trimesh/allocate.h>
#include <vector>
#include <muParser.h>

using namespace std;
using namespace vcg;
using namespace mu;


FilterDirt::FilterDirt()
{

    typeList << FP_DIRT;
    
    FilterIDType tt;
    foreach(tt , types())
	actionList << new QAction(filterName(tt), this);
}

QString FilterDirt::filterName(FilterIDType filterId) const
{
    switch (filterId) {
    case FP_DIRT:
        return QString("Dust Accumulation");
        break;
    default:
        assert(0); return QString("error");
        break;
    }
}
QString FilterDirt::filterInfo(FilterIDType filterId) const
{
    switch (filterId) {
    case FP_DIRT:
        return QString("Simulate dust accumulation over the mesh");
        break;
    default:
        assert(0); return QString("error");
        break;
    }
}

void FilterDirt::initParameterSet(QAction* filter,MeshDocument &md, RichParameterSet &par){
    par.addParam(new RichInt("nparticles",1000,"Number of Dust Particles","Number of Dust Particles to Generate"));
    //par.addParam(new RichDynamicFloat("step",0,0,10,"Steps","Steps of simulation"));

    return;
}

int FilterDirt::getRequirements(QAction */*action*/)
{	
    return MeshModel::MM_FACEFACETOPO | MeshModel::MM_VERTCOLOR |MeshModel::MM_FACECOLOR;
}

bool FilterDirt::applyFilter(QAction *filter, MeshDocument &md, RichParameterSet &par, vcg::CallBackPos *cb)
{


    if(md.size()>=2){//This is temporary just to try new steps of simulation


        MeshModel* dmesh=md.getMesh("Dust Mesh");
        Point3f dir;
        Point3f new_bar_coords;
        dir[0]=0;
        dir[1]=0;
        dir[2]=0.5;
        if(dmesh!=0){
                   CMeshO::VertexIterator vi;//= dmesh->cm.vert.begin();
                   CMeshO::PerVertexAttributeHandle<DustParticle<CMeshO> > pi = tri::Allocator<CMeshO>::GetPerVertexAttribute<DustParticle<CMeshO> >(dmesh->cm,"ParticleInfo");
                   CMeshO::CoordType new_pos;
                   CMeshO::CoordType int_p;
                   CMeshO::FacePointer new_f;
                   bool stop_movement=false;
                   for(vi=dmesh->cm.vert.begin();vi!=dmesh->cm.vert.end();++vi){
                       stop_movement=false;
                       new_pos=StepForward((*vi).P(),pi[vi].face,dir);
                       while(!stop_movement){

                           if(!IsOnFace(new_pos,pi[vi].face)){


                            //if(ComputeIntersection((*vi).P(),new_pos,pi[vi].face,int_p,new_f)){
                            ComputeIntersection((*vi).P(),new_pos,pi[vi].face,int_p,new_f);

                            pi[vi].face=new_f;
                            Segment3f s1=Segment3f((*vi).P(),new_pos);
                            Segment3f s2=Segment3f((*vi).P(),int_p);
                            float t=s2.Length()/s1.Length();
                            new_pos=int_p;
                            (*vi).P()=new_pos;
                            (*pi[vi].face).C()=Color4b::Blue;//Debugging
                            new_pos=StepForward((*vi).P(),pi[vi].face,dir,t);

                            /*if(!IsOnFace(new_pos,pi[vi].face))
                       {                                                                        {
                           (*pi[vi].face).C()=Color4b::Green;//Debugging
                           stop_movement=true;
                           (*vi).P()=new_pos;
                       }*/
//     }
                        }else{
                         //The new position is on the same face
                         (*pi[vi].face).C()=Color4b::Green;//Debugging
                         stop_movement=true;
                         (*vi).P()=new_pos;
                        }



                    }
              }
               }
    }else{

        //First Application
        vector<Point3f> dustVertexVec;
        vector<DustParticle<CMeshO> > dustParticleVec;

        DustSampler<CMeshO> ts(dustVertexVec,dustParticleVec);
        MeshModel* currMM=md.mm();
        if (currMM->cm.fn==0) {
                errorMessage = "This filter requires a mesh with some faces,<br> it does not work on PointSet";
                return false;
        }
        std::string func_d = "ny";


        currMM->updateDataMask(MeshModel::MM_FACEFACETOPO);
        currMM->updateDataMask(MeshModel::MM_FACEMARK);
        currMM->updateDataMask(MeshModel::MM_FACECOLOR);
        currMM->updateDataMask(MeshModel::MM_VERTQUALITY);
        tri::UnMarkAll(currMM->cm);

        //clean Mesh

        tri::Allocator<CMeshO>::CompactFaceVector(currMM->cm);
        tri::Clean<CMeshO>::RemoveUnreferencedVertex(currMM->cm);
        tri::Clean<CMeshO>::RemoveDuplicateVertex(currMM->cm);
        tri::Allocator<CMeshO>::CompactVertexVector(currMM->cm);

        //clean flags
        tri::UpdateFlags<CMeshO>::FaceClear(currMM->cm);

        //update Mesh
        tri::UpdateTopology<CMeshO>::FaceFace(currMM->cm);
        tri::UpdateNormals<CMeshO>::PerFaceNormalized(currMM->cm);
        tri::UpdateFlags<CMeshO>::FaceProjection(currMM->cm);

        Parser p;


        setPerVertexVariables(p);
        p.SetExpr(func_d);

        CMeshO::VertexIterator vi;

        for(vi=currMM->cm.vert.begin();vi!=currMM->cm.vert.end();++vi)
        {
            setAttributes(vi,currMM->cm);
            try {
                (*vi).Q() = p.Eval();
            } catch(Parser::exception_type &e) {
                errorMessage = e.GetMsg().c_str();
                return false;
            }
        }

        tri::SurfaceSampling<CMeshO,DustSampler<CMeshO> >::Montecarlo(currMM->cm,ts,par.getInt("nparticles"));

        //dmm -> Dust Mesh Model
        MeshModel* dmm=md.addNewMesh("Dust Mesh");

        dmm->cm.Clear();
        tri::Allocator<CMeshO>::AddVertices(dmm->cm,dustVertexVec.size());

        CMeshO::PerVertexAttributeHandle<DustParticle<CMeshO> > ph= tri::Allocator<CMeshO>::AddPerVertexAttribute<DustParticle<CMeshO> > (dmm->cm,std::string("ParticleInfo"));


        CMeshO::VertexIterator vIter=dmm->cm.vert.begin();
        vector<Point3f>::iterator dvIter;
        std::vector< DustParticle<CMeshO> >::iterator dpIter=dustParticleVec.begin();

        for(dvIter=dustVertexVec.begin();dvIter!=dustVertexVec.end();++dvIter){
            (*vIter).P()=CMeshO::CoordType ((*dvIter)[0],(*dvIter)[1],(*dvIter)[2]);

            ph[vIter]=(*dpIter);
            ++dpIter;
            ++vIter;
        }
    }

    return true;
}

MeshFilterInterface::FilterClass FilterDirt::getClass(QAction *)
{
    return MeshFilterInterface::VertexColoring;
}

void FilterDirt::setPerVertexVariables(Parser &p)
{
    p.DefineVar("x", &x);
    p.DefineVar("y", &y);
    p.DefineVar("z", &z);
    p.DefineVar("nx", &nx);
    p.DefineVar("ny", &ny);
    p.DefineVar("nz", &nz);
    p.DefineVar("r", &r);
    p.DefineVar("g", &g);
    p.DefineVar("b", &b);
    p.DefineVar("q", &q);
    p.DefineVar("vi",&v);
    p.DefineVar("rad",&rad);

    // define var for user-defined attributes (if any exists)
    // if vector is empty, code won't be executed
    for(int i = 0; i < (int) v_attrNames.size(); i++)
        p.DefineVar(v_attrNames[i],&v_attrValue[i]);
}

void FilterDirt::setAttributes(CMeshO::VertexIterator &vi, CMeshO &m)
{
    x = (*vi).P()[0]; // coord x
    y = (*vi).P()[1]; // coord y
    z = (*vi).P()[2]; // coord z

    nx = (*vi).N()[0]; // normal coord x
    ny = (*vi).N()[1]; // normal coord y
    nz = (*vi).N()[2]; // normal coord z

    r = (*vi).C()[0];  // color R
    g = (*vi).C()[1];  // color G
    b = (*vi).C()[2];  // color B

    q = (*vi).Q();     // quality

    if(tri::HasPerVertexRadius(m)) rad = (*vi).R();
    else rad=0;

    v = vi - m.vert.begin(); // zero based index of current vertex

    // if user-defined attributes exist (vector is not empty)
    //  set variables to explicit value obtained through attribute's handler
    for(int i = 0; i < (int) v_attrValue.size(); i++)
        v_attrValue[i] = vhandlers[i][vi];
}

Q_EXPORT_PLUGIN(FilterDirt)