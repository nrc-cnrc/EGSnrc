/*
###############################################################################
#
#  EGSnrc egs++ conez geometry headers
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:
#
###############################################################################
*/


#ifndef EGS_CONEZ_
#define EGS_CONEZ_

#include "egs_base_geometry.h"
#include "egs_input.h"
#include "egs_math.h"
#include "egs_functions.h"

#ifdef WIN32

    #ifdef BUILD_CONEZ_DLL
        #define EGS_CONEZ_EXPORT __declspec(dllexport)
    #else
        #define EGS_CONEZ_EXPORT __declspec(dllimport)
    #endif
    #define EGS_CONEZ_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_CONEZ_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_CONEZ_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_CONEZ_EXPORT
        #define EGS_CONEZ_LOCAL
    #endif

#endif

template <class T>
class EGS_CONEZ_EXPORT EGS_ConezT : public EGS_BaseGeometry {
protected:

    EGS_Float *theta,*cos_t,*cos2_t,*sin_t,*sin2_t;
    // for conc-cones, all apices coincide
    EGS_Vector xo;
    // projection operator
    T a;

public:

    ~EGS_ConezT() {
        if (nreg) {
            delete [] theta;
            delete [] cos_t;
            delete [] cos2_t;
            delete [] sin_t;
            delete [] sin2_t;
        }
    };

    EGS_ConezT(int nc, const EGS_Float *angle,
               const EGS_Vector &position, const string &Name,
               const T &A) : EGS_BaseGeometry(Name), a(A), xo(position) {
        if (nc>0) {
            theta=new EGS_Float [nc];
            cos_t=new EGS_Float [nc];
            cos2_t=new EGS_Float [nc];
            sin_t=new EGS_Float [nc];
            sin2_t=new EGS_Float [nc];

            for (int i=0; i<nc; i++) {
                // theta converted to radians in egs_conez.cpp
                theta[i]=angle[i];
                EGS_Float tmp=cos(theta[i]);
                cos_t[i]=tmp;
                cos2_t[i]=tmp*tmp;
                tmp=sin(theta[i]);
                sin_t[i]=tmp;
                sin2_t[i]=tmp*tmp;

            }
            nreg=nc;
        }
    };

    EGS_ConezT(const vector<EGS_Float> &angle,
               const EGS_Vector &position, const string &Name,
               const T &A) : EGS_BaseGeometry(Name), a(A), xo(position) {
        if (angle.size()>0) {
            theta=new EGS_Float [angle.size()];
            cos_t=new EGS_Float [angle.size()];
            cos2_t=new EGS_Float [angle.size()];
            sin_t=new EGS_Float [angle.size()];
            sin2_t=new EGS_Float [angle.size()];

            for (int i=0; i<angle.size(); i++) {
                theta[i]=angle[i];
                EGS_Float tmp=cos(theta[i]);
                cos_t[i]=tmp;
                cos2_t[i]=tmp*tmp;
                tmp=sin(theta[i]);
                sin_t[i]=tmp;
                sin2_t[i]=tmp*tmp;
            }
            nreg=angle.size();
        }
    };

    int inside(const EGS_Vector &x) {
        egsInformation("inside: x = (%g,%g,%g)\n",x.x,x.y,x.z);
        EGS_Vector rc(x-xo);
        EGS_Float rcp=a*rc;

        int region_test=1;

        /*
              if(rcp<0) return -1;
        */
        if (rcp<0) {
            region_test=-1;
        }

        EGS_Float cos2_phi=rcp*rcp/rc.length2();

        /*
              if(cos2_phi<cos2_t[nreg-1]) return -1;  // outside all cones
              if(cos2_phi>cos2_t[0]) return 0;  // inside central cone
        */
        if (region_test==1 && cos2_phi<cos2_t[nreg-1]) {
            region_test=-1;
        }
        if (region_test==1 && cos2_phi>cos2_t[0]) {
            region_test=0;
        }

        // find particle region
        int ic=0,oc=nreg,ms;
        if (region_test==1) {
            while (oc-ic>1) {
                ms=(ic+oc)/2;
                if (cos2_phi>cos2_t[ms]) {
                    oc=ms;
                }
                else {
                    ic=ms;
                }
            }
            region_test=oc;
        }

        /*
              return oc;
        */
        EGS_Float tmp;
        if (rcp<0) {
            tmp=acos(sqrt(cos2_phi))*180/M_PI+90;
        }
        else {
            tmp=acos(sqrt(cos2_phi))*180/M_PI;
        }
        egsInformation("inside: %2d %12.9f %12.9f %12.9f %12.9f\n",
                       region_test,rcp,rc.length2(),cos2_phi,tmp);

        return region_test;

    };

    bool isInside(const EGS_Vector &x) {
        return false;
    }
    int  isWhere(const EGS_Vector &x) {
        return -1;
    }

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {
        egsInformation("howfar: ireg = %d x = (%g,%g,%g) u = (%g,%g,%g)\n",ireg,
                       x.x,x.y,x.z,u.x,u.y,u.z);
        // projections and other often used variables
        EGS_Vector rc(x-xo);
        /*
        EGS_Float
          urc=u*rc,
          up=a*u,
          rcp=a*rc;
          */
        EGS_Float urc=u*rc;
        EGS_Float up=a*u;
        EGS_Float rcp=a*rc;
        EGS_Vector rcp_vec=a*rcp;
        EGS_Float d=1e30;
        int new_region=-1;

        egsInformation("howfar: %2d %12.9f %12.9f\n",
                       ireg,rcp,acos(rcp/rc.length())*180/M_PI);
//egsInformation("howfar: %2d %12.9f %12.9f %12f.9\n",
//    ireg,rcp,rc.length(),
//    acos(rcp/rc.length())*180/M_PI);

        // in any region?
        if (ireg>=0) {
            // outer cone normal at near side of cone
            EGS_Vector n=(rc-rcp_vec)*cos2_t[ireg]*2-rcp_vec*sin2_t[ireg]*2;
            // outer cone normal at far side of cone
            EGS_Vector n_reflected=a*(2*(a*n))-n;

            if (!(u*n<0 && u*n_reflected<0)) { // outer cone hit
                EGS_Float A=cos2_t[ireg]-up*up;
                EGS_Float B=0.5*(u*n);
                EGS_Float C=rc.length2()*cos2_t[ireg]-rcp*rcp;

                if ((u*n)*(u*n_reflected)<0) { // two hits on physical cone
                    d=-B+sqrt(B*B-A*C);
                    d/=A;
//egsInformation("%d %f %f %f %f %f %f %f ",
//    ireg,acos(rcp/rc.length())*181/M_PI,A,B,C,d,
//    u*n,u*n_reflected);
                }
                else { // one physical, one unphysical hit
                    d=-B-sqrt(B*B-A*C);
                    d/=A;
//egsInformation(" %d %f %f ",ireg,acos(rcp/rc.length())*180/M_PI,d);
                }

                new_region=ireg+1;
//egsInformation("--> %d\n",new_region);
            }

            if (ireg) { // there is an inner cone
                int ireg_inner=ireg-1;

                // inner cone normal at near side of cone
                EGS_Vector
                n=(rc-rcp_vec)*cos2_t[ireg_inner]*2-rcp_vec*sin2_t[ireg_inner]*2;
                EGS_Vector n_reflected=a*(2*(a*n))-n;

                if (u*n<0) { // inner cone hit
                    EGS_Float A=cos2_t[ireg_inner]-up*up;
                    EGS_Float B=0.5*(u*n);
                    EGS_Float C=rc.length2()*cos2_t[ireg_inner]-rcp*rcp;

                    EGS_Float dd;
                    if (u*n_reflected<0) { // one physical, one unphysical hit
                        dd=-B+sqrt(B*B-A*C);
                    }
                    else { // two hits on physical cone
                        dd=-B-sqrt(B*B-A*C);
                    }
                    dd/=A;

                    if (dd<d) {
                        d=dd;
                    }

                    new_region=ireg-1;
                }
            }
        }

        else { // outside all regions
            int ireg_inner=nreg-1;

            if (rcp==0) { // neither lower nor upper half-plane
                if (up<0) {
                    return -1;
                }

                EGS_Vector
                n=(rc-rcp_vec)*cos2_t[ireg_inner]*2-rcp_vec*sin2_t[ireg_inner]*2;
                EGS_Float A=cos2_t[ireg_inner]-up*up;
                EGS_Float B=0.5*(u*n);
                EGS_Float C=rc.length2()*cos2_t[ireg_inner]-rcp*rcp;

                d=-B-sqrt(B*B-A*C);
                d/=A;

                new_region=ireg_inner;
            }

            else {
                EGS_Vector n,n_reflected;
                if (rcp>0) { // we are in the lower half-plane
                    n=(rc-rcp_vec)*cos2_t[ireg_inner]*2-rcp_vec*sin2_t[ireg_inner]*2;
                    n_reflected=a*(2*(a*n))-n;
                }

                else { // we are in the upper half-plane (rcp!=0 as already checked)
                    if (up<0) {
                        return -1;
                    }

                    n_reflected=
                        rcp_vec*sin2_t[ireg_inner]*2-(rc-rcp_vec)*cos2_t[ireg_inner]*2;
                    n=a*(2*(a*n_reflected))-n_reflected;
                }

                if (u*n<0) { // might hit physical cone
                    EGS_Float A=cos2_t[ireg_inner]-up*up;
                    EGS_Float B=0.5*(u*n);
                    EGS_Float C=rc.length2()*cos2_t[ireg_inner]-rcp*rcp;

                    if (u*n_reflected<0) { // one real, one unphysical intersection
                        d=-B+sqrt(B*B-A*C);
                        d/=A;
                    }
                    else // either (1 or 2) real intersections
                        // OR 2 unphysical intersections
                    {
                        d=-B-sqrt(B*B-A*C);
                        d/=A;

                        // check for unphysical intersection
                        if (rcp>0 && (up<0 && -d*up>rcp)) {
                            return -1;
                        }
                        if (rcp<0 && (up<0 || -d*up>rcp)) {
                            return -1;
                        }
                    }

                    new_region=ireg_inner;
                }
                else {
                    return -1;
                }
            }
        }

        if (new_region>=nreg) {
            new_region=-1;
        }
        // correct t-step
        egsInformation("about to return: new_region = %d d = %g\n",new_region,d);
        if (d<t) {
            t=d;
            return new_region;
        }
        return ireg;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Vector rc(x-xo);
        EGS_Float d;
        EGS_Float rcp=a*rc;
        EGS_Float srt=sqrt(rc.length2()-rcp*rcp);

        // check outer cone first if inside geometry (seems to be working)
        if (ireg>=0) {
            d=rcp*sin_t[ireg]-srt*cos_t[ireg]; // outer cone
//egsInformation("%d %f\n",ireg,d);

            if (ireg) { // inner cone
                int ireg_inner=ireg-1;
                EGS_Float dd=srt*cos_t[ireg_inner]-rcp*sin_t[ireg_inner];
//egsInformation(" %d-%d %f",ireg,ireg_inner,dd);

                if (dd<d) {
                    d=dd;
                }
//egsInformation("---->%f\n",d);
            }
//egsInformation("%d %f %f %f\n",ireg,srt,rcp,d);
        }

        else { // two choices if in region -1
            int ireg_inner=nreg-1;

            if (rcp>0) {
                /* getting some negative values */
                d=srt*cos_t[ireg_inner]-rcp*sin_t[ireg_inner];
                /*
                egsInformation("%d %f %f %f %f ---> %f\n",
                    ireg_inner,srt,rcp,cos_t[ireg_inner],sin_t[ireg_inner],d);
                */
            }

            else {
                /* seems to be working */
                EGS_Float mag=rc.length();
                EGS_Float beta=M_PI/2+asin(-rcp/mag);

                if (2*(beta-theta[ireg_inner])>M_PI) {
                    d=mag;    // distance to apex
                }
                else {
                    d=srt*cos_t[ireg_inner]-rcp*sin_t[ireg_inner];
                }
            }
        }
        return d;
    };

    const string &getType() const {
        return a.getType();
    };

    void printInfo() const {
        EGS_BaseGeometry::printInfo();
        a.printInfo();
        egsInformation(" cone apex = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
        egsInformation(" opening angles (degrees)=    ");
        for (int j=0; j<nreg; j++) {
            egsInformation("%g ",theta[j]*180/M_PI);
        }
        egsInformation("\n===================================================\n");
    };
};

class EGS_CONEZ_LOCAL XProjector {
public:
    XProjector() {};
    EGS_Float operator*(const EGS_Vector &x) const {
        return x.x;
    };
    EGS_Vector operator*(const EGS_Float f)
    const {
        return EGS_Vector(f,0,0);
    };
    string &getType() const {
        return type;
    };
    void printInfo() const {};
private:
    static string type;
};

class EGS_CONEZ_LOCAL YProjector {
public:
    YProjector() {};
    EGS_Float operator*(const EGS_Vector &x) const {
        return x.y;
    };
    EGS_Vector operator*(const EGS_Float f)
    const {
        return EGS_Vector(0,f,0);
    };
    string &getType() const {
        return type;
    };
    void printInfo() const {};
private:
    static string type;
};

class EGS_CONEZ_LOCAL ZProjector {
public:
    ZProjector() {};
    EGS_Float operator*(const EGS_Vector &x) const {
        return x.z;
    };
    EGS_Vector operator*(const EGS_Float f)
    const {
        return EGS_Vector(0,0,f);
    };
    string &getType() const {
        return type;
    };
    void printInfo() const {};
private:
    static string type;
};

class EGS_CONEZ_LOCAL Projector {
public:
    Projector(const EGS_Vector &A) : a(A) {};
    EGS_Vector operator*(const EGS_Float f)
    const {
        return EGS_Vector(a.x*f,a.y*f,a.z*f);
    };
    EGS_Float operator*(const EGS_Vector &x) const {
        return a*x;
    };
    string &getType() const {
        return type;
    };
    void printInfo() const {
        egsInformation(" cone axis = (%g,%g,%g)\n",a.x,a.y,a.z);
    };

private:
    EGS_Vector a;
    static string type;
};

typedef EGS_ConezT<XProjector> EGS_ConezX;
typedef EGS_ConezT<YProjector> EGS_ConezY;
typedef EGS_ConezT<ZProjector> EGS_ConezZ;
typedef EGS_ConezT<Projector> EGS_Conez;

#endif
