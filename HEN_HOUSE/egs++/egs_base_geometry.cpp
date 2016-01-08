/*
###############################################################################
#
#  EGSnrc egs++ base geometry
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
#  Contributors:    Frederic Tessier
#
###############################################################################
*/


/*! \file egs_base_geometry.cpp
 *  \brief Base geometry implementation
 *  \IK
 *
 *  Also provides a static geometry factory used to manage all constructed
 *  geometries and maintain a list of all media in all geometries.
 */
#include "egs_base_geometry.h"
#include "egs_functions.h"
#include "egs_library.h"
#include "egs_input.h"

#include <algorithm>
#include <vector>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

using namespace std;

typedef EGS_BaseGeometry *(*EGS_GeometryCreationFunction)(EGS_Input *);

#ifndef SKIP_DOXYGEN
/*!  \brief This class implements functionality related to the dynamic
  loading of geometry libraries, maintaining a list of geometries, etc.

  \interwarning
 */
class EGS_LOCAL EGS_GeometryPrivate {
public:

    int nnow, ntot;
    EGS_BaseGeometry **geoms;
    vector<string> media;
    vector<EGS_Library *> glibs;
    static string geom_delimeter;
    static string libkey;
    static string create_key;
    string dso_path;

    EGS_GeometryPrivate() : nnow(0), ntot(0), geoms(0) {
        //egsInformation("EGS_GeometryPrivate() at 0x%x\n",this);
        setUp();
    };

    EGS_GeometryPrivate(const EGS_GeometryPrivate &p) :
        nnow(0), ntot(0), geoms(0) {
        //egsInformation("EGS_GeometryPrivate(0x%x) at 0x%x\n",&p,this);
        setUp();
    };

    void setUp() {
        char *hhouse = getenv("HEN_HOUSE");
        if (!hhouse) {
            egsFatal("Environment variable HEN_HOUSE must be defined\n");
        }
        dso_path = hhouse;
#if defined WIN32 && !defined CYGWIN
        char c = '\\';
#else
        char c = '/';
#endif
        if (dso_path[dso_path.size()-1] != c) {
            dso_path += c;
        }
        //dso_path += "geometry"; dso_path += c;
        dso_path += "egs++";
        dso_path += c;
        dso_path += "dso";
        dso_path += c;
        dso_path += CONFIG_NAME;
    };

    ~EGS_GeometryPrivate() {
        //egsInformation("Destructing EGS_GeometryPrivate at 0x%x, "
        //        "ntot=%d nnow=%d\n",this,ntot,nnow);
        if (!ntot) {
            return;
        }
        clearGeometries();
        delete [] geoms;
        //egsInformation("Deleting geometry libs\n");
        {
            for (unsigned int j=0; j<glibs.size(); j++) {
                delete glibs[j];
            }
        }
    };

    void clearGeometries() {
        media.clear();
        if (!ntot) {
            return;
        }
        int j = 0, iloop = 0;
        while (nnow > 0) {
            if (geoms[j]->deref() == -1) {
                delete geoms[j];
            }
            else {
                geoms[j++]->ref();
            }
            if (j >= nnow && nnow) {
                j = 0;
                ++iloop;
                if (iloop > 20) egsWarning("~EGS_GeometryPrivate(): failed "
                                               "to delete all geometries after 20 loops!\n");
                break;
            }
        }
    };

    void grow(int ngrow) {
        ntot += ngrow;
        EGS_BaseGeometry **tmp = new EGS_BaseGeometry* [ntot];
        for (int j=0; j<nnow; j++) {
            tmp[j] = geoms[j];
        }
        if (geoms) {
            delete [] geoms;
        }
        geoms = tmp;
    };

    int addGeometry(EGS_BaseGeometry *g) {
        if (!g) {
            return -1;
        }
        for (int j=0; j<nnow; j++)
            if (geoms[j]->getName() == g->getName()) {
                return -1;
            }
        if (nnow >= ntot) {
            grow(10);
        }
        geoms[nnow++] = g;
        return nnow-1;

    };

    void removeGeometry(EGS_BaseGeometry *g) {
        for (int j=0; j<nnow; j++) {
            if (geoms[j] == g) {
                geoms[j] = geoms[--nnow];
                break;
            }
        }
    };

    EGS_BaseGeometry *getGeometry(const string &name) {
        for (int j=0; j<nnow; j++)
            if (geoms[j]->getName() == name) {
                return geoms[j];
            }
        return 0;
    };

    int addMedium(const string &Name) {
        if (EGS_Input::compare(Name,"vacuum")) {
            return -1;
        }
        for (unsigned int j=0; j<media.size(); j++)
            if (media[j] == Name) {
                return j;
            }
        media.push_back(Name);
        return media.size()-1;
    };

    int getMediumIndex(const string &Name) {
        for (unsigned int j=0; j<media.size(); j++)
            if (media[j] == Name) {
                return j;
            }
        return -1;
    };

    int nMedia() const {
        return media.size();
    };

    const char *getMediumName(int ind) const {
        if (ind < 0 || ind > media.size()-1) {
            return 0;
        }
        return media[ind].c_str();
    };

    EGS_BaseGeometry *createSingleGeometry(EGS_Input *i);

};

class EGS_LOCAL EGS_PrivateGeometryLists {
public:
    EGS_PrivateGeometryLists() : nnow(0), ntot(0) {
        addList(new EGS_GeometryPrivate);
    };
    ~EGS_PrivateGeometryLists() {
        //egsInformation("Deleting geometry lists\n");
        if (ntot > 0) {
            for (int j=0; j<nnow; j++) {
                //egsInformation("Deleting list %d\n",j);
                delete lists[j];
            }
            delete [] lists;
        }
    };
    int size() const {
        return nnow;
    };
    EGS_GeometryPrivate &operator[](int j) {
        return *lists[j];
    };
    void addList(EGS_GeometryPrivate *l) {
        if (!l) {
            return;
        }
        if (nnow >= ntot) {
            EGS_GeometryPrivate **tmp = new EGS_GeometryPrivate* [ntot+10];
            for (int j=0; j<nnow; j++) {
                tmp[j] = lists[j];
            }
            if (ntot > 0) {
                delete [] lists;
            }
            lists = tmp;
            ntot += 10;
        }
        lists[nnow++] = l;
    };
    int nnow, ntot;
    EGS_GeometryPrivate **lists;
};

string EGS_GeometryPrivate::geom_delimeter = "geometry definition";
string EGS_GeometryPrivate::libkey = "library";
string EGS_GeometryPrivate::create_key = "createGeometry";
#endif

int EGS_BaseGeometry::active_glist = 0;

static char buf_unique[32];

int EGS_BaseGeometry::error_flag = 0;

#ifdef SINGLE
    EGS_Float EGS_BaseGeometry::epsilon = 1e-5;
#else
    EGS_Float EGS_BaseGeometry::epsilon = 1e-7;
#endif

#ifndef SKIP_DOXYGEN
EGS_BaseGeometry *EGS_GeometryPrivate::createSingleGeometry(EGS_Input *i) {
    string libname;
    if (!i) {
        egsWarning("createSingleGeometry: null input?\n");
        return 0;
    }
    int error = i->getInput(EGS_GeometryPrivate::libkey,libname);
    if (error) {
        egsWarning("createSingleGeometry: input item %s does not define the"
                   " geometry library\n",i->name());
        return 0;
    }
    EGS_Library *lib = 0;
    for (unsigned int j=0; j<glibs.size(); j++) {
        if (libname == glibs[j]->libraryName()) {
            lib = glibs[j];
            break;
        }
    }
    if (!lib) {
        lib = new EGS_Library(libname.c_str(),dso_path.c_str());
        lib->load();
        if (!lib->isLoaded()) {
            egsWarning("createSingleGeometry: Failed to load library '%s' from"
                       " %s\n",libname.c_str(),dso_path.c_str());
            return 0;
        }
        glibs.push_back(lib);
    }
    EGS_GeometryCreationFunction gcreate = (EGS_GeometryCreationFunction)
                                           lib->resolve(EGS_GeometryPrivate::create_key.c_str());
    if (!gcreate) {
        egsWarning("createSingleGeometry: failed to resolve the %s function\n"
                   "  in geometry library %s\n",
                   EGS_GeometryPrivate::create_key.c_str(),lib->libraryName());
        return 0;
    }
    EGS_BaseGeometry *g = gcreate(i);
    if (!g) {
        egsWarning("createSingleGeometry: got null geometry\n");
        egsWarning("  library: %s\n",lib->libraryName());
        egsWarning("  input:\n");
        i->print(4,cerr);
        return 0;
    }
    if (!addGeometry(g)) {
        egsWarning("createSingleGeometry: failed to add the geometry %s\n"
                   " to the list of geometries. This implies that a geometry with"
                   " this name already exists\n",g->getName().c_str());
        delete g;
        return 0;
    }
    return g;

}
#endif

//static EGS_LOCAL vector<EGS_GeometryPrivate> egs_geometries;
static EGS_LOCAL EGS_PrivateGeometryLists egs_geometries;

EGS_Float EGS_BaseGeometry::howfarToOutside(int ireg, const EGS_Vector &x,
        const EGS_Vector &u) {
    if (ireg < 0) {
        return 0;
    }
    EGS_Vector xx(x);
    EGS_Float ttot = 0;
    while (1) {
        EGS_Float t = 1e30;
        int inew = howfar(ireg,xx,u,t);
        ttot += t;
        if (inew < 0) {
            break;
        }
        xx += u*t;
        ireg = inew;
    }
    return ttot;
}

void EGS_BaseGeometry::setActiveGeometryList(int list) {
    int n = egs_geometries.size();
    //egsInformation("EGS_BaseGeometry::setActiveGeometryList: size=%d list=%d\n",
    //        n,list);
    for (int j=n; j<=list; j++)
        //egs_geometries.push_back(EGS_GeometryPrivate());
    {
        egs_geometries.addList(new EGS_GeometryPrivate);
    }
    active_glist = list;
}

/*
extern "C" void __list_geometries() {
    egsInformation("The following %d geometries are defined:\n",
            egs_geometries[active_glist].nnow);
    for(int j=0; j<egs_geometries[active_glist].nnow; j++) {
        egsInformation("%d %s %s\n",j,
                egs_geometries[active_glist].geoms[j]->getType().c_str(),
                egs_geometries[active_glist].geoms[j]->getName().c_str());
    }
}
*/

EGS_BaseGeometry::EGS_BaseGeometry(const string &Name) : nreg(0), name(Name),
    med(-1), region_media(0), nref(0), debug(false), is_convex(true),
    has_rho_scaling(false), rhor(0), bproperty(0), bp_array(0) {
    if (!egs_geometries.size()) {
        egs_geometries.addList(new EGS_GeometryPrivate);
    }
    if (!name.size()) {
        name = getUniqueName();
    }
    if (egs_geometries[active_glist].addGeometry(this) < 0)
        egsFatal("EGS_BaseGeometry::EGS_BaseGeometry:\n"
                 "  a geometry with name %s alread exists\n",name.c_str());
}

EGS_BaseGeometry::~EGS_BaseGeometry() {
    if (region_media) {
        delete [] region_media;
    }
    if (rhor && has_rho_scaling) {
        delete [] rhor;
    }
    if (bp_array) {
        delete [] bp_array;
    }
    //egsInformation("Deleting geometry at 0x%x, list=%d\n",this,active_glist);
    egs_geometries[active_glist].removeGeometry(this);
}

void EGS_BaseGeometry::clearGeometries() {
    egs_geometries[active_glist].clearGeometries();
}

EGS_BaseGeometry *EGS_BaseGeometry::getGeometry(const string &Name) {
    return egs_geometries[active_glist].getGeometry(Name);
}

void EGS_BaseGeometry::setMedium(const string &Name) {
    med = egs_geometries[active_glist].addMedium(Name);
    if (region_media)
        for (int j=0; j<nreg; j++) {
            region_media[j] = med;
        }
}

int EGS_BaseGeometry::addMedium(const string &medname) {
    return egs_geometries[active_glist].addMedium(medname);
}

int EGS_BaseGeometry::getMediumIndex(const string &medname) {
    return egs_geometries[active_glist].getMediumIndex(medname);
}

void EGS_BaseGeometry::setMedium(int istart, int iend, const string &Name,
                                 int delta) {
    int imed = egs_geometries[active_glist].addMedium(Name);
    setMedium(istart,iend,imed,delta);
}

void EGS_BaseGeometry::setMedium(int istart, int iend, int imed, int delta) {
    if (nreg <= 1) {
        med = imed;
        return;
    }
    if (delta <= 0) {
        return;
    }
    if (istart < 0) {
        istart = 0;
    }
    if (iend > nreg-1) {
        iend = nreg-1;
    }
    if (!region_media) {
        region_media = new short [nreg];
        for (int j=0; j<nreg; j++) {
            region_media[j] = med;
        }
    }
    for (int j=istart; j<=iend; j+=delta) {
        region_media[j] = imed;
    }
}

int EGS_BaseGeometry::nMedia() {
    return egs_geometries[active_glist].nMedia();
}

const char *EGS_BaseGeometry::getMediumName(int ind) {
    return egs_geometries[active_glist].getMediumName(ind);
}

EGS_BaseGeometry *EGS_BaseGeometry::createSingleGeometry(EGS_Input *input) {
    return egs_geometries[active_glist].createSingleGeometry(input);
}

EGS_BaseGeometry *EGS_BaseGeometry::createGeometry(EGS_Input *input) {
    EGS_Input *ginput = input;
    bool delete_it = false;
    if (!input->isA(egs_geometries[active_glist].geom_delimeter)) {
        ginput = input->takeInputItem(egs_geometries[active_glist].geom_delimeter);
        delete_it = true;
    }
    if (!ginput) {
        egsWarning("EGS_BaseGeometry::createGeometry: no geometry specification"
                   " in this input\n");
        return 0;
    }
    EGS_Input *ij;
    bool error = false;
    while ((ij = ginput->takeInputItem("geometry")) != 0) {
        EGS_BaseGeometry *g = egs_geometries[active_glist].createSingleGeometry(ij);
        if (!g) {
            error = true;
        }
        delete ij;
    }
    if (error) {
        egsFatal("EGS_BaseGeometry::createGeometry: errors during geometry"
                 " definition\n");
        return 0;
    }
    string sim_geom;
    int err = ginput->getInput("simulation geometry",sim_geom);
    if (err) {
        egsWarning("EGS_BaseGeometry::createGeometry: missing/wrong keyword"
                   " 'simulation geometry'\n");
        return 0;
    }
    EGS_BaseGeometry *g = egs_geometries[active_glist].getGeometry(sim_geom);
    if (!g) egsWarning("EGS_BaseGeometry::createGeometry: a geometry with "
                           "the name %s does not exist\n",sim_geom.c_str());
    if (delete_it) {
        delete ginput;
    }
    return g;
}

string EGS_BaseGeometry::getUniqueName() {
    sprintf(buf_unique,"geometry%d",egs_geometries[active_glist].nnow);
    string result(buf_unique);
    return result;
}

void EGS_BaseGeometry::setName(EGS_Input *i) {
    int err = i->getInput("name",name);
    if (err) {
        name = getUniqueName();
    }
    EGS_Input *inp;
    int irep=0;
    while ((inp = i->takeInputItem("replica"))) {
        string typ;
        int ncopy;
        vector<EGS_Float> trans, trans_o;
        vector<EGS_Float> rot_axis;
        EGS_Float rot_angle, rot_angle_o;
        int err1 = inp->getInput("type",typ);
        int err2 = inp->getInput("number of copies",ncopy);
        int err3 = inp->getInput("translation delta",trans);
        int err3a = inp->getInput("first translation",trans_o);
        int err4 = inp->getInput("rotation axis",rot_axis);
        int err5 = inp->getInput("rotation delta",rot_angle);
        int err5a = inp->getInput("first rotation",rot_angle_o);
        bool do_it = true;
        int ttype;
        if (err1 || err2) {
            if (err1) egsWarning("geometry replication: 'type' not defined ->"
                                     " ignoring input\n");
            if (err2) egsWarning("geometry replication: 'number of copies' "
                                     "not defined -> ignoring input\n");
            do_it = false;
        }
        else {
            if (ncopy < 1) {
                egsWarning("geometry replication: %d copies?\n",ncopy);
                do_it = false;
            }
            if (typ == "line") {
                if (trans.size() != 3) {
                    egsWarning("geometry replication: got %d inputs for "
                               "'translation', need 3\n",trans.size());
                    do_it = false;
                }
                else {
                    if (err3a) {
                        trans_o.push_back(0);
                        trans_o.push_back(0);
                        trans_o.push_back(0);
                    }
                }
                ttype = 0;
            }
            else if (typ == "rotation") {
                if (rot_axis.size() != 3) {
                    egsWarning("geometry replication: got %d inputs for "
                               "'rotation axis', need 3\n",rot_axis.size());
                    do_it = false;
                }
                if (err4) {
                    egsWarning("geometry replication: missing 'rotation delta'"
                               " input\n");
                    do_it = false;
                }
                if (err5a) {
                    rot_angle_o = 0;
                }
                ttype = 1;
            }
            else {
                egsWarning("geometry replication: unknown replica type %s\n",
                           typ.c_str());
                do_it = false;
            }
        }
        if (do_it) {
            ++irep;
            char buf[1024];
            for (int icopy=1; icopy<=ncopy; icopy++) {
                //string content(":start geometry:\n");
                string content;
                content += "    library = egs_gtransformed\n";
                sprintf(buf,"    name = %s_rep%d_%d\n",name.c_str(),irep,icopy);
                content += buf;
                sprintf(buf,"    my geometry = %s\n",name.c_str());
                content += buf;
                content += "    :start transformation:\n";
                if (ttype == 0)
                    sprintf(buf,"        translation = %g %g %g\n",
                            trans_o[0]+trans[0]*icopy,
                            trans_o[1]+trans[1]*icopy,
                            trans_o[2]+trans[2]*icopy);
                else
                    sprintf(buf,"        rotation = %g %g %g  %g\n",
                            rot_axis[0],rot_axis[1],rot_axis[2],
                            rot_angle_o+rot_angle*icopy);
                content += buf;
                content += "    :stop transformation:\n";
                //content += ":stop geometry:\n";
                EGS_Input aux;
                aux.setContentFromString(content);
                EGS_BaseGeometry *g =
                    EGS_BaseGeometry::createSingleGeometry(&aux);
                if (!g) egsWarning("geometry replication: failed to create"
                                       " replica %d of %s\n",icopy,name.c_str());
                //else egsInformation("geometry replication: created replica"
                //       " %s\n",g->getName().c_str());
            }
        }
    }
}

void EGS_BaseGeometry::printInfo() const {
    egsInformation("======================== geometry =====================\n");
    egsInformation(" type = %s\n",getType().c_str());
    egsInformation(" name = %s\n",getName().c_str());
    egsInformation(" number of regions = %d\n",nreg);
}

void EGS_BaseGeometry::describeGeometries() {
    egsInformation("\nThe following geometries are defined:\n\n");
    for (int j=0; j<egs_geometries[active_glist].nnow; j++) {
        egs_geometries[active_glist].geoms[j]->printInfo();
    }
}

void EGS_BaseGeometry::setMedia(EGS_Input *inp) {
    EGS_Input *input = inp;
    bool delete_it = false;
    if (!input->isA("media input")) {
        input = inp->takeInputItem("media input");
        if (!input) {
            return;
        }
        // i.e., if there is no media related input for this geometry,
        // we don't warn as we assume that media will be set from the outside
        delete_it = true;
    }
    vector<string> media_names;
    int err = input->getInput("media",media_names);
    int *med_ind = 0;
    int nmed = media_names.size();
    if (!err && nmed > 0) {
        med_ind = new int [nmed];
        for (int j=0; j<nmed; j++) {
            med_ind[j] = egs_geometries[active_glist].addMedium(media_names[j]);
        }
    }
    else {
        nmed = nMedia();
        if (nmed < 1) {
            if (delete_it) {
                delete input;
            }
            return;
        }
        med_ind = new int [nmed];
        for (int j=0; j<nmed; j++) {
            med_ind[j] = j;
        }
    }
    setMedia(input,nmed,med_ind);
    setRelativeRho(input);
    delete [] med_ind;
    if (delete_it) {
        delete input;
    }
}

void EGS_BaseGeometry::setMedia(EGS_Input *input, int nmed, const int *mind) {
    EGS_Input *i;
    med = mind[0];
    while ((i = input->takeInputItem("set medium"))) {
        vector<int> inp;
        int err = i->getInput("set medium",inp);
        delete i;
        if (!err) {
            //if( inp.size() == 2 ) setMedium(inp[0],inp[0]+1,mind[inp[1]]);
            if (inp.size() == 2) {
                setMedium(inp[0],inp[0],mind[inp[1]]);
            }
            else if (inp.size() == 3) {
                setMedium(inp[0],inp[1],mind[inp[2]]);
            }
            else if (inp.size() == 4) {
                setMedium(inp[0],inp[1],mind[inp[2]],inp[3]);
            }
            else egsWarning("EGS_BaseGeometry::setMedia(): found %d inputs\n"
                                "in a 'set medium' input. 2 or 3 are allowed\n",inp.size());
        }
        else egsWarning("EGS_BaseGeometry::setMedia(): wrong 'set medium'"
                            " input\n");
    }
}

void EGS_BaseGeometry::setRelativeRho(int start, int end, EGS_Float rho) {
    if (start < 0) {
        start = 0;
    }
    if (end >= nreg) {
        end = nreg-1;
    }
    if (end >= start) {
        int j;
        if (!rhor) {
            rhor = new EGS_Float [nreg];
            for (j=0; j<nreg; j++) {
                rhor[j] = 1;
            }
        }
        for (j=start; j<=end; j++) {
            rhor[j] = rho;
        }
        has_rho_scaling = true;
    }
}

void EGS_BaseGeometry::setRelativeRho(EGS_Input *input) {
    EGS_Input *i;
    while ((i = input->takeInputItem("set relative density"))) {
        vector<EGS_Float> tmp;
        int err = i->getInput("set relative density",tmp);
        if (!err) {
            if (tmp.size() == 2) {
                int start = (int)(tmp[0]+0.1);
                int end = start;
                setRelativeRho(start,end,tmp[1]);
            }
            else if (tmp.size() == 3) {
                int start = (int)(tmp[0]+0.1);
                int end = (int)(tmp[1]+0.1);
                setRelativeRho(start,end,tmp[2]);
            }
            else {
                egsWarning("EGS_BaseGeometry::setRelativeRho(): found %d "
                           "inputs in a 'set relative density' input.\n",tmp.size());
                egsWarning("  2 or 3 are allowed => input ignored\n");
            }
        }
        delete i;
    }
}

int EGS_BaseGeometry::computeIntersections(int ireg, int n, const EGS_Vector &X,
        const EGS_Vector &u, EGS_GeometryIntersections *isections) {
    if (n < 1) {
        return -1;
    }
    int ifirst = 0;
    EGS_Float t, ttot = 0;
    EGS_Vector x(X);
    int imed;
    if (ireg < 0) {
        t = 1e30;
        ireg = howfar(ireg,x,u,t,&imed);
        if (ireg < 0) {
            return 0;
        }
        isections[0].t = t;
        isections[0].rhof = 1;
        isections[0].ireg = -1;
        isections[0].imed = -1;
        ttot = t;
        ++ifirst;
        x += u*t;
    }
    else {
        imed = medium(ireg);
    }

    for (int j=ifirst; j<n; j++) {
        isections[j].imed = imed;
        isections[j].rhof = getRelativeRho(ireg);
        isections[j].ireg = ireg;
        t = 1e30;
        int inew = howfar(ireg,x,u,t,&imed);
        ttot += t;
        isections[j].t = ttot;
        if (inew < 0 || inew == ireg) {
            return j+1;
        }
        ireg = inew;
        x += u*t;
    }

    return ireg >= 0 ? -1 : n;

}

void EGS_BaseGeometry::setBooleanProperty(EGS_BPType prop) {
    bproperty = prop;
    if (bp_array) {
        delete [] bp_array;
        bp_array = 0;
    }
}

void EGS_BaseGeometry::addBooleanProperty(int bit) {
    if (bit < 0 || bit >= 8*sizeof(EGS_BPType)) {
        egsWarning("EGS_BaseGeometry::addBooleanProperty: attempt to set the "
                   "%d'th bith!\n",bit);
        return;
    }
    EGS_BPType prop = 1 << bit;
    bproperty |= prop;
    if (bp_array) {
        for (int j=0; j<nreg; j++) {
            bp_array[j] |= prop;
        }
    }
}

void EGS_BaseGeometry::setBooleanProperty(EGS_BPType prop, int start, int end,
        int step) {
    if (start < 0) {
        start = 0;
    }
    if (end >= nreg) {
        end = nreg-1;
    }
    if (start == 0 && end == nreg-1 && step==1) {
        setBooleanProperty(prop);
    }
    else {
        if (!bp_array) {
            bp_array = new EGS_BPType [nreg];
            for (int j=0; j<nreg; j++) {
                bp_array[j] = 0;
            }
        }
        for (int j=start; j<=end; j+=step) {
            bp_array[j] = prop;
        }
    }
}

void EGS_BaseGeometry::addBooleanProperty(int bit, int start, int end,
        int step) {
    if (bit < 0 || bit >= 8*sizeof(EGS_BPType)) {
        egsWarning("EGS_BaseGeometry::addBooleanProperty: attempt to set the "
                   "%d'th bith!\n",bit);
        return;
    }
    if (start < 0) {
        start = 0;
    }
    if (end >= nreg) {
        end = nreg-1;
    }
    if (start == 0 && end == nreg-1 && step==1) {
        addBooleanProperty(bit);
    }
    else {
        EGS_BPType prop = 1 << bit;
        if (!bp_array) {
            bp_array = new EGS_BPType [nreg];
            for (int j=0; j<nreg; j++) {
                bp_array[j] = 0;
            }
        }
        for (int j=start; j<=end; j+=step) {
            bp_array[j] |= prop;
        }
    }
}

void EGS_BaseGeometry::getLabelRegions(const string &str, vector<int> &regs) {

    // get all regions lists for this named label
    for (int i=0; i<labels.size(); i++) {
        if (labels[i].name.compare(str) == 0) {
            regs.insert(regs.end(), labels[i].regions.begin(), labels[i].regions.end());
        }
    }

    // sort region list and remove duplicates
    sort(regs.begin(), regs.end());
    regs.erase(unique(regs.begin(), regs.end()), regs.end());
}


int EGS_BaseGeometry::setLabels(EGS_Input *input) {
    EGS_Input *i;
    int labelCount=0;
    while ((i = input->takeInputItem("set label"))) {

        // get input string
        string inp;
        int err = i->getInput("set label",inp);
        delete i;

        // bail out on read error
        if (err) {
            egsWarning("EGS_BaseGeometry::setLabels(): error while reading 'set label' input\n");
            return 0;
        }

        // set the labels from input string
        int nLabel = setLabels(inp);

        // increase label count
        labelCount += nLabel;
    }

    return labelCount;
}


int EGS_BaseGeometry::setLabels(const string &inp) {

    // tokenize input string
    vector<string> tokens;
    const char *ptr = inp.c_str();
    do {
        const char *begin = ptr;
        while (*ptr != ' ' && *ptr) {
            ptr++;
        }
        tokens.push_back(string(begin, ptr));
    }
    while (*ptr++ != '\0');

    // bail out if there are no label tokens
    if (tokens.size() < 1) {
        egsWarning("EGS_BaseGeometry::setLabels(): no label name\n");
        return 0;
    }

    // parse label into a label class
    label lab;
    lab.name = tokens[0];
    for (int i=1; i<tokens.size(); i++) {
        int reg = atoi(tokens[i].c_str());
        if (reg < nreg && reg >= 0) {
            lab.regions.push_back(reg);
        }
        else {
            egsWarning("EGS_BaseGeometry::setLabels(): label \"%s\": region %d is beyond the number " \
                       "of regions in this geometry\n", lab.name.c_str(), reg);
        }
    }

    // continue if there is no region
    if (lab.regions.size() <= 0) {
        return 0;
    }

    // sort region list and remove duplicates
    sort(lab.regions.begin(), lab.regions.end());
    lab.regions.erase(unique(lab.regions.begin(), lab.regions.end()), lab.regions.end());

    // push current label onto vector of labels
    labels.push_back(lab);

    return 1;
}


