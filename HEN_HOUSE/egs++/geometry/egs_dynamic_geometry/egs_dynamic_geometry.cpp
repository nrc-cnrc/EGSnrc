/*
###############################################################################
#
#  EGSnrc egs++ dynamic_geometry geometry
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
#  Author:          Alex Demelo, 2023
#
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_dynamic_geometry.cpp
 *  \brief A dynamic geometry
 *  \RT
 */

#include "egs_dynamic_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"

// ----------------------------------------------------------------------------
// Implementation of EGS_DynamicGeometry methods

// ----------------------------------------------------------------------------
// Avoid using deprecated methods from the base class
void EGS_DynamicGeometry::setMedia(EGS_Input *, int, const int *) {
    egsWarning("EGS_DynamicGeometry::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_DynamicGeometry::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_DynamicGeometry::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_DynamicGeometry::setRelativeRho(): don't use this "
               "method. Use the \n setRelativeRho() methods of the underlying "
               "geometry\n");
}

void EGS_DynamicGeometry::setBScaling(int start, int end, EGS_Float rho) {
    setBScaling(0);
}

void EGS_DynamicGeometry::setBScaling(EGS_Input *) {
    egsWarning("EGS_DynamicGeometry::setBScaling(): don't use this "
               "method. Use the \n setBScaling() methods of the underlying "
               "geometry\n");
}

// ----------------------------------------------------------------------------
// External C function to create EGS_DynamicGeometry
extern "C" {

    /*! \brief Create a dynamic geometry from input.
     *
     * This function is part of the EGSnrc dynamic geometry plugin interface.
     * \param input The input specifying the dynamic geometry.
     * \return A pointer to the created EGS_DynamicGeometry object.
     */
    EGS_DYNAMIC_GEOMETRY_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        EGS_BaseGeometry *g = 0;
        EGS_Input *ij = input->takeInputItem("geometry", false);
        if (ij) {
            g = EGS_BaseGeometry::createSingleGeometry(ij);
            delete ij;
            if (!g) {
                egsWarning("createGeometry(EGS_DynamicGeometry): got a null pointer as a geometry?\n");
                return 0;
            }
        }
        if (!g) {
            string gname;
            int err = input->getInput("my geometry", gname);
            if (err) {
                egsWarning("createGeometry(EGS_DynamicGeometry): my geometry must be defined\n either inline or using 'my geometry = some_name'\n");
                return 0;
            }
            g = EGS_BaseGeometry::getGeometry(gname);
            if (!g) {
                egsWarning("createGeometry(EGS_DynamicGeometry): no geometry named %s is defined\n", gname.c_str());
                return 0;
            }
        }
        g->ref();

        // Now getting motion information for dynamic component
        EGS_Input *dyninp = input->takeInputItem("motion");

        if (dyninp) {
            EGS_DynamicGeometry *result = new EGS_DynamicGeometry(g, dyninp);
            result->setName(input);
            result->setBoundaryTolerance(input);
            result->setLabels(input);
            return result;
        }
        else {
            egsWarning("EGS_DynamicGeometry: no control points input.\n");
            return 0;
        }
    }

    /*! \brief Get label regions for dynamic geometry.
     *
     * This function is part of the EGSnrc dynamic geometry plugin interface.
     * \param str Label string.
     * \param regs Vector to store label regions.
     */
    void EGS_DynamicGeometry::getLabelRegions(const string &str, vector<int> &regs) {
        // label defined in the geometry being transformed
        g->getLabelRegions(str, regs);

        // label defined in self (transformation input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);
    }

    /*! \brief Build dynamic geometry from input.
     *
     * This function is part of the EGSnrc dynamic geometry plugin interface.
     * \param g Pointer to the base geometry.
     * \param dyninp Input specifying the dynamic geometry.
     */
    void EGS_DynamicGeometry::buildDynamicGeometry(EGS_BaseGeometry *g, EGS_Input *dyninp) {
        stringstream itos;
        ncpts = 0;
        vector<EGS_Float> point;
        EGS_ControlPoint cpt;
        int err;
        int icpts = 1;
        itos << icpts;
        string inputTag = "control point";
        string inputTag_backCompat = "control point " + itos.str();
        EGS_Input *currentInput;
        int rotsize = 0;

        // Control points read one by one from motion block in dynamic geometry definition, and saved as a vector to points
        while (!(err = dyninp->getInput(sstring, point))) {

            // Checking the size to make sure it is a valid control point input
            if (point.size() != 6 && point.size() != 7) {
                egsWarning("EGS_DynamicGeometry: Control point %i must specify either 6 or 7 values.\n", icpts);
            }
            else {
                if (ncpts == 0) {
                    // Variable to make sure all control point definitions have consistent formats
                    rotsize = point.size();
                }

                // Make sure each time index is larger than the last
                if (ncpts > 0 && point[0] < cpts[ncpts - 1].time) {
                    egsWarning("EGS_DynamicGeometry: Time index of control point %i < time index of control point %i\n", icpts, ncpts);
                }

                // Checks that time index is valid (larger than zero)
                else if (point[0] < 0.0) {
                    egsWarning("EGS_DynamicGeometry: Time index of control point %i < 0.0\n", icpts);
                }

                // Checks that control point formats follow the first
                else if (ncpts > 0 && point.size() != rotsize) {
                    egsWarning("EGS_DynamicGeometry: Rotation definition inconsistent \n");
                }
                else {
                    ncpts++;
                    if (ncpts == 1 && point[0] > 0.0) {
                        egsWarning("EGS_DynamicGeometry: Time index of control point 1 > 0.0.  This will generate many warning messages.\n");
                    }
                    vector<EGS_Float> T_vect; // Vector storing translation information
                    vector<EGS_Float> R_vect; // Vector storing rotation information

                    // Add translation coordinates to the translation vector
                    T_vect.push_back(point[1]);
                    T_vect.push_back(point[2]);
                    T_vect.push_back(point[3]);

                    // Add rotation coordinates to rotation vector (cases differentiate cpt formats, 6 is 2 rotation parameters, 7 is 3)
                    // In each case vector order is determined by format of EGS_rotationMatrix constructor (EGS_Float arguments)
                    if (point.size() == 6) {
                        R_vect.push_back(point[6]); // Rotation about z
                        R_vect.push_back(point[4]); // Rotation about x
                    } // 2 number case
                    if (point.size() == 7) {
                        R_vect.push_back(point[4]);
                        R_vect.push_back(point[5]); // Rotation about y
                        R_vect.push_back(point[6]); // Rotation about z
                    } // 3 number case

                    // Add it to the vector of control points
                    cpt.time = point[0];
                    cpt.trnsl = T_vect;
                    cpt.rot = R_vect;
                    cpts.push_back(cpt);
                    icpts++;
                    itos.str("");
                    itos << icpts;

                    // Define next control point i string for getInput in while condition
                    inputTag_backCompat = "control point " + itos.str();
                }
            }
        }
        if (ncpts <= 1) {
            egsFatal("EGS_DynamicGeometry: Not enough or missing control points.\n");
        }
        if (cpts[ncpts - 1].time == 0.0) {
            egsFatal("EGS_DynamicGeometry: Time index of last control point = 0.  Something's wrong.\n");
        }
        else {

            // Normalize time index to max. value
            for (int i = 0; i <= ncpts - 1; i++) {
                cpts[i].time /= cpts[ncpts - 1].time;
            }
        }

        // Sets position to initial time in egs_view upon opening
        updatePosition(0);
    }

    /*! \brief Get interpolated coordinates based on the time index
     *
     * This function is part of the EGSnrc dynamic geometry plugin interface.
     * \param rand Random number for sampling.
     * \param gipt Control point struct used to track the current state of the geometry.
     * \return 0 if successful, 1 otherwise.
     */
    int EGS_DynamicGeometry::getCoordGeom(EGS_Float rand, EGS_ControlPoint &gipt) {
        int iindex = 0;
        int i;

        // The following loop determines which 2 control points the current time index falls between
        for (i = 0; i < ncpts; i++) {
            if (rand < cpts[i].time - epsilon) {
                iindex = i;
                break;
            }
        }

        if (i == ncpts) {
            egsWarning("EGS_DynamicGeometry: could not locate control point.\n");
            return 1;
        }

        // Below 3 vectors are defined, a vector containing the lower bound translation coordinates, a vector containing the upper bound translation coordinates, and a vector for the sampled translation coordinates
        vector<EGS_Float> translation_LB = cpts[iindex - 1].trnsl;
        vector<EGS_Float> translation_UB = cpts[iindex].trnsl;
        vector<EGS_Float> translation_samp;

        // The following is a factor (between 0 and 1) used for sampling. Essentially, it represents the fractional position within the interval
        EGS_Float factor = (rand - cpts[iindex - 1].time) / (cpts[iindex].time - cpts[iindex - 1].time);

        // Translations are given as the lower bound plus the length of the interval multiplied by the fractional position factor. So its essentially lower bound + n% of the interval length
        translation_samp.push_back(translation_LB[0] + (translation_UB[0] - translation_LB[0]) * factor);
        translation_samp.push_back(translation_LB[1] + (translation_UB[1] - translation_LB[1]) * factor);
        translation_samp.push_back(translation_LB[2] + (translation_UB[2] - translation_LB[2]) * factor);

        // Update the translation coordinates in the current state control point object
        gipt.trnsl = translation_samp;

        // Again 3 vectors defined, lower bound, upper bound, and sampled, this time for rotation coordinates
        vector<EGS_Float> rotation_LB = cpts[iindex - 1].rot;
        vector<EGS_Float> rotation_UB = cpts[iindex].rot;
        vector<EGS_Float> rotation_samp;

        // Now set rotations. Current coordinates computed as before (lowerbound+n% of interval length), but now we also convert degrees to radians
        // These must be done case by case (since order of arguments matters)
        if (cpts[iindex].rot.size() == 2) {
            rotation_samp.push_back((rotation_LB[0] + (rotation_UB[0] - rotation_LB[0]) * factor) * (M_PI / 180));
            rotation_samp.push_back((rotation_LB[1] + (rotation_UB[1] - rotation_LB[1]) * factor) * (M_PI / 180));
        }
        else if (cpts[iindex].rot.size() == 3) {
            rotation_samp.push_back((rotation_LB[0] + (rotation_UB[0] - rotation_LB[0]) * factor) * (M_PI / 180));
            rotation_samp.push_back((rotation_LB[1] + (rotation_UB[1] - rotation_LB[1]) * factor) * (M_PI / 180));
            rotation_samp.push_back((rotation_LB[2] + (rotation_UB[2] - rotation_LB[2]) * factor) * (M_PI / 180));
        }
        else {
            egsWarning("EGS_DynamicGeometry: Invalid number of rotation parameters\n");
        }

        // Update the rotation coordinates in the current state control point object
        gipt.rot = rotation_samp;

        return 0;
    }

    /*! \brief Compute intersections for dynamic geometry.
     *
     * This function is part of the EGSnrc dynamic geometry plugin interface.
     * \param ireg Current region index.
     * \param n Number of particles.
     * \param x Position vector.
     * \param u Direction vector.
     * \param isections Pointer to EGS_GeometryIntersections object.
     * \return Number of intersections.
     */
    int EGS_DynamicGeometry::computeIntersections(int ireg, int n, const EGS_Vector &x,
            const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        EGS_Vector xt(x), ut(u);
        T.inverseTransform(xt);
        T.rotateInverse(ut);
        return g->computeIntersections(ireg, n, xt, ut, isections);
    }

    /**
     * @brief Check if the given region index corresponds to a real region in the dynamic geometry.
     *
     * @param ireg The region index.
     * @return True if the region is real, false otherwise.
     */
    bool EGS_DynamicGeometry::isRealRegion(int ireg) const {
        return g->isRealRegion(ireg);
    }

    /**
     * @brief Check if a point is inside the dynamic geometry.
     *
     * @param x The point to check.
     * @return True if the point is inside, false otherwise.
     */
    bool EGS_DynamicGeometry::isInside(const EGS_Vector &x) {
        EGS_Vector xt(x);
        T.inverseTransform(xt);
        return g->isInside(xt);
    }

    /**
     * @brief Determine the region of a point
     *
     * @param x The point to check.
     * @return The region number (see EGS_BaseGeometry::isWhere() for details).
     */
    int EGS_DynamicGeometry::isWhere(const EGS_Vector &x) {
        EGS_Vector xt(x);
        T.inverseTransform(xt);
        return g->isWhere(xt);
    }

    /**
     * @brief Alias for EGS_DynamicGeometry::isWhere().
     *
     * @param x The point to check.
     * @return The region number (see EGS_BaseGeometry::isWhere() for details).
     */
    int EGS_DynamicGeometry::inside(const EGS_Vector &x) {
        return isWhere(x);
    }

    /**
     * @brief Get the medium index for a given region in the dynamic geometry.
     *
     * @param ireg The region index.
     * @return The medium index.
     */
    int EGS_DynamicGeometry::medium(int ireg) const {
        return g->medium(ireg);
    }

    /**
     * @brief Calculate the distance to the outside of the dynamic geometry from a point along a direction.
     *
     * @param ireg The current region index.
     * @param x The starting point.
     * @param u The direction vector.
     * @return The distance to the outside or 0 if the current region is invalid.
     */
    EGS_Float EGS_DynamicGeometry::howfarToOutside(int ireg, const EGS_Vector &x,
            const EGS_Vector &u) {
        return ireg >= 0 ? g->howfarToOutside(ireg, x * T, u * T.getRotation()) : 0;
    }

    /**
     * @brief Calculate the distance to the next boundary in the dynamic geometry.
     *
     * @param ireg The current region index.
     * @param x The starting point.
     * @param u The direction vector.
     * @param t The distance to the next boundary.
     * @param newmed The medium index after boundary crossing.
     * @param normal The normal vector at the boundary.
     * @return The new region index after boundary crossing.
     */
    int EGS_DynamicGeometry::howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
                                    EGS_Float &t, int *newmed, EGS_Vector *normal) {
        EGS_Vector xt(x), ut(u);
        T.inverseTransform(xt);
        T.rotateInverse(ut);
        int inew = g->howfar(ireg, xt, ut, t, newmed, normal);
        if (inew != ireg && normal) {
            *normal = T.getRotation() * (*normal);
        }
        return inew;
    }

    /**
     * @brief Calculate the distance to the nearest boundary of the dynamic geometry from a point.
     *
     * @param ireg The current region index.
     * @param x The point.
     * @return The distance to the nearest boundary.
     */
    EGS_Float EGS_DynamicGeometry::hownear(int ireg, const EGS_Vector &x) {
        EGS_Vector xt(x);
        T.inverseTransform(xt);
        return g->hownear(ireg, xt);
    }

    /**
     * @brief Get the maximum step size allowed in the dynamic geometry.
     *
     * @return The maximum step size.
     */
    int EGS_DynamicGeometry::getMaxStep() const {
        return g->getMaxStep();
    }

    /**
     * @brief Check if the specified region has a boolean property.
     *
     * @param ireg The region index.
     * @param prop The boolean property to check.
     * @return True if the region has the specified boolean property, false otherwise.
     */
    bool EGS_DynamicGeometry::hasBooleanProperty(int ireg, EGS_BPType prop) const {
        return g->hasBooleanProperty(ireg, prop);
    }

    /**
     * @brief Set a boolean property for the dynamic geometry.
     *
     * @param prop The boolean property to set.
     */
    void EGS_DynamicGeometry::setBooleanProperty(EGS_BPType prop) {
        g->setBooleanProperty(prop);
    }

    /**
     * @brief Add a boolean property to the dynamic geometry.
     *
     * @param bit The bit index of the boolean property to add.
     */
    void EGS_DynamicGeometry::addBooleanProperty(int bit) {
        g->addBooleanProperty(bit);
    }

    /**
     * @brief Set a boolean property for a range of regions in the dynamic geometry.
     *
     * @param prop The boolean property to set.
     * @param start The starting region index.
     * @param end The ending region index.
     * @param step The step size between regions.
     */
    void EGS_DynamicGeometry::setBooleanProperty(EGS_BPType prop, int start, int end, int step) {
        g->setBooleanProperty(prop, start, end, step);
    }

    /**
     * @brief Add a boolean property to a range of regions in the dynamic geometry.
     *
     * @param bit The bit index of the boolean property to add.
     * @param start The starting region index.
     * @param end The ending region index.
     * @param step The step size between regions.
     */
    void EGS_DynamicGeometry::addBooleanProperty(int bit, int start, int end, int step) {
        g->addBooleanProperty(bit, start, end, step);
    }

    /**
     * @brief Get the type of the dynamic geometry.
     *
     * @return The type as a string reference.
     */
    const string &EGS_DynamicGeometry::getType() const {
        return type;
    }

    /**
     * @brief Get the relative density scaling factor for a given region in the dynamic geometry.
     *
     * @param ireg The region index.
     * @return The relative density scaling factor.
     */
    EGS_Float EGS_DynamicGeometry::getRelativeRho(int ireg) const {
        return g->getRelativeRho(ireg);
    }

    /**
     * @brief Get the boundary scaling factor for a given region in the dynamic geometry.
     *
     * @param ireg The region index.
     * @return The boundary scaling factor.
     */
    EGS_Float EGS_DynamicGeometry::getBScaling(int ireg) const {
        return g->getBScaling(ireg);
    }

    /**
     * @brief Determine the next state of the dynamic geometry.
     *
     * This method obtains a time index, either from the simulation source or by sampling itself if the source yields nothing.
     * It then obtains the corresponding position and orientation coordinates through the `getCoordGeom` method and creates
     * and sets the dynamic geometry's transform.
     *
     * @param rndm Random number generator.
     */
    void EGS_DynamicGeometry::getNextGeom(EGS_RandomGenerator *rndm) {
        int errg = 1;
        EGS_ControlPoint gipt;

        // Get the source from active application to extract time
        EGS_Application *app = EGS_Application::activeApplication();
        while (errg) {
            // Get time from source if it exists (otherwise gives -1)
            ptime = app->getTimeIndex();
            if (ptime < 0) {
                // If no time is given by the source, randomly sample from 0 to 1
                ptime = rndm->getUniform();
                // Set randomly sampled time index for all objects in the simulation (through base source)
                app->setTimeIndex(ptime);
            }
            // Run the `getCoordGeom` method that will sample the control points given to find the transformation that will be applied for the current history
            errg = getCoordGeom(ptime, gipt);
        }

        // Create and set the current geometry transformation using the sampled coordinates from `getCoordGeom`. This is where the overloaded `EGS_AffineTransform` is used
        EGS_AffineTransform *tDG = EGS_AffineTransform::getTransformation(gipt.trnsl, gipt.rot);
        setTransformation(*tDG);

        // Call `getNextGeom` on base geometry in case there are lower-level dynamic geometries
        g->getNextGeom(rndm);
    }

    /**
     * @brief Update the position of the dynamic geometry.
     *
     * This method is used to update the geometry state as needed in egs_view. It takes the desired time index,
     * computes the corresponding coordinates, and sets the geometry transform to update the egs_view display.
     *
     * @param time Desired time index for the update.
     */
    void EGS_DynamicGeometry::updatePosition(EGS_Float time) {
        int errg = 1;
        EGS_ControlPoint gipt;

        // Run the `getCoordGeom` method that will use the control points given to find the transformation that will be applied for the current time index
        errg = getCoordGeom(time, gipt);

        // Create and set the geometry transform with the updated coordinates
        EGS_AffineTransform *tDG = EGS_AffineTransform::getTransformation(gipt.trnsl, gipt.rot);
        setTransformation(*tDG);

        // Call `updatePosition` on the base to allow lower-level geometries to update as needed
        g->updatePosition(time);
    }

    /**
     * @brief Check if the simulation geometry contains dynamic geometry.
     *
     * This method is used to determine whether the simulation geometry contains dynamic geometry.
     * It sets the `hasdynamic` flag to true if the simulation contains dynamic geometry.
     *
     * @param hasdynamic Boolean flag to indicate if dynamic geometry is present.
     */
    void EGS_DynamicGeometry::containsDynamic(bool &hasdynamic) {
        // If the dynamic geometry implementation of `containsDynamic` is called, the simulation does indeed contain a dynamic geometry, so the boolean flag is set to true
        hasdynamic = true;
    }

}


