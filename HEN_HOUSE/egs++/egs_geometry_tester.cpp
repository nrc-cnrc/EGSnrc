/*
###############################################################################
#
#  EGSnrc egs++ geometry tester
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


/*! \file egs_geometry_tester.cpp
 *  \brief Implementation of the geometry testing classes.
 *  \IK
 */
#include "egs_geometry_tester.h"
#include "egs_input.h"
#include "egs_base_geometry.h"
#include "egs_rndm.h"
#include "egs_shapes.h"
#include "egs_timer.h"
#include "egs_math.h"
#include "egs_functions.h"

#include <cstdio>
#include <string>
using namespace std;

static int __geometry_error = 0;

#ifndef SKIP_DOXYGEN
/*! \brief This class implements the functionality of the EGS_GeometryTester
  class.

  \internwarning
 */
class EGS_LOCAL EGS_PrivateTester {

public:

    EGS_PrivateTester(EGS_GeometryTester *, EGS_Input *);
    ~EGS_PrivateTester() {
        /*
        if( inside_shape ) delete inside_shape;
        if( inside_time_shape ) delete inside_time_shape;
        if( hownear_shape ) delete hownear_shape;
        if( howfar_shape ) delete howfar_shape;
        if( howfar_time_shape ) delete howfar_time_shape;
        */
        EGS_Object::deleteObject(inside_shape);
        EGS_Object::deleteObject(inside_time_shape);
        EGS_Object::deleteObject(hownear_shape);
        EGS_Object::deleteObject(hownear_time_shape);
        EGS_Object::deleteObject(howfar_shape);
        EGS_Object::deleteObject(howfar_time_shape);
    };
    void testInside(EGS_BaseGeometry *);
    void testInsideTime(EGS_BaseGeometry *);
    void testHownear(int ntry, EGS_BaseGeometry *);
    void testHownearTime(EGS_BaseGeometry *);
    void testHowfar(EGS_BaseGeometry *, bool time);

    FILE *fp_info, *fp_warn, *fp_inside, *fp_hownear, *fp_howfar;
    FILE *fp_this_test;

private:

    EGS_GeometryTester  *parent;
    EGS_RandomGenerator *rndm;

    int                 n_inside;
    EGS_BaseShape       *inside_shape;

    int                 n_inside_time;
    EGS_BaseShape       *inside_time_shape;

    int                 n_hownear;
    EGS_BaseShape       *hownear_shape;

    int                 n_hownear_time;
    EGS_BaseShape       *hownear_time_shape;

    int                 n_howfar;
    EGS_BaseShape       *howfar_shape;
    bool                check_infinity;

    int                 n_howfar_time;
    EGS_BaseShape       *howfar_time_shape;
    bool                store_steps;

    void setTest(EGS_Input *i, const char *delim, int &n, EGS_BaseShape **s);

    int beginTest(int n, const EGS_BaseShape *s, const char *func,
                  const char *name, const EGS_BaseGeometry *g);

};
#endif

EGS_GeometryTester::EGS_GeometryTester(EGS_Input *i) {
    p = new EGS_PrivateTester(this,i);
}

EGS_GeometryTester::~EGS_GeometryTester() {
    delete p;
}

void EGS_GeometryTester::testInside(EGS_BaseGeometry *g) {
    p->testInside(g);
}

void EGS_GeometryTester::testInsideTime(EGS_BaseGeometry *g) {
    p->testInsideTime(g);
}

void EGS_GeometryTester::testHownear(int ntry, EGS_BaseGeometry *g) {
    p->testHownear(ntry,g);
}

void EGS_GeometryTester::testHownearTime(EGS_BaseGeometry *g) {
    p->testHownearTime(g);
}

void EGS_GeometryTester::testHowfar(EGS_BaseGeometry *g) {
    p->testHowfar(g,false);
}

void EGS_GeometryTester::testHowfarTime(EGS_BaseGeometry *g) {
    p->testHowfar(g,true);
}

void EGS_GeometryTester::printPosition(const EGS_Vector &x) {
    fprintf(p->fp_this_test,"%g %g %g\n",x.x,x.y,x.z);
};

#ifndef SKIP_DOXYGEN
EGS_PrivateTester::EGS_PrivateTester(EGS_GeometryTester *p, EGS_Input *input) {
    if (!p) egsFatal("EGS_PrivateTester::EGS_PrivateTester:\n"
                         " attempt to construct a tester with a null parent\n");
    parent = p;
    rndm = 0;
    n_inside = 0;
    n_inside_time = 0;
    n_hownear = 0;
    n_hownear_time = 0;
    n_howfar = 0;
    n_howfar_time = 0;
    inside_shape = 0;
    inside_time_shape = 0;
    hownear_shape = 0;
    howfar_shape = 0;
    hownear_time_shape = 0;
    howfar_time_shape = 0;
    hownear_shape = 0;
    store_steps = true;
    check_infinity = true;
    fp_info = stdout;
    fp_warn = stderr;
    fp_inside = stdout;
    fp_hownear = stdout;
    fp_howfar = stdout;

    rndm = EGS_RandomGenerator::createRNG(input);
    if (!rndm) {
        rndm = EGS_RandomGenerator::defaultRNG();
    }

    setTest(input,"inside test",n_inside,&inside_shape);
    fp_inside = fp_this_test;

    setTest(input,"inside time test",n_inside_time,&inside_time_shape);

    setTest(input,"hownear test",n_hownear,&hownear_shape);
    fp_hownear = fp_this_test;

    setTest(input,"hownear time test",n_hownear_time,&hownear_time_shape);

    setTest(input,"howfar test",n_howfar,&howfar_shape);
    fp_howfar = fp_this_test;

    setTest(input,"howfar time test",n_howfar_time,&howfar_time_shape);

}

void EGS_PrivateTester::setTest(EGS_Input *input, const char *delim, int &n,
                                EGS_BaseShape **s) {
    EGS_Input *i = input->takeInputItem(delim);
    if (!i) fprintf(fp_warn,"EGS_PrivateTester::EGS_setTest: \n"
                        "  no '%s' specification\n",delim);
    else {
        string shape_name;
        int ierr = i->getInput("bounding shape name",shape_name);
        EGS_BaseShape *shape = 0;
        if (!ierr) {
            shape = EGS_BaseShape::getShape(shape_name);
        }
        if (!shape) {
            EGS_Input *ishape = i->takeInputItem("bounding shape");
            if (!ishape) fprintf(fp_warn,"EGS_PrivateTester::EGS_setTest: \n"
                                     "  no 'bounding shape' definition for %s\n",delim);
            else {
                shape = EGS_BaseShape::createShape(ishape);
                delete ishape;
            }
        }
        if (!shape) fprintf(fp_warn,"EGS_PrivateTester::EGS_PrivateTester:"
                                "\n  got null shape for %s\n",delim);
        else {
            shape->ref();
            *s = shape;
        }
        int err = i->getInput("ntest",n);
        if (err) fprintf(fp_warn,"EGS_PrivateTester::setTest: \n"
                             "  missing/wrong 'ntest' input for %s\n",delim);
        string fname;
        fp_this_test = stdout;
        err = i->getInput("file name",fname);
        if (!err && fname.size() > 0) {
            fp_this_test = fopen(fname.c_str(),"w");
            if (!fp_this_test) {
                fprintf(fp_warn,"EGS_PrivateTester::setTest: \n"
                        "  failed to open file %s from writing\n",fname.c_str());
                fp_this_test = stdout;
            }
        }
        int ci;
        err = i->getInput("check infinity",ci);
        if (!err && ci == 0) {
            check_infinity = false;
        }

        string delimeter(delim);
        if (delimeter == "howfar time test") {
            string ss;
            i->getInput("store steps",ss);
            if (ss == "no" || ss == "0") {
                fprintf(fp_info,"will not store steps in howfar test\n");
                store_steps = false;
            }
        }

        delete i;
    }
}

void EGS_PrivateTester::testInside(EGS_BaseGeometry *g) {
    if (beginTest(n_inside,inside_shape,"testInside()","inside test",g)) {
        return;
    }
    int n_in = 0;
    fp_this_test = fp_inside;
    for (int j=0; j<n_inside; j++) {
        EGS_Vector x = inside_shape->getRandomPoint(rndm);
        int ireg = g->inside(x);
        if (ireg >= 0) {
            n_in++;
            parent->printPosition(x);
        }
    }
    fprintf(fp_info,"finished inside test. Point inside: %d (%g)\n",
            n_in,((double) n_in)/((double) n_inside));
}

void EGS_PrivateTester::testInsideTime(EGS_BaseGeometry *g) {
    if (beginTest(n_inside_time,inside_time_shape,"testInsideTime()",
                  "inside time test",g)) {
        return;
    }
    int n_in = 0;
    EGS_Timer t;
    for (int j=0; j<n_inside_time; j++) {
        EGS_Vector x = inside_time_shape->getRandomPoint(rndm);
        int ireg = g->inside(x);
        if (ireg >= 0) {
            n_in++;
        }
    }
    EGS_Float cpu = t.time();
    fprintf(fp_info,"finished inside time test.\n");
    fprintf(fp_info,"   point inside: %d (%g)\n",n_in,
            ((double) n_in)/((double) n_inside_time));
    fprintf(fp_info,"   cpu time: %g seconds\n",cpu);
}

void EGS_PrivateTester::testHownear(int ntry, EGS_BaseGeometry *g) {
    if (ntry < 1) {
        fprintf(fp_warn,"EGS_GeometryTester::testHownear(): ntry must be >0\n");
        return;
    }
    if (beginTest(n_hownear,hownear_shape,"testHownear()","hownear test",g)) {
        return;
    }
    int n_fail = 0;
    for (int j=0; j<n_hownear; j++) {
        EGS_Vector x = hownear_shape->getRandomPoint(rndm);
        int ireg = g->inside(x);
        EGS_Float tperp = g->hownear(ireg,x);
        for (int i=0; i<ntry; i++) {
            EGS_Float cost = 2*rndm->getUniform()-1;
            EGS_Float sint = tperp*sqrt(1-cost*cost);
            EGS_Float cphi, sphi;
            rndm->getAzimuth(cphi,sphi);
            EGS_Vector xi(x.x+sint*cphi,x.y+sint*sphi,x.z+tperp*cost);
            int ireg_i = g->inside(xi);
            if (ireg_i != ireg) {
                n_fail++;
                fprintf(fp_hownear," point (%g,%g,%g) with tperp=%g fails for "
                        "(%g,%g,%g): %d %d\n",x.x,x.y,x.z,tperp,xi.x,xi.y,xi.z,ireg,ireg_i);
            }
        }
    }
    fprintf(fp_info,"finished hownear test. number of failed points: %d\n",
            n_fail);
}

void EGS_PrivateTester::testHownearTime(EGS_BaseGeometry *g) {
    if (beginTest(n_hownear_time,hownear_time_shape,"testHownearTime()",
                  "hownear time test",g)) {
        return;
    }
    double sum_tperp = 0;
    EGS_Timer t;
    for (int j=0; j<n_hownear_time; j++) {
        EGS_Vector x = hownear_time_shape->getRandomPoint(rndm);
        int ireg = g->inside(x);
        sum_tperp += g->hownear(ireg,x);
    }
    EGS_Float cpu = t.time();
    fprintf(fp_info,"finished hownear time test.\n");
    fprintf(fp_info,"   average tperp: %g\n",sum_tperp/n_hownear_time);
    fprintf(fp_info,"   cpu time: %g seconds\n",cpu);
}


#define N_MAX_STEP 200

void EGS_PrivateTester::testHowfar(EGS_BaseGeometry *g, bool time) {
    EGS_Vector positions[N_MAX_STEP];
    int regions[N_MAX_STEP];
    int btest;
    int ncase;
    EGS_BaseShape *hshape;
    bool ss;
    if (time) {
        ncase = n_howfar_time;
        btest = beginTest(n_howfar_time,howfar_time_shape,"testHowfarTime()",
                          "howfar time test",g);
        hshape = howfar_time_shape;
        ss = store_steps;
        fprintf(fp_info,"Store steps: %d\n",ss);
    }
    else {
        ncase = n_howfar;
        btest = beginTest(n_howfar,howfar_shape,"testHowfar()","howfar test",g);
        hshape = howfar_shape;
        ss = true;
    }
    if (btest) {
        return;
    }
    const EGS_AffineTransform *T = hshape->getTransform();
    egsWarning("has transofrmation: %d\n",(T != 0));
    //EGS_BaseGeometry::geometry_error = &__geometry_error;
    EGS_Timer timer;
    fp_this_test = fp_howfar;
    double nstep = 0;
    for (int j=0; j<ncase; j++) {
        EGS_Vector x = hshape->getRandomPoint(rndm);
        EGS_Float cost = 2*rndm->getUniform()-1;
        EGS_Float sint = sqrt(1-cost*cost);
        EGS_Float cphi, sphi;
        rndm->getAzimuth(cphi,sphi);
        EGS_Vector u(sint*cphi,sint*sphi,cost);
        EGS_Vector Xo(x);
        bool go_back = true;
        //egsWarning("x = (%g,%g,%g) u = (%g,%g,%g)\n",x.x,x.y,x.z,u.x,u.y,u.z);
retry:
        //if( !go_back ) egsWarning("Initial position: (%g,%g,%g) direction: "
        //      "(%g,%g,%g)\n",x.x,x.y,x.z,u.x,u.y,u.z);
        EGS_Float t_step = 1e15;
        int ireg = g->inside(x);
        int ireg_first = ireg;
        ireg = g->howfar(ireg,x,u,t_step);
        if (ireg < 0 && ireg != ireg_first && !time) {
            EGS_Vector tmp(x + u*t_step);
            parent->printPosition(tmp);
        }
        //egsWarning("ireg: %d new_ireg: %d t = %g\n",ireg_first,ireg,t_step);
        int n_step = 0;
        if (ireg >= 0) {
            x += u*t_step;
            if (ss) {
                positions[n_step] = x;
                regions[n_step++] = ireg;
            }
            if (!time) {
                parent->printPosition(x);
            }
        }
        //if( !go_back && ireg >= 0 ) egsWarning("Starting loop: ireg=%d "
        //       "x=(%g,%g,%g)\n",ireg,x.x,x.y,x.z);
        while (ireg >= 0) {
            t_step = 1e15;
            int ireg_new = g->howfar(ireg,x,u,t_step);
            //if( !go_back ) egsWarning("new region=%d step=%g x=(%g,%g,%g)\n",
            //        ireg_new,t_step,x.x,x.y,x.z);
            //if( __geometry_error ) {
            //    egsWarning("Geometry error occured\n");
            //    __geometry_error = 0;
            //    if( go_back ) {
            //        go_back = false; x = Xo; goto retry;
            //    }
            //}
            if (ireg_new == ireg) {
                break;
            }
            // if the above condition is true,
            // we assume to be in an anfinite geometry with a direction
            // such that we never get out.
            ireg = ireg_new;
            x += u*t_step;
            nstep += 1;
            if (ss) {
                positions[n_step] = x;
                regions[n_step++] = ireg;
                if (n_step >= N_MAX_STEP) {
                    if (go_back) {
                        egsWarning("testHowfar(): number of steps exceeded %d for"
                                   " case %d:\n",N_MAX_STEP,j+1);
                        egsWarning("  direction = (%g,%g,%g)\n",u.x,u.y,u.z);
                        egsWarning("  initial position = (%g,%g,%g) region = %d\n",
                                   Xo.x,Xo.y,Xo.z,ireg_first);
                        for (int j=0; j<N_MAX_STEP; j++)
                            egsWarning("x = (%g,%g,%g) ireg = %d\n",positions[j].x,
                                       positions[j].y,positions[j].z,regions[j]);
                        go_back = false;
                        g->setDebug(true);
                        x = Xo;
                        goto retry;
                    }
                    egsFatal("\nQuiting now\n\n");
                }
            }
            if (!time) {
                if (check_infinity && x.length2() > 1e10) {
                    egsWarning("testHowfar(): x -> infinity for case %d?\n",
                               j+1);
                    egsWarning("  direction = (%g,%g,%g)\n",u.x,u.y,u.z);
                    egsWarning("  initial position = (%g,%g,%g) region = %d\n",
                               Xo.x,Xo.y,Xo.z,ireg_first);
                    for (int j=0; j<n_step; j++) egsWarning("x = (%g,%g,%g) "
                                                                "ireg = %d\n",positions[j].x,positions[j].y,
                                                                positions[j].z,regions[j]);
                }
                if (ireg >= 0) {
                    int itest = g->inside(x);
                    if (itest < 0) {
                        egsWarning("testHowfar(): after howfar step ireg = %d"
                                   " but inside() returns %d\n",ireg,itest);
                        egsWarning("  position = (%g,%g,%g)\n",x.x,x.y,x.z);
                        egsWarning("  direction = (%g,%g,%g)\n",u.x,u.y,u.z);
                        egsWarning("  initial position = (%g,%g,%g) "
                                   "region = %d\n",Xo.x,Xo.y,Xo.z,ireg_first);
                        for (int j=0; j<n_step; j++) egsWarning("x = (%g,%g,%g) "
                                                                    "ireg = %d\n",positions[j].x,positions[j].y,
                                                                    positions[j].z,regions[j]);
                        break;
                    }
                }
                parent->printPosition(x);
            }
        }
    }
    EGS_Float cpu = timer.time();
    if (time) {
        fprintf(fp_info,"finished howfar time test, cpu time = %g seconds\n",
                cpu);
        fprintf(fp_info,"  average number of steps: %g\n",nstep/ncase);
    }
    else {
        fprintf(fp_info,"finished howfar test.\n");
    }
}


int EGS_PrivateTester::beginTest(int n, const EGS_BaseShape *s,
                                 const char *func, const char *name, const EGS_BaseGeometry *g) {
    if (!s) {
        fprintf(fp_warn,"EGS_GeometryTester::%s: no bounding shape defined\n",
                func);
        return 1;
    }
    if (n <= 0) {
        fprintf(fp_warn,"EGS_GeometryTester::%s: number of test cases to run "
                "is less than 1 (%d)\n",func,n);
        return 2;
    }
    fprintf(fp_info,"\nEGS_GeometryTester::%s: starting %s\n",func,name);
    fprintf(fp_info,"   number of cases: %d\n",n);
    fprintf(fp_info,"   bounding shape: %s\n",s->getObjectType().c_str());
    //fprintf(fp_info,"   geometry:\n"); g->printInfo();
    fprintf(fp_info,"   geometry: %s (type %s)\n",g->getName().c_str(),
            g->getType().c_str());
    return 0;
}
#endif

#ifndef SKIP_DOXYGEN
class EGS_LOCAL EGS_CylTester : public EGS_GeometryTester {
public:
    EGS_CylTester(const EGS_Vector &A,EGS_Input *i) :
        EGS_GeometryTester(i), a(A) {
        a.normalize();
    };
    ~EGS_CylTester() {};
    void printPosition(const EGS_Vector &x) {
        EGS_Float z = x*a;
        EGS_Float r = sqrt(x.length2()-z*z);
        fprintf(p->fp_this_test,"%g %g\n",z,r);
    };
private:
    EGS_Vector a;
};

class EGS_LOCAL EGS_SphereTester : public EGS_GeometryTester {
public:
    EGS_SphereTester(const EGS_Vector &Xo, EGS_Input *i) :
        xo(Xo), EGS_GeometryTester(i) {};
    ~EGS_SphereTester() {};
    void printPosition(const EGS_Vector &x) {
        EGS_Vector xp(x-xo);
        EGS_Float r2 = xp.length2();
        fprintf(p->fp_this_test,"%g %g\n",xp.z,sqrt(r2-xp.z*xp.z));
    };
private:
    EGS_Vector xo;
};

class EGS_LOCAL EGS_TransformedTester : public EGS_GeometryTester {
public:
    EGS_TransformedTester(const EGS_AffineTransform &t, EGS_Input *i) :
        T(t), EGS_GeometryTester(i) {};
    ~EGS_TransformedTester() {};
    void printPosition(const EGS_Vector &x) {
        EGS_Vector xp(x*T);
        fprintf(p->fp_this_test,"%g %g %g\n",xp.x,xp.y,xp.z);
    };
private:
    EGS_AffineTransform T;
};
#endif

EGS_GeometryTester *EGS_GeometryTester::getGeometryTester(EGS_Input *input) {
    if (!input) {
        egsWarning("EGS_GeometryTester::getGeometryTester:\n"
                   "  input is null?\n");
        return 0;
    }
    bool delete_it = false;
    EGS_Input *i;
    if (input->isA("geometry tester")) {
        i = input;
    }
    else {
        i = input->takeInputItem("geometry tester");
        if (!i) {
            egsWarning("EGS_GeometryTester::getGeometryTester:\n"
                       "  no 'geometry tester' input\n");
            return 0;
        }
        delete_it = true;
    }
    EGS_Input *ishape;
    while ((ishape = i->takeInputItem("bounding shape"))) {
        egsWarning("*** adding shape\n");
        EGS_BaseShape::createShape(ishape);
        delete ishape;
    }
    string type;
    EGS_GeometryTester *tester;
    int err = i->getInput("output type",type);
    if (err || i->compare(type,"normal")) {
        tester = new EGS_GeometryTester(i);
    }
    else if (i->compare(type,"cylindrical")) {
        vector<EGS_Float> axis;
        int err1 = i->getInput("axis",axis);
        if (!err1 && axis.size() == 3) {
            tester = new EGS_CylTester(EGS_Vector(axis[0],axis[1],axis[2]),i);
        }
        else {
            egsWarning("EGS_GeometryTester::getGeometryTester: no 'axis'"
                       " input for cylindrical output type\n");
            tester = new EGS_GeometryTester(i);
        }
    }
    else if (i->compare(type,"spherical")) {
        vector<EGS_Float> Xo;
        int err1 = i->getInput("midpoint",Xo);
        if (!err1 && Xo.size() == 3) {
            tester = new EGS_SphereTester(EGS_Vector(Xo[0],Xo[1],Xo[2]),i);
        }
        else {
            tester = new EGS_SphereTester(EGS_Vector(),i);
        }
    }
    else if (i->compare(type,"transformed")) {
        EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(i);
        if (!t) {
            egsWarning("EGS_GeometryTester::getGeometryTester: no "
                       "transformation defined for a transformed tester\n");
            tester = new EGS_GeometryTester(i);
        }
        else {
            tester = new EGS_TransformedTester(*t,i);
            delete t;
        }
    }
    else {
        egsWarning("EGS_GeometryTester::getGeometryTester: unknown tester"
                   " type %s\n",type.c_str());
        tester = new EGS_GeometryTester(i);
    }
    if (delete_it) {
        delete i;
    }
    return tester;
}

