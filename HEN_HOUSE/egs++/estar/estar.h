/*
###############################################################################
#
#  EGSnrc estar
#  Copyright (C) 2026 National Research Council Canada
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
#  Author:          Sehmimul Hoque, 2022
#
#  Contributors:    Martin J. Berger
#                   Johnathan S. Coursey
#                   Reid Townson
#                   Ernesto Mainegra-Hing
#
#  Based on the original ESTAR code by Martin J. Berger,
#  National Institute of Standards and Technology (NIST). Including
#  modifications by Johnathan S. Coursey.
#
###############################################################################
*/

#pragma once
#include <string>
#include "estar_formulaCalcs.h"
#include "estar_dataParser.h"

/*!
 * \file estar.h
 * \brief Core ESTAR density correction calculation interface.
 *
 * This header exposes the public interface for computing electron stopping
 * power density corrections using the ESTAR method described in:
 *
 *   Sternheimer, R.M., Berger, M.J., and Seltzer, S.M. (1984).
 *   "Density effect for the ionization loss of charged particles in
 *   various substances." Atomic Data and Nuclear Data Tables, 30, 261-271.
 *
 * The main entry points for external callers are estar_() and
 * compoundstoelements_(), which are called directly from Mortran/Fortran
 * via the EGSnrc C/Fortran interface. All other functions in this header
 * are internal implementation details exposed for unit testing purposes.
 */


/*! \brief Output a density correction file.
 *
 *  Writes the density correction factors, energy grid, mean I-value,
 *  atomic numbers and mass fractions to a file in the standard EGSnrc
 *  density correction format (113-point energy grid).
 *
 *  \param mediaDensity   Density of the medium (g/cm^3).
 *  \param densityCorr    Array of 113 density correction factors delta,
 *                        one per point on the standard energy grid.
 *  \param enGrid         Array of 113 energies (MeV) defining the standard
 *                        energy grid.
 *  \param meanIval       Pointer to the mean excitation energy I (eV) of
 *                        the medium.
 *  \param fc             formula_calc struct containing the atomic numbers,
 *                        weight fractions and element count for the medium.
 *  \param outputFilename Path to the output density correction file.
 *                        Trailing whitespace is stripped before use.
 */
void outputDensityFile(float mediaDensity, double *densityCorr, double *enGrid,
                       float *meanIval, formula_calc fc, std::string outputFilename);


/*! \brief Compute density correction factors for a medium.
 *
 *  Implements the Sternheimer (1984) method for computing the density effect
 *  correction delta to the Bethe-Bloch stopping power formula. The correction
 *  factors are computed on the standard 113-point energy grid and stored in
 *  \p densityCorr. The corresponding energies are stored in \p enGrid.
 *
 * The density-effect cutoff (the energy below which delta=0) was a feature of
 * Sternheimer's parametric approximation (Sternheimer & Peierls, Phys. Rev. B3,
 * 3681, 1971), introduced to reduce computational burden. Since EGSnrc uses
 * the "exact" Sternheimer method (Sternheimer, Berger & Seltzer, Atom. Data Nucl.
 * Data Tabl. 30, 261, 1984), in which delta emerges naturally from the oscillator-
 * strength calculation and requires no imposed cutoff, this function is no longer
 * needed.
 *
 *  Internally this function:
 *    -# Calls getDataFromFormulae() to compute the mean excitation energy
 *       and mean Z/A for the medium.
 *    -# Builds the dispersion oscillator model (Sternheimer eq. 5).
 *    -# Solves for the adjustment factor using Newton's method (eq. 8).
 *    -# Evaluates the density correction on the energy grid using a
 *       bisection solver and cubic spline interpolation (eq. 1).
 *    -# Writes the results to a density correction file via outputDensityFile().
 *
 *  \param isCompound     1 if the medium is a compound, 0 otherwise.
 *  \param NEP            Number of elements (or compounds) in \p elementArray.
 *  \param mediaDensity   Density of the medium (g/cm^3). Must be > 0.
 *  \param elementArray   Array of \p NEP element chemical symbols (e.g. "H", "O").
 *  \param massFraction   Array of \p NEP mass fractions (weight fractions),
 *                        one per element. Must sum to 1.
 *  \param numOfAtoms     Array of \p NEP atom counts per formula unit
 *                        (used for compounds).
 *  \param densityCorr    Output array of 113 density correction factors.
 *                        Must be pre-allocated by the caller.
 *  \param enGrid         Output array of 113 energies (MeV).
 *                        Must be pre-allocated by the caller.
 *  \param meanIval       On input: if >= 0, overrides the computed I-value
 *                        with this value (eV). On output: holds the I-value
 *                        actually used.
 *  \param ipotval        Pointer to the user-supplied I-value override (eV).
 *                        Set to -1 if no override is desired.
 *  \param mediaNum       Integer ID of the medium (used for diagnostic output).
 *  \param outputFilename Path to write the density correction file. Pass an
 *                        empty string to suppress file output.
 *
 *  \returns 0 on success, 9 on any error (bad input, non-convergence, etc.).
 *           Error details are printed to stdout before returning.
 */
int estarCalculation(int isCompound, int NEP, float mediaDensity,
                     std::string *elementArray, double *massFraction,
                     float *numOfAtoms, double *densityCorr, double *enGrid,
                     float *meanIval, float *ipotval, int mediaNum,
                     std::string outputFilename);


/*! \brief Result struct returned by fbspol().
 *
 *  Contains the interpolated density correction value at a given log-energy
 *  point, together with the indices of the bracketing spline interval so
 *  that the caller can use them to set bisection bounds.
 */
struct bspol {
    double density_corr; /*!< Interpolated density correction factor delta
                          *   at the queried log-energy point. */
    int lb_index;        /*!< Index of the lower bound of the bracketing
                          *   spline interval in the yql[] array. */
    int ub_index;        /*!< Index of the upper bound of the bracketing
                          *   spline interval in the yql[] array. */
};


/*! \brief Evaluate the cubic spline interpolant at a given point.
 *
 *  Given the spline coefficients produced by fscof() and a query point \p s,
 *  performs a binary search to locate the bracketing interval and evaluates
 *  the cubic polynomial within that interval using Horner's method.
 *
 *  The result also carries the bracketing interval indices, which are used
 *  by the caller to set the bounds for the bisection solver (bisec()).
 *
 *  \param s   The query point (log of y = tau*(tau+2), where tau = E/m_e c^2).
 *  \param x   Array of \p n knot positions (log-energy values yql[]).
 *  \param a   Spline coefficient array a[] from fscof().
 *  \param b   Spline coefficient array b[] from fscof().
 *  \param c   Spline coefficient array c[] from fscof().
 *  \param d   Spline coefficient array d[] from fscof().
 *  \param n   Number of knots.
 *
 *  \returns A bspol struct containing the interpolated density correction
 *           and the bracketing interval indices.
 */
bspol fbspol(double s, const std::vector<double> &x, const std::vector<double> &a,
             const std::vector<double> &b, const std::vector<double> &c,
             const std::vector<double> &d);


/*! \brief Spline coefficient struct returned by fscof().
 *
 *  Holds the four cubic spline coefficient arrays for a natural cubic spline
 *  fit to the density correction data d[] on the knot grid x[]. The spline
 *  is evaluated by fbspol() using Horner's method:
 *
 *    delta(s) = ((d[i]*(s-x[i]) + c[i])*(s-x[i]) + b[i])*(s-x[i]) + a[i]
 *
 *  where i is the index of the bracketing interval.
 */
struct scof {
    std::vector<double> a; /*!< Zeroth-order spline coefficients (function values). */
    std::vector<double> b; /*!< First-order spline coefficients. */
    std::vector<double> c; /*!< Second-order spline coefficients. */
    std::vector<double> d; /*!< Third-order spline coefficients. */

    explicit scof(int n) : a(n), b(n), c(n), d(n) {}
};


/*! \brief Compute natural cubic spline coefficients.
 *
 *  Fits a natural cubic spline (zero second derivative at both endpoints)
 *  to the data points (x[i], f[i]) for i = 0, ..., nmax-1. The resulting
 *  coefficients are stored in the returned scof struct and are intended for
 *  use with fbspol().
 *
 *  \param nmax  Number of data points. Must be >= 2.
 *  \param x     Array of \p nmax knot positions (log-energy values yql[]).
 *               Must be strictly monotone.
 *  \param f     Array of \p nmax function values (density corrections d[]).
 *
 *  \returns A scof struct containing the four coefficient arrays a, b, c, d.
 */
scof fscof(int nmax, const vector<double> &x, const vector<double> &f);


/*! \brief Objective function for the bisection solver.
 *
 *  Evaluates the function whose root locates the exact density correction
 *  for a given kinetic energy. The root condition is:
 *
 *    log(y) - log(1 / sum_i( f[i] / (eps[i] + x) )) = 0
 *
 *  where y = tau*(tau+2) and tau = E / m_e c^2 (equation 1, Sternheimer 1984).
 *
 *  \param tau   Reduced kinetic energy (E / m_e c^2) of the particle.
 *  \param f     Array of \p nmax oscillator strengths.
 *  \param eps   Array of \p nmax squared reduced oscillator energies
 *               (after adjustment factor has been applied).
 *  \param nmax  Number of dispersion oscillators.
 *  \param x     The trial value of l^2 (the variable being solved for).
 *
 *  \returns The value of the objective function at \p x. A return value of
 *           zero indicates that \p x is the exact solution.
 */
double objective_function(double tau, const vector<double> &f,
                          const vector<double> &eps, int nmax, double x);


/*! \brief Find the root of objective_function() by bisection.
 *
 *  Locates l^2 such that objective_function(tau, f, eps, nmax, l^2) = 0,
 *  which gives the exact density correction via equation 1 of Sternheimer
 *  1984. The root must be bracketed by [\p lowerbound, \p upperbound] — i.e.
 *  the function must have opposite signs at the two bounds. The bracketing
 *  interval is provided by the lb_index and ub_index fields of the bspol
 *  struct returned by fbspol().
 *
 *  If the bounds do not bracket a root, an error is printed to stdout and
 *  \p lowerbound is returned as a sentinel value.
 *
 *  \param lowerbound  Lower bound of the search interval (q[lb_index]).
 *  \param upperbound  Upper bound of the search interval (q[ub_index]).
 *  \param tolerance   Convergence tolerance on the absolute function value.
 *  \param tau         Reduced kinetic energy (E / m_e c^2).
 *  \param f           Array of \p nmax oscillator strengths.
 *  \param eps         Array of \p nmax squared reduced oscillator energies.
 *  \param nmax        Number of dispersion oscillators.
 *
 *  \returns The value of l^2 at the root to within \p tolerance, or
 *           \p lowerbound if the bounds do not bracket a root.
 */
double bisec(double lowerbound, double upperbound, double tolerance,
             double tau, const std::vector<double> &f,
             const std::vector<double> &eps, int nmax);
