/*
###############################################################################
#
#  EGSnrc egs++ input
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
#  Contributors:    Ernesto Mainegra-Hing
#                   Frederic Tessier
#
###############################################################################
*/


/*! \file egs_input.cpp
 *  \brief EGS_Input implementation
 *  \IK
 */

#include "egs_input.h"
#include "egs_functions.h"

#ifdef NO_SSTREAM
    #include <strstream>
    #define S_STREAM std::istrstream
#else
    #include <sstream>
    #define S_STREAM std::istringstream
#endif

#include <algorithm>
#include <cctype>
#include <fstream>

using namespace std;

static char start_key_begin[] = ":START";
static char stop_key_begin[] = ":STOP";
static char start_key_end[] = ":";
static char stop_key_end[] = ":";
static char indent_space[] = "    ";

#ifndef SKIP_DOXYGEN
/*! \brief Private implementation of the EGS_Input functionality.

    \internwarning
*/
class EGS_LOCAL EGS_InputPrivate {
public:
    string        key;
    string        value;
    vector<EGS_InputPrivate *> children;
    int nref;

    EGS_InputPrivate() : nref(0) {};
    EGS_InputPrivate(const string &Key, const string &Val = "") : key(Key),
        value(Val), children(), nref(0) { };
    EGS_InputPrivate(const EGS_InputPrivate &p, bool deep=false) :
        key(p.key), value(p.value), nref(0), children() {
        for (unsigned int j=0; j<p.children.size(); j++) {
            if (deep) {
                children.push_back(new EGS_InputPrivate(*p.children[j],deep));
            }
            else {
                children.push_back(p.children[j]);
                children[j]->nref++;
            }
        }
    }

    ~EGS_InputPrivate() {
        for (unsigned int j=0; j<children.size(); j++) {
            deleteItem(children[j]);
        }
    };

    int replace(const string &replace_what, const string &replace_with);

    int setContentFromFile(const char *fname);
    int setContentFromString(string &input);
    int setContent(istream &input);

    int addContentFromFile(const char *fname);
    int addContentFromString(string &input);
    int addContent(istream &input);

    void processInputLoop(EGS_InputPrivate *p);

    void addItem(EGS_InputPrivate *p) {
        p->nref++;
        children.push_back(p);
    };

    EGS_InputPrivate *takeInputItem(const string &key, bool self=true) {
        if (self && isA(key)) {
            return this;
        }
        for (vector<EGS_InputPrivate *>::iterator it=children.begin();
                it != children.end(); it++) {
            if (compareKeys((*it)->key,key)) {
                EGS_InputPrivate *res = *it;
                children.erase(it);
                return res;
            }
            //if( (*it)->key == key ) {
            //    EGS_InputPrivate *res = *it;
            //    children.erase(it); return res;
            //}
        }
        return 0;
    };

    EGS_InputPrivate *getInputItem(const string &Key) {
        if (isA(Key)) {
            return this;
        }
        for (unsigned int j=0; j<children.size(); j++)
            if (compareKeys(children[j]->key,Key)) {
                return children[j];
            }
        return 0;
    };

    bool isA(const string &Key) const {
        return compareKeys(key,Key);
    };

    static void removeComment(const string &start, const string &end,
                              string &input, bool newline);

    static int findStart(int start, int stop,
                         const string &start_key, const string &end_key,
                         const string &input, string &what, int &end);
    static int findStop(int start, const string &start_string,
                        const string &end_string, const string &input, int &ie);

    static bool compareKeys(const string &s1, const string &s2);

    void print(int nind, ostream &) const;

    void removeEmptyLines(string &input);

    static void deleteItem(EGS_InputPrivate *p) {
        if (p) {
            if (!p->nref) {
                delete p;
            }
            else {
                p->nref--;
            }
        }
    };

};
#endif

EGS_Input::EGS_Input() {
    p = 0;
}

EGS_Input::EGS_Input(const EGS_Input &input) {
    if (input.p) {
        p = input.p;
        p->nref++;
    }
    else {
        p = 0;
    }
}

EGS_Input::EGS_Input(const string &name, const string &value) {
    p = new EGS_InputPrivate(name,value);
}

EGS_Input::~EGS_Input() {
    EGS_InputPrivate::deleteItem(p);
}

int EGS_Input::setContentFromFile(const char *fname) {
    EGS_InputPrivate::deleteItem(p);
    p = new EGS_InputPrivate;
    return p->setContentFromFile(fname);
}

int EGS_Input::setContentFromString(string &input) {
    EGS_InputPrivate::deleteItem(p);
    p = new EGS_InputPrivate;
    return p->setContentFromString(input);
}

int EGS_Input::addContentFromFile(const char *fname) {
    if (!p) {
        p = new EGS_InputPrivate;
    }
    return p->addContentFromFile(fname);
}

int EGS_Input::addContentFromString(string &input) {
    if (!p) {
        p = new EGS_InputPrivate;
    }
    return p->addContentFromString(input);
}

EGS_Input *EGS_Input::takeInputItem(const string &key, bool self) {
    if (!p) {
        return 0;
    }
    EGS_InputPrivate *item = p->takeInputItem(key,self);
    if (!item) {
        return 0;
    }
    if (item == p && !self) {
        return 0;
    }
    EGS_Input *result = new EGS_Input;
    result->p = item;
    if (item == p) {
        p = 0;
    }
    return result;
}

EGS_Input *EGS_Input::getInputItem(const string &key) const {
    if (!p) {
        return 0;
    }
    EGS_InputPrivate *item = p->getInputItem(key);
    if (!item) {
        return 0;
    }
    item->nref++;
    EGS_Input *result = new EGS_Input;
    result->p = item;
    return result;
}

void EGS_Input::addInputItem(const EGS_Input &input) {
    if (!input.p) {
        return;
    }
    if (!p) {
        p = new EGS_InputPrivate(*input.p);
    }
    else {
        p->addItem(input.p);
    }
}

const char *EGS_Input::name() const {
    if (!p) {
        return 0;
    }
    return p->key.c_str();
}

bool EGS_Input::isA(const string &key) const {
    if (!p) {
        return false;
    }
    return p->isA(key);
}

template <class T> int EGS_LOCAL
get_input(const EGS_InputPrivate *p, const string &key, vector<T> &values) {
    if (!p) {
        return -1;
    }
    const EGS_InputPrivate *p1;
    if (!p->children.size()) {
        if (!p->isA(key)) {
            return -1;
        }
        p1 = p;
    }
    else {
        for (unsigned int j=0; j<p->children.size(); j++) {
            p1 = p->children[j];
            if (p1->isA(key)) {
                break;
            }
            p1 = 0;
        }
        if (!p1) {
            return -1;
        }
    }
    values.erase(values.begin(),values.end());
    S_STREAM in(p1->value.c_str());
    int error = 0;
    while (1) {
        T tmp;
        in >> tmp;
        if (!in.fail()) {
            values.push_back(tmp);
        }
        if (in.eof()) {
            if (values.size() <= 0) {
                error = 1;
            }
            break;
        }
        if (!in.good()) {
            if (values.size() <= 0) {
                error = 2;
            }
            break;
        }
    }
    return error;
}

int EGS_Input::getInput(const string &key, vector<string> &values) const {
    return get_input(p,key,values);
}

int EGS_Input::getInput(const string &key, vector<EGS_Float> &values) const {
    return get_input(p,key,values);
}

int EGS_Input::getInput(const string &key, vector<int> &values) const {
    return get_input(p,key,values);
}

template <class T> int EGS_LOCAL
get_input(const EGS_InputPrivate *p, const string &key, T &value) {
    if (!p) {
        return -1;
    }
    const EGS_InputPrivate *p1;
    if (!p->children.size()) {
        if (!p->isA(key)) {
            return -1;
        }
        p1 = p;
    }
    else {
        for (unsigned int j=0; j<p->children.size(); j++) {
            p1 = p->children[j];
            if (p1->isA(key)) {
                break;
            }
            p1 = 0;
        }
        if (!p1) {
            return -1;
        }
    }
    S_STREAM in(p1->value.c_str());
    T tmp;
    in >> tmp;
    if (!in.fail()) {
        value = tmp;
        return 0;
    }
    return 1;
}

int EGS_Input::getInput(const string &key, string &value) const {
    vector<string> v;
    //int err = get_input(p,key,v);
    int err = getInput(key,v);
    if (err) {
        return err;
    }
    value = v[0];
    for (unsigned int j=1; j<v.size(); j++) {
        value += " ";
        value += v[j];
    }
    return 0;
}

//int EGS_Input::getInput(const string &key, string &value) const {
//    return get_input(p,key,value);
//}

int EGS_Input::getInput(const string &key, float &value) const {
    return get_input(p,key,value);
}

int EGS_Input::getInput(const string &key, double &value) const {
    return get_input(p,key,value);
}

int EGS_Input::getInput(const string &key, int &value) const {
    return get_input(p,key,value);
}

// If the stupid MS compiler would support the >> operator on 64 bit
// integers, we would just use the template as in the other functions.
// But as it doesn't, we need a separate implementation (which is pretty
// simple-minded, but it should do for now).
int EGS_Input::getInput(const string &key, EGS_I64 &value) const {
    vector<string> aux;
    int err = getInput(key,aux);
    if (err) {
        return err;
    }
    if (aux.size() > 1) {
        return 1;
    }
    if (aux[0].size() < 10) {
        // if it is less than 10 chars long, it is guaranteed to be
        // less then 1e9 => will fit into a 32 bit integer.
        int n;
        err = getInput(key,n);
        if (err) {
            return err;
        }
        value = n;
        return 0;
    }
    int nfirst = 0;
    bool neg = false;
    if (aux[0][0] == '+') {
        nfirst = 1;
    }
    else if (aux[0][0] == '-') {
        nfirst = 1;
        neg = true;
    };
    EGS_I64 fac=1, res = 0;
    for (int j=aux[0].size()-1; j>=nfirst; j--) {
        if (!isdigit(aux[0][j])) {
            return 2;
        }
        EGS_I64 c = aux[0][j]-48;
        res += c*fac;
        fac *= 10;
    }
    if (neg) {
        res *= (-1);
    }
    value = res;
    return 0;
}


int EGS_Input::getInput(const string &key, const vector<string> &allowed,
                        int def, bool *found) const {
    string res;
    int err = getInput(key,res);
    if (!err) {
        for (unsigned int j=0; j<allowed.size(); j++) {
            if (EGS_InputPrivate::compareKeys(res,allowed[j])) {
                if (found) {
                    *found = true;
                }
                return j;
            }
        }
    }
    if (found) {
        *found = false;
    }
    return def;
}

#ifndef SKIP_DOXYGEN
int EGS_InputPrivate::setContentFromFile(const char *fname) {
    ifstream in(fname);
    if (!in) {
        return -1;
    }
    return setContent(in);
}

int EGS_InputPrivate::addContentFromFile(const char *fname) {
    // the following removes white space at the beginning of file names,
    // which may come from the file name being defined in an "include file"
    // key-value pair
    const char *s = fname;
    while (isspace(*s) && (*s)) {
        ++s;
    }
    ifstream in(s);
    if (!in) {
        return -1;
    }
    return addContent(in);
}

int EGS_InputPrivate::setContentFromString(string &input) {
    S_STREAM in(input.c_str());
    return setContent(in);
}

int EGS_InputPrivate::addContentFromString(string &input) {
    S_STREAM in(input.c_str());
    return addContent(in);
}

void EGS_InputPrivate::removeComment(const string &start, const string &end,
                                     string &input, bool newline) {
    string::size_type spos=0, epos=0;
    while ((epos = input.find(end,spos)) < input.size()) {
        spos = input.find(start,spos);
        if (spos < epos) {
            string::size_type len = epos - spos;
            if (!newline) {
                len += end.size();
            }
            input.erase(spos,len);
            if (newline) {
                spos += end.size();
            }
        }
        else {
            spos = epos+1;
        }
    }
}

int EGS_InputPrivate::setContent(istream &in) {
    for (unsigned int j=0; j<children.size(); j++) {
        delete children[j];
    }
    children.erase(children.begin(),children.end());
    return addContent(in);
}

bool EGS_InputPrivate::compareKeys(const string &s1, const string &s2) {
    string t1, t2;
    unsigned int j;
    for (j=0; j<s1.size(); j++)
        if (!isspace(s1[j])) {
            t1 += ::toupper(s1[j]);
        }
    for (j=0; j<s2.size(); j++)
        if (!isspace(s2[j])) {
            t2 += ::toupper(s2[j]);
        }
    return (t1 == t2);

}

#ifdef INPUT_DEBUG
    #include <iostream>
#endif

int EGS_InputPrivate::replace(const string &replace_what,
                              const string &replace_with) {
    string::size_type pos = 0;
    int nr = 0;
    while ((pos = key.find(replace_what,pos)) < key.size()) {
        key.replace(pos,replace_what.size(),replace_with);
        ++nr;
    }
    pos = 0;
    while ((pos = value.find(replace_what,pos)) < value.size()) {
        value.replace(pos,replace_what.size(),replace_with);
        ++nr;
    }
    for (int j=0; j<children.size(); j++) {
        nr += children[j]->replace(replace_what,replace_with);
    }
    return nr;
}
#endif

/*! \brief A base class for input loops.
 Basic functionality for input loops is provided by this class. Input loops
 can be used whenever a large number of similar input blocks are needed.

Input loops are handy to define several geometries or/and sources
in the input files which require only small variations in their definitions.
This way cumbersome repetition of identical information is avoided minimizing
the length of the input file.
Loop variables can be of type 0 (integer) or 1 (real) or 2 (strings)
specified by the first number in the loop variable input.
Now one can have in the input file the following construct:
\verbatim
:start input loop:
    loop count = N
    loop variable = 0 var1 v1min v1delta
    loop variable = 1 var2 v2min v2delta
    loop variable = 2 var3 v3_1 v3_2 ... v3_N
    some other input ...
:stop input loop:
\endverbatim
Then, everything in the input loop block except for
the definition of <b><em>loop count</em></b> and
<b><em>loop variable</em></b> will be repeated
<em>N</em> times, replacing all occurences of <em>\$(var1)</em>
with <em>v1min+v1delta*i</em>,
of <em>\$(var2)</em> with <em>v2min+v2delta*i</em> and of <em>\$(var3)</em>
with <em>v3_1, v3_2 ... v3_N</em>.

For real input variable (type 1), one can add a c style printf format string at the end of the
loop variable input line to specify the format to use when replacing the variable with
its values. By default, the format is "%lg". For example:
\verbatim
:start input loop:
    loop count = 3
    loop variable = 1 var1 0 0.1 %.3f
    $(var1)
:stop input loop:
\endverbatim
specifies that when replacing the variable in the instances of the loop, the value of
var1 should be printed with a "%.3f" format, so this loop would yield:
\verbatim
0.000
0.100
0.200
\endverbatim


For string valued lists, a check is done to make sure that the list size
is not smaller than the loop size. In principle, a list loop doesn't need
a loop count entry, but if one wants to have non-list variables as well,
then the loop count must be there.

Input loops can also be nested. For instance:
\verbatim
:start input loop:
    loop count = 2
    loop variable = 0 var1 10 5
    :start input loop:
        loop count = 5
        loop variable = 0 var2 1 1
        key = $(var2) $(var1)
    :stop input loop:
:stop input loop:
\endverbatim
will produce
\verbatim
key = 1 10
key = 2 10
key = 3 10
key = 4 10
key = 5 10
key = 1 15
key = 2 15
key = 3 15
key = 4 15
key = 5 15
\endverbatim
in the input tree after the input loops have been processed.
This can be used to do 2D and even 3D array replicas.

*/
class EGS_LOCAL EGS_InputLoopVariable {
public:
    bool is_list;//!< True if input loop type is 2, else it is set to false.
    string vname,//!< Loop variable name.
           vr;   //!< Loop variable replacement string.
    char buf[128];
    EGS_InputLoopVariable(const string &var) : vname(var), is_list(false) {
        vr = "$(";
        vr += vname;
        vr += ")";
    };
    virtual ~EGS_InputLoopVariable() {};
    const char *getVarNameReplacement() const {
        return vr.c_str();
    };
    const char *getVarReplacement() const {
        return buf;
    };
    virtual void setVarReplacement(int) = 0;
    static EGS_InputLoopVariable *getInputLoopVariable(const char *input);
};

/*! \brief A base class for integer valued input loops.
 Basic functionality for input loops is provided by this class.

*/
class EGS_LOCAL EGS_IntegerInputLoopVariable : public EGS_InputLoopVariable {
public:
    int vmin, vdelta;
    EGS_IntegerInputLoopVariable(int  Vmin, int Vdelta, const string &var) :
        EGS_InputLoopVariable(var), vmin(Vmin), vdelta(Vdelta) {};
    void setVarReplacement(int i) {
        int v = vmin + vdelta*i;
        sprintf(buf,"%d",v);
    };
};

/*! \brief A base class for real valued input loops.
 Basic functionality for input loops is provided by this class.

*/
class EGS_LOCAL EGS_FloatInputLoopVariable : public EGS_InputLoopVariable {
public:
    double vmin, vdelta;
    string format;
    EGS_FloatInputLoopVariable(double Vmin, double Vdel, const string &var, const string &Format) :
        EGS_InputLoopVariable(var), vmin(Vmin), vdelta(Vdel), format(Format) {};
    void setVarReplacement(int i) {
        double v = vmin + vdelta*i;
        sprintf(buf,format.c_str(),v);
    };
};
class EGS_LOCAL EGS_ListInputLoopVariable : public EGS_InputLoopVariable {
public:
    vector<string> list;
    EGS_ListInputLoopVariable(vector<string> List, const string &var) :
        EGS_InputLoopVariable(var), list(List) {
        is_list = true;
    };
    void setVarReplacement(int i) {
        string str = list[i];
        sprintf(buf,"%s",str.c_str());
    };
    int list_size() {
        return list.size();
    }
};
EGS_InputLoopVariable *EGS_InputLoopVariable::getInputLoopVariable(
    const char *input) {
    if (!input) {
        return 0;
    }
    S_STREAM in(input);
    string name;
    int type;
    in >> type >> name;
    if (in.fail() || !in.good()) {
        egsWarning("Failed reading type and name from %s\n",input);
        return 0;
    }
    if (type < 0 || type > 2) {
        egsFatal("Invalid loop type in input: %s\n"
                 "Only integer [0], float [1] and list [2] are valid types!\n",
                 input);
    }
    EGS_InputLoopVariable *result;
    if (type == 0) {
        int vmin, vdelta;
        in >> vmin >> vdelta;
        result = new EGS_IntegerInputLoopVariable(vmin,vdelta,name);
    }
    else if (type == 1) {
        double vmin, vdelta;
        in >> vmin >> vdelta;
        string format;
        in >> format;
        if (format.empty()) {
            format = "%lg";
            if (in.fail()) {
                in.clear();
            }
        }
        result = new EGS_FloatInputLoopVariable(vmin,vdelta,name,format);
    }
    else if (type == 2) {
        vector<string> vstr;
        string str, s_tmp;
        while (!in.eof()) {
            in >> s_tmp;
            if (!in.fail()) {
                vstr.push_back(s_tmp);
            }
        }
        result = new EGS_ListInputLoopVariable(vstr,name);
        if (in.fail() && in.eof()) { // possibly white spaces at end of line
            if (!vstr.size()) { // end-of-line reached and no list-item found
                delete result;
                result = 0;
                egsFatal("No list-items found reading loop-input: %s\n",input);
            }
            //Found white spaces at the end of loop-input list which is ok.
            return result;
        }
        if (in.bad()) {
            delete result;
            result = 0;
            egsFatal("Fatal error reading loop input list from %s\n",input);
        }
    }
    if (in.fail()) {
        egsWarning("Failed reading vmin vdelta from %s\n",input);
        delete result;
        result = 0;
    }
    return result;
}

#ifndef SKIP_DOXYGEN
void EGS_InputPrivate::processInputLoop(EGS_InputPrivate *p) {
    egsWarning("Processing input loop\n");
    EGS_InputPrivate *ic = p->takeInputItem("loop count");
    if (!ic) {
        egsWarning("processInputLoop: no 'loop count' input\n");
        return;
    }
    int nloop = -1;
    int err = get_input(ic,"loop count",nloop);
    delete ic;
    if (err || nloop < 1) {
        egsWarning("processInputLoop: got %d for loop count, expecting 1 or "
                   "more\n",nloop);
        return;
    }
    EGS_InputPrivate *iv;
    vector<EGS_InputLoopVariable *> ivars;
    while ((iv = p->takeInputItem("loop variable")) != 0) {
        EGS_InputLoopVariable *v =
            EGS_InputLoopVariable::getInputLoopVariable(iv->value.c_str());
        if (v->is_list) {
            if (((EGS_ListInputLoopVariable *)v)->list_size()<nloop) {
                egsFatal("procesInputLoop: loop size (%d) larger than list size (%d)!\n"
                         "This will cause a segmentation fault error, aborting ....\n",
                         nloop, ((EGS_ListInputLoopVariable *)v)->list_size());
            }
        }
        if (!v) egsWarning("processInputLoop: failed to create loop variable"
                               " based on the input %s\n",iv->value.c_str());
        else {
            ivars.push_back(v);
        }
        delete iv;
    }
    if (!ivars.size()) {
        egsWarning("processInputLoop: no loop variables\n");
        return;
    }
    int nvar = ivars.size();
    int j;
    /*
    for(j=0; j<p->children.size(); j++) {
        for(int iloop=0; iloop<nloop; iloop++) {
            EGS_InputPrivate *pnew = new EGS_InputPrivate(*p->children[j],true);
            for(int ivar=0; ivar<nvar; ivar++) {
                ivars[ivar]->setVarReplacement(iloop);
                pnew->replace(ivars[ivar]->getVarNameReplacement(),
                        ivars[ivar]->getVarReplacement());
            }
            children.push_back(pnew);
        }
    }
    */
    for (int iloop=0; iloop<nloop; iloop++) {
        for (j=0; j<p->children.size(); j++) {
            EGS_InputPrivate *pnew = new EGS_InputPrivate(*p->children[j],true);
            for (int ivar=0; ivar<nvar; ivar++) {
                ivars[ivar]->setVarReplacement(iloop);
                pnew->replace(ivars[ivar]->getVarNameReplacement(),
                              ivars[ivar]->getVarReplacement());
            }
            children.push_back(pnew);
        }
    }

    for (j=0; j<ivars.size(); j++) {
        delete ivars[j];
    }
}

int EGS_InputPrivate::addContent(istream &in) {
    string input;
    bool last_was_space = false;
    while (1) {
        char c;
        in.get(c);
        if (in.eof() || !in.good()) {
            break;
        }
        bool take_it = true;
        if (isspace(c)) {
            if (last_was_space && c != '\n') {
                take_it = false;
            }
            last_was_space = true;
        }
        else {
            last_was_space = false;
        }
        if (take_it) {
            input += c;
        }
    }
    removeComment("#","\n",input,true);
    removeComment("!","\n",input,true);
    removeComment("//","\n",input,true);
    removeComment("/*","*/",input,false);
    removeEmptyLines(input);
    int res = 0;
    vector<string> start_keys, stop_keys;
    int p = 0;
    int ep = input.size();
    string what;
    int ie;
    while ((p=findStart(p,ep,start_key_begin,start_key_end,input,what,ie))>=0) {
        string the_start = start_key_begin;
        string the_end = stop_key_begin;
        for (int j=0; j<what.size(); j++) {
            char c = ::toupper(what[j]);
            if (!isspace(c)) {
                the_start += c;
                the_end += c;
            }
        }
        the_start += start_key_end;
        the_end += stop_key_end;
        int p1;
        int ep = findStop(ie+1,the_start,the_end,input,p1);
        if (ep > ie+1) {
            EGS_InputPrivate *ip = new EGS_InputPrivate(what);
            string content;
            content.assign(input,ie+1,p1-ie-1);
            input.erase(p,ep-p);
            ip->setContentFromString(content);
            if (ip->isA("input loop")) {
                processInputLoop(ip);
                delete ip;
            }
            else {
                children.push_back(ip);
            }
        }
        else {
            egsWarning("No matching stop delimeter for %s\n",what.c_str());
            return -1;
        }
    }
    p = 0;
    string::size_type p1;
    while ((p1=input.find('\n',p)) < input.size()) {
        int j=p1;
        while (--j > p && isspace(input[j]));
        if (j > p) {
            if (input[j] == ',' || input[j] == '\\') {
                input[p1] = ' ';
                input[j] = ' ';
            }
        }
        p = p1+1;
    }
    p=0;
    while ((p1=input.find('\n',p)) < input.size()) {
        string::size_type p2 = input.find('=',p);
        if (p2 < p1) {
            string what;
            what.assign(input,p,p2-p);
            string value;
            value.assign(input,p2+1,p1-p2-1);
            for (int j=0; j<value.size(); j++)
                if (value[j] == ',') {
                    value[j] = ' ';
                }
            if (compareKeys(what,"includefile")) {
                int res = addContentFromFile(value.c_str());
                if (res) egsWarning("EGS_Input: failed to add content from "
                                        "include file %s\n",value.c_str());
            }
            else {
                EGS_InputPrivate *ip = new EGS_InputPrivate(what,value);
                children.push_back(ip);
            }
        }
        p = p1+1;
    }

#ifdef INPUT_DEBUG
    print(0,std::cout);
#endif

    return 0;

}

int EGS_InputPrivate::findStop(int start, const string &the_start,
                               const string &the_end, const string &input,
                               int &ie) {
    int ns=0, ne=0;
    int have_start=1, have_end=0;
    int end_started = start;
    for (int j=start; j<input.size(); j++) {
        if (!isspace(input[j])) {
            char c = ::toupper(input[j]);
            if (the_start[ns] == c) {
                ns++;
            }
            else {
                ns=0;
                if (the_start[ns] == c) {
                    ns++;
                }
            }
            if (ns == the_start.size()) {
                ns=0;
                have_start++;
            }
            if (!ne) {
                end_started = j;
            }
            if (the_end[ne] == c) {
                ne++;
            }
            else {
                ne=0;
                if (the_end[ne] == c) {
                    end_started = j;
                    ne++;
                }
            }
            if (ne == the_end.size()) {
                ne = 0;
                have_end++;
                if (have_end == have_start) {
                    ie = end_started;
                    return j;
                }
                end_started = j+1;
            }
        }
    }
    return -1;
}

int EGS_InputPrivate::findStart(int start, int stop, const string &start_key,
                                const string &end_key, const string &input,
                                string &what, int &end) {
    string::size_type pos = start;
    unsigned int ns=0;
    while (1) {
        if (pos >= stop) {
            return -1;
        }
        char c = ::toupper(input[pos++]);
        if (start_key[ns] == c) {
            ns++;
        }
        else {
            ns=0;
            if (start_key[ns] == c) {
                ns++;
            }
        }
        if (ns == start_key.size()) {
            break;
        }
    }
    string::size_type epos = input.find(end_key,pos);
    if (epos < stop) {
        what.assign(input,pos,epos-pos);
        end = epos + end_key.size();
        return pos-start_key.size();
    }
    return -2;
}

void EGS_InputPrivate::print(int indent, ostream &out) const {
    if (children.size() > 0) {
        if (key.size() > 0) {
            for (int j=0; j<indent; j++) {
                out << indent_space;
            }
            out << start_key_begin << " " << key << start_key_end << endl;
        }
        indent += 1;
        for (int i=0; i<children.size(); i++) {
            children[i]->print(indent,out);
        }
        indent -= 1;
        if (key.size() > 0) {
            for (int j=0; j<indent; j++) {
                out << indent_space;
            }
            out << stop_key_begin << " " << key << stop_key_end << endl;
        }
    }
    else {
        for (int j=0; j<indent; j++) {
            out << indent_space;
        }
        out << key << " = " << value << endl;
    }
}

void EGS_InputPrivate::removeEmptyLines(string &input) {
    int pos=0, pos1;
    while ((pos1=input.find('\n',pos)) < input.size()) {
        if (pos1 == pos) {
            input.erase(pos,1);
        }
        else {
            bool is_only_space = true;
            for (int j=pos; j<pos1; j++) {
                if (!isspace(input[j])) {
                    is_only_space = false;
                    break;
                }
            }
            if (is_only_space) {
                input.erase(pos,pos1+1-pos);
            }
            else {
                pos = pos1+1;
            }
        }
    }
}
#endif

bool EGS_Input::compare(const string &s1, const string &s2) {
    return EGS_InputPrivate::compareKeys(s1,s2);
}

void EGS_Input::print(int nind,ostream &out) {
    if (p) {
        p->print(nind,out);
    }
}

#ifdef TEST

#include <iostream>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cctype>

void egs_warning(const char *msg,...) {
    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
}

EGS_InfoFunction egsWarning = egs_warning;

int main() {

    EGS_InputPrivate p("test");
    p.setContent(cin);
    p.print(0,cout);
    return 0;
}

#endif
