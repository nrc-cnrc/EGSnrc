/*
###############################################################################
#
#  EGSnrc egs++ geometry tester headers
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
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_geometry_tester.h
 *  \brief EGS_GeometryTester class header file
 *  \IK
 */

#ifndef EGS_GEOMETRY_TESTER_
#define EGS_GEOMETRY_TESTER_

#include "egs_libconfig.h"

class EGS_Input;
class EGS_Vector;
class EGS_PrivateTester;
class EGS_BaseGeometry;

/*!  \brief A class for testing geometries.

  \ingroup egspp_main

  This class can perform various tests on a geometry object,
  which is useful in the development process of new
  geometry classes. The various tests available are described
  in the documentation of the testing functions
  testInside(), testInsideTime(), testHowfar(), testHowfarTime(),
  testHownear(), and testHownearTime().
  As the geometry to be tested will be typically defined  via an input
  file, we use the same input file to also specify the tests to be run
  => EGS_GeometryTester::EGS_GeometryTester(EGS_Input *)
  is the only constructor defined
  so far. This class can be used by external developers to test their
  own geometries => we hide the implementation in the class EGS_PrivateTester
  to demonstrate good coding practices. The input needed to perform
  a set of tests on a geometry is discussed in the
  getGeometryTester() documentation. The file test_geometry.cpp
  has a simple main program for performing a geometry test.
  and is compiled and linked against the main \c egspp library when
  building the library to create a geometry testing utility
  called \c gtest (\c gtest.exe on Windows). One uses it by executing
  \verbatim
  gtest input_file
  \endverbatim
  where \c input_file is the name of a file (including absoulute path,
  if not in the local directory) specifying a geometry
  and the tests to be performed on the geometry.
*/
class EGS_EXPORT EGS_GeometryTester {

public:

    //! \brief Construct a geometry tester from an input tree.
    EGS_GeometryTester(EGS_Input *);

    //! \brief Destructor
    virtual ~EGS_GeometryTester();

    /*! \brief Performs an inside test.

      For a given number of trials (specified in the input file), generates
      a random point within a bounding shape (specified in the input file)
      and tests if the point is inside the geometry \a g using the
      \link EGS_BaseGeometry::inside() inside() \endlink
      method. Prints the point to a file using the printPosition() function,
      if the point is inside the geometry \a g.
      If one plots the resulting data file with some 3D capable plotting
      program, one can verify by visual inspection that the
      \link EGS_BaseGeometry::inside() inside() \endlink method
      of the geometry does
      what it is supposed to do. An inside test is requested by including
      \verbatim
      :start inside test:
          :start bounding shape:
              type = box
              box size = 16
          :stop bounding shape:
          ntest = 10000
          file name = box_test.inside
      :stop inside test:
      \endverbatim
      into the geometry tester definition in an input file
      (see getGeometryTester()). Any shape available can be used
      to create the points, not just a box as in the above example
      (see \ref Shapes).
     */
    void testInside(EGS_BaseGeometry *g);

    /*! \brief Performs an inside time test

      Same as the above function, but now inside points are not printed to a
      file. Instead, the number of inside points is counted and the CPU time
      necessary to perform the given number of inside calls is measured.
      This test method may be useful to test the speed of alternative
      inside() implementations. An inside time test is specified in the
      same way in the input file as the inside test, except that now
      the \c file \c name input is not needed (and ignored, if present)
      as no output is generated.
     */
    void testInsideTime(EGS_BaseGeometry *);

    /*! \brief Performs a hownear test.

      For a given number of points x within a bounding shape calls
      the hownear(x) method of the geometry \a g. Picks ntry random points
      at a distance hownear() from x. For each of these new ntry points
      checks if they are in the same region using inside().
      If the hownear() method works properly, all such points should be
      inside the same region => prints points where this condition
      is not satisfied to a file so that the developer can check if
      and where the hownear() method is failing. A hownear test is
      specified in the same way as the \link testInside() inside \endlink
      test in the input file, except that the input is between
      <code>:start hownear test: :stop nownear test:</code>
     */
    void testHownear(int ntry, EGS_BaseGeometry *g);

    /*! \brief Performs a hownear time test

      Measures the time to do a certain number of hownear() calls
      in a way similar to the previous function but without generating
      output of failing points. A hownear time test is
      specified in the same way as the \link testInside() inside \endlink
      test in the input file, except that the input is between
      <code>:start hownear time test: :stop nownear time test:</code>
      and the <code>file name</code> key is not needed.
     */
    void testHownearTime(EGS_BaseGeometry *);

    /*! \brief Performs a howfar test

      For a given number of trials (specified in the input file), generates
      a random point x within a bounding shape (specified in the input file)
      and a random direction u. Determines all intersection points of the line
      x + u*t with the various regions of the geometry and prints these
      intersection points to a file using printPosition().
      Visual inspection of the plot generated from the data should be
      useful to determine if the howfar() geometry method is doing what
      it is supposed to do. A howfar test is
      specified in the same way as the \link testInside() inside \endlink
      test in the input file, except that the input is between
      <code>:start howfar test: :stop nowfar test:</code>
     */
    void testHowfar(EGS_BaseGeometry *);

    /*! \brief Performs a howfar time test

      Same as the above function but now intersection points are not printed
      to a file. Instead, the CPU time needed to perform the given number of
      howfar() calls is measured. This may be useful to test the speed of
      alternative howfar() implementations. A howfar time test is
      specified in the same way as the \link testInside() inside \endlink
      test in the input file, except that the input is between
      <code>:start howfar time test: :stop nowfar time test:</code> and
      the <code>file name</code> key is not needed.
     */
    void testHowfarTime(EGS_BaseGeometry *);

    /*! \brief Outputs the position \a x to a file

      This function is called from the various testing methods to print
      the position \a x to a file. Its default implementation is to just
      print the 3 coordinates of \a x. It is declared virtual so that
      one can overwrite it with alternative implementations, e.g.
      print x.z and sqrt(x.x*x.x + x.y*x.y) when testing cylindrical
      geometries, etc.
     */
    virtual void printPosition(const EGS_Vector &x);

    /*! \brief Creates a geometry tester object from the input \a i.

      This static function can be used to obtain a pointer to a
      EGS_GeometryTester object constructed from the specifications in
      the input pointed to by i.The input required to create a geometry
      tester is
      \verbatim
      :start geometry tester:
          output type = normal or spherical or cylindrical or transformed
          :start some test:
              definition of a test
          :stop some test:
          :start some_other test:
              definition of a test
          :stop some_other test:
          ...
      :stop geometry tester:
      \endverbatim
      In the above \c some or \c some_other stands for one of the
      possible tests
      (\link testInside() inside\endlink,
      \link testInsideTime() inside time\endlink,
      \link testHowfar() howfar\endlink,
      \link testHowfarTime() howfar time\endlink,
      \link testHownear() hownear\endlink,
      \link testHownearTime() hownear time\endlink).
      The \c output \c type key is optional and results in a "normal"
      output, if missing. The type of output determines how the positions
      of the points in the various tests are written to the file:
      - normal: write a triplet with x-, y- and z-position
      - spherical: In this case an optional \c midpoint key can be
        used to specify the midpoint of the sphere. The output produced
        is \f$x'_z, \sqrt{x_x^{\prime 2} + x_y^{\prime 2}}\f$, where
        \f$\vec{x}' = \vec{x} - \vec{x}_0\f$ is the position in the frame
        with origin coinciding with the sphere midpoint. This type of
        output us useful, for instance, to more easily check that
        the \c howfar intersections are on a spherical surface.
      - cylindrical: In this case one must input an \c axis key
        specifying the cylinder axis. The output produced will
        be \f$x'_z, \sqrt{x_x^{\prime 2} + x_y^{\prime 2}}\f$, where
        \f$\vec{x}'\f$ is the particle position in a frame where the
        z-axis coincides with the cylinder axis. This is useful,
        for instance, to more easily check that the \c howfar intersections
        are on a cylindrical or conical surface
      - transformed: in this case one must specify an affine transformation
        in the inpout (see EGS_AffineTransform::getTransformation())
        and the position will be transformed before being output to
        the file.

     */
    static EGS_GeometryTester *getGeometryTester(EGS_Input *i);

protected:

    EGS_PrivateTester *p;  //!< Hides the implementation details.

};


#endif
