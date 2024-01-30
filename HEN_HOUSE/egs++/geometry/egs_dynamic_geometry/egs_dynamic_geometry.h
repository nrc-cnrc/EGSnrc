/*
###############################################################################
#
#  EGSnrc egs++ dynamic_geometry geometry headers
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Alex Demelo 2023
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_dynamic_geometry.h
 *  \brief A dynamic geometry: header
 *  \IK
 */

#ifndef EGS_DYNAMIC_GEOMETRY_
#define EGS_DYNAMIC_GEOMETRY_

#include "egs_base_geometry.h"
#include "egs_transformations.h"
#include "egs_rndm.h"
#include "egs_vector.h"
#include "egs_application.h"
#include <vector>
#include <string>
#include <sstream>

#ifdef WIN32

    #ifdef BUILD_DYNAMIC_GEOMETRY_DLL
        #define EGS_DYNAMIC_GEOMETRY_EXPORT __declspec(dllexport)
    #else
        #define EGS_DYNAMIC_GEOMETRY_EXPORT __declspec(dllimport)
    #endif
    #define EGS_DYNAMIC_GEOMETRY_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_DYNAMIC_GEOMETRY_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_DYNAMIC_GEOMETRY_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_DYNAMIC_GEOMETRY_EXPORT
        #define EGS_DYNAMIC_GEOMETRY_LOCAL
    #endif

#endif

/*! \brief A dynamic geometry.

  \ingroup Geometry
  \ingroup CompositeG

*/

class EGS_DYNAMIC_GEOMETRY_EXPORT EGS_DynamicGeometry :
    public EGS_BaseGeometry {

public:

    struct EGS_ControlPointGeom {

	EGS_Float time; //time index for control point
	vector<EGS_Float> trnsl; //vector specifying x,y,z translation
	vector<EGS_Float> rot;
    };


   /*! \brief Construct a dynamic geometry using \a G as the
    geometry and cpts as the control points.
    */
    EGS_DynamicGeometry(EGS_BaseGeometry *G, EGS_Input *dyninp, const string &Name = "") : EGS_BaseGeometry(Name), g(G){
	type = g->getType();
        type += "D";
        nreg = g->regions();
        is_convex = g->isConvex();
        has_rho_scaling = g->hasRhoScaling();
        has_B_scaling = g->hasBScaling();
	EGS_DynamicGeometry::buildDynamicGeometry(g, dyninp); //build dynamic geometry is where many of the geometry attributes (including control points) are extracted

        if (cpts.size()<2) {
            egsWarning("EGS_DynamicSource: not enough or missing control points.\n");
        }
        else {
            if (cpts[0].time > 0.0) {
                egsWarning("EGS_DynamicSource: time index of control point 1 > 0.0.  This will generate many warning messages.\n");
            }
            int npts = cpts.size();
            for (int i=0; i<npts; i++) {
                if (i>0 && cpts[i].time < cpts[i-1].time-epsilon) {
                    egsWarning("EGS_DynamicSource: time index of control point %i < time index of control point %i\n",i,i-1);
                }
                if (cpts[i].time<0.0) {
                    egsWarning("EGS_DynamicSource: time index of control point %i < 0.0\n",i);
                }
            }
            //normalize time values
            for (int i=0; i<npts-1; i++) {
                cpts[i].time /= cpts[npts-1].time;
            }
        }

    };

    ~EGS_DynamicGeometry() {
        if (!g->deref()) {
            delete g;
        }
    };

    //sets the current state transform of the geometry. This is called when checking location. Same as gtransformed.
    void setTransformation(EGS_AffineTransform t) {
        T = t;
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &x,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        EGS_Vector xt(x), ut(u);
        T.inverseTransform(xt);
        T.rotateInverse(ut);
        return g->computeIntersections(ireg,n,xt,ut,isections);
    };
    bool isRealRegion(int ireg) const {
        return g->isRealRegion(ireg);
    };
    bool isInside(const EGS_Vector &x) {
        EGS_Vector xt(x);
        T.inverseTransform(xt);
        return g->isInside(xt);
    };
    int isWhere(const EGS_Vector &x) {
        EGS_Vector xt(x);
        T.inverseTransform(xt);
        return g->isWhere(xt);
    };
    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        return g->medium(ireg);
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        return ireg >= 0 ? g->howfarToOutside(ireg,x*T,u*T.getRotation()) : 0;
    };
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        EGS_Vector xt(x), ut(u);
        T.inverseTransform(xt);
        T.rotateInverse(ut);
        int inew = g->howfar(ireg,xt,ut,t,newmed,normal);
        if (inew != ireg && normal) {
            *normal = T.getRotation()*(*normal);
        }
        return inew;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Vector xt(x);
        T.inverseTransform(xt);
        return g->hownear(ireg,xt);
    };

    int getMaxStep() const {
        return g->getMaxStep();
    };


    bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        return g->hasBooleanProperty(ireg,prop);
    };
    void setBooleanProperty(EGS_BPType prop) {
        g->setBooleanProperty(prop);
    };
    void addBooleanProperty(int bit) {
        g->addBooleanProperty(bit);
    };
    void setBooleanProperty(EGS_BPType prop, int start, int end, int step=1) {
        g->setBooleanProperty(prop,start,end,step);
    };
    void addBooleanProperty(int bit, int start, int end, int step=1) {
        g->addBooleanProperty(bit,start,end,step);
    };

    const string &getType() const {
        return type;
    };

    EGS_Float getRelativeRho(int ireg) const {
        return g->getRelativeRho(ireg);
    }
    void setRelativeRho(int start, int end, EGS_Float rho);

    void setRelativeRho(EGS_Input *);

    EGS_Float getBScaling(int ireg) const {
        return g->getBScaling(ireg);
    }
    void setBScaling(int start, int end, EGS_Float bf);

    void setBScaling(EGS_Input *);


    virtual void getLabelRegions(const string &str, vector<int> &regs);

     /*equivalent of get next particle but for geometries. It is tasked with determining the next state of the dynamic geometry. This is done by obtaining a time index (either
      * from the simulation source or by sampling itself if the source yields nothing), obtaining the corresponding position and orientation coordinates (through getCoordGeom) and creating
      * and setting the dynamic geometry's transform */
    void getNextGeom(EGS_RandomGenerator *rndm) {
	int errg = 1;
        EGS_ControlPointGeom gipt;

	//here get source from activeapplication in order to extract time
	EGS_Application *app = EGS_Application::activeApplication();
	while (errg) {
		ptime = app->getTimeIndex();//gets time from source if it exists (otherwise gives -1).
		if (ptime<0) {
			//if no time is given by the source the geometry will randomly sample from 0 to 1.
			ptime = rndm->getUniform();
                        //set randomly sampled time index for all objects in the simualtion (through base source)
			app->setTimeIndex(ptime);
		}
           errg = getCoordGeom(ptime,gipt);//now run the get coord method that will sample the cpt given to find the transformation that will be applied for the current history
        }

        //create and set the current geometry transformation using the sampled coordinates from getCoordGeom. This is where overloaded EGsSAffineTransform is used
	EGS_AffineTransform *tDG = EGS_AffineTransform::getTransformation(gipt.trnsl, gipt.rot);
	setTransformation(*tDG);
        //call getNextGeom on base geometry in case there are lower level dynamic geometries
	g->getNextGeom(rndm);
    };

     void updatePosition(EGS_Float time) {
         /*this method is essentially just to update the geometry state as needed in egsview. It takes the desired time index, and computes the correspoding coordinates, and sets the
          * geometry transform to update the egsview display*/
        int errg = 1;
        EGS_ControlPointGeom gipt;
        errg = getCoordGeom(time,gipt); //run the get coord method that will use the control points given to find the transformation that will be applied for the current time index
        //create and set the geometry transform with the updated coordinates
        EGS_AffineTransform *tDG = EGS_AffineTransform::getTransformation(gipt.trnsl, gipt.rot);
        setTransformation(*tDG);
        //call update positions on base to allow lower level geometries to update as needed
        g->updatePosition(time);
    };

    void containsDynamic(bool &hasdynamic) {
        //this method is essentially used to determine whether the simulation geometry contains a dynamic geometry.
        hasdynamic=true;  //if the dynamic geometry implementation of containsdynamic is called, the simulation does indeed contain a dynamic geometry, so the boolean flag is set to true
        //g->containsDynamic(hasdynamic); //technically we dont need this as we only need to find one. We don't care about the number we just want to know if there is any at all
    }


protected:
    EGS_BaseGeometry    *g;   //!< The geometry undergoing dynamic motion

    string              type; //!< The geometry type

    EGS_AffineTransform T;

    vector<EGS_ControlPointGeom> cpts;  //control points

    int ncpts;  //no. of control points

    EGS_Float ptime; //time index corresponding to particle

                   //could just be a random number.
    /*! \brief Don't define media in the transformed geometry definition.
    This function is re-implemented to warn the user to not define
    media in the definition of a transformed geometry. Instead, media should
    be defined when specifying the geometry to be transformed.
    */
    void setMedia(EGS_Input *inp,int,const int *);

    int getCoordGeom(EGS_Float rand, EGS_ControlPointGeom &gipt);

    void buildDynamicGeometry(EGS_BaseGeometry *g, EGS_Input *dyninp);
    //EGS_BaseGeometry *createGeometry(EGS_Input *input);
};

#endif
