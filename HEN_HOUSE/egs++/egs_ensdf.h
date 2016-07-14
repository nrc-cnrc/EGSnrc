/*
###############################################################################
#
#  EGSnrc egs++ ensdf headers
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
#  Author:          Reid Townson, 2016
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_ensdf.h
 *  \brief The ensdf library header file
 *  \RT
 *
 */

#ifndef EGS_ENSDF_
#define EGS_ENSDF_

#include "egs_libconfig.h"
#include "egs_functions.h"
#include "egs_math.h"
#include "egs_alias_table.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>

using namespace std;

template <class T> class Branch {
public:

    Branch() {}

    ~Branch() {
        for (typename vector<T *>::iterator it  = branchLeaves.begin();
                it!=branchLeaves.end(); it++) {
            (*it)->removeBranch();
        }
        branchLeaves.clear();
    }

    void addLeaf(T *leaf) {
        branchLeaves.push_back(leaf);
    }

    void removeLeaf(T *leaf) {
        branchLeaves.erase(std::remove(branchLeaves.begin(),
                                       branchLeaves.end(),
                                       leaf), branchLeaves.end());
    }

    vector<T *> getLeaves() const {
        return branchLeaves;
    }

    // A new == operator for this class
    bool operator==(const Branch<T> &rhs) const {
        for (typename vector<T *>::const_iterator it = branchLeaves.begin();
                it!=branchLeaves.end(); it++) {

            bool foundLeaf = false;
            for (typename vector<T *>::const_iterator irhs =
                        rhs.branchLeaves.begin();
                    irhs!=rhs.branchLeaves.end(); irhs++) {

                if (*irhs != 0 && *it != 0) {
                    if (*irhs == *it) {
                        foundLeaf = true;
                    }
                }
            }

            if (!foundLeaf) {
                return false;
            }
        }
        return true;
    }

protected:
    vector<T *> branchLeaves;
};

template <class T> class Leaf {
public:

    Leaf(T *existingTree) {
        tree = existingTree;
        if (tree) {
            tree->addLeaf(this);
        }
    }

    ~Leaf() {
        if (tree) {
            tree->removeLeaf(this);
        }
        tree = 0;
    }

    virtual T *getBranch() const {
        return tree;
    }

    void removeBranch() {
        tree = 0;
    }

    // A new == operator for this class
    bool operator== (const T &rhs) const {
        if (tree==0 && rhs.tree==0) {
            return true;
        }
        else if ((tree==0) && rhs.tree!=0) {
            return false;
        }
        else if ((tree!=0) && rhs.tree==0) {
            return false;
        }
        else if (tree!=0 && rhs.tree!=0) {
            return *tree == *(rhs.tree);
        }
    }

private:
    T *tree;
};

// The Record class
class Record {
public:
    Record(vector<string> ensdf);
    virtual ~Record();
    vector<string> getRecords() const;

protected:
    double recordToDouble(int startPos, int endPos);
    double parseHalfLife(int startPos, int endPos);
    unsigned short int setZ(string id);
    map<string, unsigned short int> getElementMap();
    unsigned short int findZ(string element);

    // All the lines corresponding to this record type
    vector<string> lines;
};

// Comment Record
class CommentRecord : public Record {
public:
    CommentRecord(vector<string> ensdf);
    string getComment();

private:
    string comment;
    void processEnsdf();
};

// Parent Record
class ParentRecord : public Record, public Branch<Leaf<ParentRecord> > {
public:
    ParentRecord(vector<string> ensdf);
    double getHalfLife() const;

protected:
    double halfLife;

private:
    void processEnsdf();
};

class ParentRecordLeaf : public Leaf<ParentRecord> {
public:
    ParentRecordLeaf(ParentRecord *myRecord);
    virtual const ParentRecord *getParentRecord() const;
};

// Normalization Record
class NormalizationRecord : public Record, public
    Branch<Leaf<NormalizationRecord> >, public ParentRecordLeaf {
public:
    NormalizationRecord(vector<string> ensdf, ParentRecord *parent);
    double getRelativeMultiplier() const;
    double getTransitionMultiplier() const;
    double getBranchMultiplier() const;
    double getBetaMultiplier() const;

protected:
    double normalizeRelative;
    double normalizeTransition;
    double normalizeBeta;
    double normalizeBranch;

private:
    void processEnsdf();
};

class NormalizationRecordLeaf : public Leaf<NormalizationRecord> {
public:
    NormalizationRecordLeaf(NormalizationRecord *myRecord);
    virtual const NormalizationRecord *getNormalizationRecord() const;
};

// Level Record
class LevelRecord : public Record, public Branch<Leaf<LevelRecord> > {
public:
    LevelRecord(vector<string> ensdf);
    double getEnergy() const;
    double getHalfLife() const;

protected:
    double energy;
    double halfLife;

private:
    void processEnsdf();
};

class LevelRecordLeaf : public Leaf<LevelRecord> {
public:
    LevelRecordLeaf(LevelRecord *myRecord);
    virtual const LevelRecord *getLevelRecord() const;
};

// Generic beta record
class BetaRecordLeaf : public Record, public ParentRecordLeaf, public
    NormalizationRecordLeaf, public LevelRecordLeaf {
public:
    BetaRecordLeaf(vector<string> ensdf, ParentRecord *myParent,
                   NormalizationRecord *myNormalization, LevelRecord *myLevel);

    virtual double getFinalEnergy() const = 0;
    virtual double getBetaIntensity() const = 0;
    virtual void setBetaIntensity(double newIntensity)  = 0;
    int getCharge() const;
    void incrNumSampled();
    EGS_I64 getNumSampled() const;
    unsigned short int getZ() const;
    unsigned short int getAtomicWeight() const;
    unsigned short int getForbidden() const;
    void setSpectrum(EGS_AliasTable *bspec);
    EGS_AliasTable* getSpectrum() const;

protected:
    EGS_I64 numSampled;
    double finalEnergy;
    double betaIntensity;
    int q;
    unsigned short int Z;
    unsigned short int A;
    unsigned short int forbidden;
    EGS_AliasTable *spectrum;
};

// Beta- record
class BetaMinusRecord : public BetaRecordLeaf {
public:
    BetaMinusRecord(vector<string> ensdf, ParentRecord *myParent,
                    NormalizationRecord *myNormalization, LevelRecord *myLevel);

    double getFinalEnergy() const;
    double getBetaIntensity() const;
    void setBetaIntensity(double newIntensity);

private:
    void processEnsdf();
};

// Beta+ Record (and Electron Capture)
class BetaPlusRecord : public BetaRecordLeaf {
public:
    BetaPlusRecord(vector<string> ensdf, ParentRecord *myParent,
                   NormalizationRecord *myNormalization, LevelRecord *myLevel);

    double getFinalEnergy() const;
    double getBetaIntensity() const;
    double getECIntensity() const;
    void setBetaIntensity(double newIntensity);
    void setECIntensity(double newIntensity);

protected:
    double ecIntensity;

private:
    void processEnsdf();
};

// Gamma record
class GammaRecord : public Record, public NormalizationRecordLeaf, public
    LevelRecordLeaf {
public:
    GammaRecord(vector<string> ensdf, NormalizationRecord *myNormalization,
                LevelRecord *myLevel);

    double getDecayEnergy() const;
    double getTransitionIntensity() const;
    void setTransitionIntensity(double newIntensity);
    int getCharge() const;
    LevelRecord *getFinalLevel() const;
    void setFinalLevel(LevelRecord *newLevel);
    double getHalfLife() const;
    void incrNumSampled();
    EGS_I64 getNumSampled() const;

protected:
    EGS_I64 numSampled;
    double decayEnergy;
    double transitionIntensity;
    double halfLife;
    int q;
    LevelRecord *finalLevel;

private:
    void processEnsdf();
};

// Alpha record
class AlphaRecord : public Record, public ParentRecordLeaf, public
    NormalizationRecordLeaf, public LevelRecordLeaf {
public:
    AlphaRecord(vector<string> ensdf, ParentRecord *myParent,
                NormalizationRecord *myNormalization, LevelRecord *myLevel);

    double getFinalEnergy() const;
    double getAlphaIntensity() const;
    int getCharge() const;
    void setAlphaIntensity(double newIntensity);
    void incrNumSampled();
    EGS_I64 getNumSampled() const;

protected:
    EGS_I64 numSampled;
    double finalEnergy;
    double alphaIntensity;
    int q;

private:
    void processEnsdf();
};


/*! \brief The ensdf class

  \ingroup egspp_main

  ... add description here

*/
class EGS_EXPORT EGS_Ensdf {

public:

    /*! \brief Construct an ensdf object
     *
     */
    EGS_Ensdf(const string isotope, const string ensdf_filename="");

    /*! \brief Destructor. Deallocates all allocated memory */
    ~EGS_Ensdf();

    vector<Record * > getRecords() const;
    vector<BetaRecordLeaf *> getBetaRecords() const;
    vector<ParentRecord * > getParentRecords() const;
    vector<LevelRecord * > getLevelRecords() const;
    vector<AlphaRecord * > getAlphaRecords() const;
    vector<GammaRecord * > getGammaRecords() const;
    vector<double > getXRayIntensities() const;
    vector<double > getXRayEnergies() const;
    vector<double > getAugerIntensities() const;
    vector<double > getAugerEnergies() const;
    
    string radionuclide;

    void normalizeIntensities();

protected:

    bool createIsotope(vector<string> ensdf);
    map<string, unsigned short int> getElementMap();
    unsigned short int findAtomicWeight(string element);
    void parseEnsdf(vector<string> ensdf);
    void buildRecords();

    void getEmissionsFromComments();

    ifstream ensdf_file;
    unsigned short int A;

    vector<Record * > myRecords;
    vector<CommentRecord * > myCommentRecords;
    vector<ParentRecord * > myParentRecords;
    vector<NormalizationRecord * > myNormalizationRecords;
    vector<LevelRecord * > myLevelRecords;
    vector<BetaRecordLeaf *> myBetaRecords;
    vector<BetaMinusRecord * > myBetaMinusRecords;
    vector<BetaPlusRecord * > myBetaPlusRecords;
    vector<AlphaRecord * > myAlphaRecords;
    vector<GammaRecord * > myGammaRecords;

private:

    vector<vector<string> > recordStack;
    vector<string> commentLines;
    vector<double>  xrayEnergies,
           xrayIntensities,
           augerEnergies,
           augerIntensities;
};




#endif
