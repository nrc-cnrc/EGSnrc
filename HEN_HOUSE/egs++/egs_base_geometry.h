/*
###############################################################################
#
#  EGSnrc egs++ base geometry headers
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
#                   Blake Walters
#
###############################################################################
*/


/*! \file     egs_base_geometry.h
 *  \brief    Base geometry class header file
 *  \IK
 ***************************************************************************/

#ifndef EGS_BASE_GEOMETRY_
#define EGS_BASE_GEOMETRY_

#include "egs_vector.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

class EGS_Input;
struct EGS_GeometryIntersections;

#ifdef BPROPERTY64
    typedef EGS_I64 EGS_BPType;
#elif defined BPROPERTY32
    typedef unsigned int EGS_BPType;
#elif defined BPROPERTY16
    typedef unsigned short EGS_BPType;
#else
    typedef unsigned char EGS_BPType;
#endif

/*! \brief Base geometry class. Every geometry class must be derived from
  EGS_BaseGeometry.

  \ingroup Geometry
  \ingroup egspp_main

  The EGS_BaseGeometry class specifies the EGSnrc geometry interface.
  See the \link Geometry geometry module \endlink documentation for
  more details.

  \todo Add time dependence

  \todo Deal with volume calculation. While it is trivial to provide a
  generic volume calculation by Monte Carlo integration, it would be
  braindead to use such an approach for geometries that can actually
  calculate the volume in each of their regions analytically.

*/

class label {
public:
    string      name;
    vector<int> regions;
};


class EGS_EXPORT EGS_BaseGeometry {

public:

    /*! \brief Construct a geometry named \a Name

        All geometries must have a unique name that is used to refer
        to them in the input file specifying the geometry.
     */
    EGS_BaseGeometry(const string &Name);

    /*! \brief Destructor

        If necessary, deallocates #region_media. It also removes
        the geometry from the internally maintained static geometry list.
     */
    virtual ~EGS_BaseGeometry();

    /*! \brief Is the geometry convex?

        A "convex" geometry is a geometry for which there is a single
        segment on a line that intersects it, which is inside the geometry.
        Example of convex geometries are spheres, cylinders, etc.
        An examples of non-convex geometries is a torus or a prism
        with a non-convex polygon as a base.
        Derived gometry classes should set #is_convex to \c false if
        they are not convex. This method is needed, for instance, for
        N-dimensional geometries (the N-dimensional geometry logic only
        applies if all dimensions are convex or if there is a single concave
        dimension, which is also the last dimension.
     */
    inline bool isConvex() const {
        return is_convex;
    };

    /*! \brief Returns the region index, if inside, or -1 if outside (obsolete)

        This method is obsolete, isInside() and isWhere() should be used
        instead.
     */
    virtual int inside(const EGS_Vector &x) = 0;

    /*! \brief Is the position \a x inside the geometry?

        This pure virtual method must be implemented by derived geometry
        classes to return \c true, if the position \a x is inside the
        geomtry and \c false otherwise.
    */
    virtual bool isInside(const EGS_Vector &x) = 0;

    /*! \brief In which region is poisition \a x?

        This pure virtual method must be implemented by derived geometry
        classes to return the index of the region to which
        \a x belongs, if the position \a x is inside the
        geomtry and -1 otherwise.
    */
    virtual int isWhere(const EGS_Vector &x) = 0;

    /*! \brief Find the bin to which \a xp belongs, given \a np bin edges \a p

        This static method uses a binary search and is provided for the
        convenience of the various classes that need to find a region index
        in order to implement the isWhere() method (e.g. sets of planes,
        cylinders, etc.). Arguably, it should not be a member of the base
        geomtry class.
      */
    static int findRegion(EGS_Float xp, int np, const EGS_Float *p) {
        int ml = 0, mu = np;
        while (mu - ml > 1) {
            int mav = (ml+mu)/2;
            if (xp <= p[mav]) {
                mu = mav;
            }
            else {
                ml = mav;
            }
        }
        return  mu - 1;
    };

    /*! \brief Calculate the distance to a boundary from \a x along the
      direction \a u.

      This is one of the 2 main geometry methods needed to implement the
      EGSnrc geometry specifcation (the other is hownear()). It must be
      implemented in derived geometry classes to calculate and return
      the following values, depending on \a ireg:

      \arg ireg >= 0
      Assume position \a x is in local region \a ireg and calculate the
      distance \a d to the next region boundary along the direction \a u.
      If \a d > \a t, simply return \a ireg.
      If \a d <= \a t, set \a t to \a d and return the new region index.

      \arg ireg < 0
      Assume that position \a x is outside. If the line defined by
      position \a x and direction \a u intersects the geometry,
      calculate the distance \a d to the entry point.
      If \a d <= \a t, set \a t to \a d and return the region index
      of the entry point, otherwise just return -1.
      If no intersection exists, or if \a d > \a t, just return -1.

      In both cases, if an intersection is found at \a d <= \a t,
      and if the pointer \a newmed is not \c null, set it
      to the medium index in the new region. In the same way, if
      \a normal is not \c null, set it to the normal vector at the
      intersection point. The normal is needed for visualization
      purpsoses only (i.e., \a normal is always \c null in a normal
      simulation).
     */
    virtual int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
                       EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) = 0;

    /*! Calculate the distance to the outer geometry boundary from \a x
        along the direction \a u.

        The base geometry provides a generic implementation for this method
        that uses howfar(). However, geometries that can can compute the
        distance to their outer boundary more rapidly should provide their
        own implementation. The reason for adding this method (May 2007)
        is that we have already run into 2 situations (CBCT and PET),
        where it is advantegeous to have this method for implementing
        advanced variance reduction techniques.

     */
    virtual EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                                      const EGS_Vector &u);

    /*! \brief Calculate the distance to a boundary for position \a x in
      any direction.

      This is the second main geometry method needed to implement the
      EGSnrc geometry specifcation (the other is howfar()).
      It must be implemented in derived classes to calculate the nearest
      distance to a boundary for position \a x in any direction assuming
      that \a x is in region \a ireg (if \a ireg >= 0)
      or that \a x is outside (if \a ireg < 0).
     */
    virtual EGS_Float hownear(int ireg, const EGS_Vector &x) = 0;

    /*! \brief Calculates the volume*relative rho (rhor) of region ireg.

      Currently only implemented in EGS_XYZGeometry
    */
    virtual EGS_Float getMass(int ireg) {
        return 1.0;
    }

    /*! \brief Returns region boundaries in direction determined by idir

      Currently only implemented in EGS_XYZGeometry, where idir=0--> X-boundaries,
      idir=1--> Y-boundaries, idir=2--> Z-boundaries
    */
    virtual EGS_Float getBound(int idir, int ind) {
        return 0.0;
    }

    /*! Returns number of planar slabs/cylinders/etc in direction idir

      Currently only implemented in EGS_XYZGeometry, where idir=0--> X-boundaries,
      idir=1--> Y-boundaries, idir=2--> Z-boundaries
    */
    virtual int getNRegDir(int idir) {
        return 0;
    }

    /*! \brief Returns the number of local regions in this geometry.

      The fact that this method is not virtual implies that derived
      geometry classes must set EGS_BaseGeometry::nreg to the number of
      regions in the geometry.
     */
    int regions() const {
        return nreg;
    };

    /*! \brief Returnes true if \a ireg is a real region, false otherwise

      This method is needed because of the region indexing style used by
      some composite geometries which results in a larger number of
      regions than actual regions. This method can be used in such cases
      to check if a region exists.
     */
    virtual bool isRealRegion(int ireg) const {
        return (ireg >= 0 && ireg < nreg);
    };

    /*! \brief Returns the medium index in region \a ireg

      If a re-implementation of this method is undertaken, it can be safely
      assumed that this method is only invoked with \a ireg inside the
      geometry (unless, of course, there is a bug)
     */
    virtual int medium(int ireg) const {
        return region_media ? region_media[ireg] : med;
    };

    /*! \brief Returns the maximum number of steps through the geometry
     *
     * This method is handy for detecting when a particle gets stuck
     * at a boundary
     */
    virtual int getMaxStep() const {
        return nreg+1;
    };

    /*! \brief Calculates intersection distances to region boundaries

      For a given position \a x, direction \a u and region number \a ireg,
      this method returns the number of intersections with the geometry
      and distances, medium indeces and relative mass densities
      for all intersections, or, if this number is larger than the size
      \a n of \a isections, -1 (but the \a n intersections are still put
      in \a isections). If the position is outside, the method checks
      if the trajectory intersects the geometry and if yes, puts in the
      first element of \a isections the distance to the entry point and
      then finds all other intersections as in the case of \a x inside.
     */
    virtual int computeIntersections(int ireg, int n, const EGS_Vector &x,
                                     const EGS_Vector &u, EGS_GeometryIntersections *isections);

    /*! \brief Set all regions to a medium with name \a Name

      This method obtains a index for the medium named \a Name from an
      internally maintained list of media and sets the medium index in
      all regions in the geometry to this index.

      \sa setMedium(int,int,const string &), setMedium(int),
      setMedium(int,int,int), setMedia() and nMedia().
     */
    void setMedium(const string &Name);

    /*! \brief Set every delta'th region between \a start and \a end
     * to the medium named \a Name

      This method is  similar to the previous method but only sets the medium
      index in all regions between \a start and \a end (inclusive).
      Note that...
     */
    void setMedium(int start, int end, const string &Name, int delta=1);

    /*! \brief Set all regions to a medium with index \a imed.

      Note that...
     */
    void setMedium(int imed) {
        med = imed;
    };

    /*! \brief Set every delta'th region between \a start and \a end
     * (inclusive) to \a imed.

      This method is similar to setMedium(int,int,constr string &,int).
      \sa setMedium(int), setMedium(int,int,constr string &,int) and
      setMedium(const string &)
     */
    void setMedium(int istart, int iend, int imed, int delta=1);

    /*! \brief Set the media in the geometry from the input pointed to
      by \a inp.

      This method sets the media from a composite property 'media input'
      that has the key <code><br>
          media = list of media<br>
      </code> and zero or more keys of the form <code><br>
          set medium = first last medium_index<br>
      </code> See PIRS-899 for more details.
     */
    void setMedia(EGS_Input *inp);

    /*!  \brief Get the number of media registered so far by all geometries.

    Returns the number of media registered sofar by all geometries.
    A static list of media names is maintained internally. Media names
    can be added using addMedium(), setMedium(int,int,const string&) or
    setMedium(const string&)
    */
    static int nMedia();

    /*!  \brief Get the name of medium with index \a ind

    Returns a pointer to the character array holding the name of the
    medium with index \a ind or \c null if there is no such medium index.
    */
    static const char *getMediumName(int ind);

    /*! \brief Add a medium or get the index of an existing medium.

    This function returns the index of a medium with the name \a medname
    in the static list of media names registered so far by all geometries.
    If a medium with name \a medname already exists, the index of this medium
    is returned. If no such medium name exists in the list, \a medname
    is first appended to the list and then its index is returned.
    */
    static int addMedium(const string &medname);

    /*! \brief Get the index of a medium named \a medname.

    If \a medname is found in the list of media, its index is returned.
    Otherwise the return value is -1.
    */
    static int getMediumIndex(const string &medname);

    /*! \brief Does this geometry object have a mass density scaling feature?

     */
    inline bool hasRhoScaling() const {
        return has_rho_scaling;
    };

    /*! \brief Get the relative mass density in region \a ireg

     */
    virtual EGS_Float getRelativeRho(int ireg) const {
        return rhor && ireg >= 0 && ireg < nreg ? rhor[ireg] : 1;
    };

    /*! \brief Set the relative mass density in regions.

     Sets the relative mass density to \a rho in all regions between
     start and end (inclusive).
     */
    virtual void setRelativeRho(int start, int end, EGS_Float rho);

    /*! \brief Set the relative mass density from an user input.

     Looks for input
     \verbatim
     set relative density = start end rho
     \endverbatim
     and sets the relative mass density to rho in all regions between
     start and end (inclusive).
     */
    virtual void setRelativeRho(EGS_Input *);

    /*! \brief Get the name of this geometry

      Every geometry must have a name and this method can be used to retrieve
      the name of a geometry.
     */
    const string &getName() const {
        return name;
    };

    /*! \brief Get the geometry type.

      This pure virtual function must be implemented in derived classes
      to return a short but descriptive geometry type string.
      getType() is used in printInfo() to describe a geometry.
     */
    virtual const string &getType() const = 0;

    /*! \brief Create a geometry (or geometries) from a given input

      This static function looks for geometry definition by quering
      for a composite property <code>geometry definition</code>.
      Every individual geometry definition is contained in a
      <code>geometry definition</code> child property <code>geometry</code>.
      The return value of this function is the geometry specified
      with <br>
      <code>  simulation geometry = name of an existing geometry </code><br>
      if such input exists or the last constructed geometry. If the input
      \a inp does not contain any valid geometry definitions, \c null
      is returned.

      Note that the <code>geometry definition</code> property is removed
      from the input \a inp by this method.

      \sa createSingleGeometry().
     */
    static EGS_BaseGeometry *createGeometry(EGS_Input *);

    /*! \brief Create a single geometry from the input \a inp.

     The input pointed to by \a inp must contain all information necessary
     to construct a geometry. The minimum information necessary is <br><code>
       library = the name of a geometry library <br>
       name = the name of this geometry
     <br></code>
     The library name should not include platform specific prefixes and
     file extensions. This function attemtps to dynamically load the
     library specified by the <code>library</code> key and to resolve the
     geometry creation function \c createGeometry that a geometry library
     must provide. If this succeeds, the geometry created by
     \c createGeometry using the input pointed to by \a inp is returned
     (if the input is not sufficient or valid to create the desired
     geometry, createGeometry will return \c null).
     */
    static EGS_BaseGeometry *createSingleGeometry(EGS_Input *inp);

    /*! \brief Clears (deletes) all geometries in the currently active geometry
               list.

        This function deletes all geometries in the currently active
        list of geometries and also clears the internally maintained list
        of media.
    */
    static void clearGeometries();

    /*! \brief Turn debugging on

     This is mainly useful in the development process of a
     new geometry. It sets the protected data member #debug to \a deb.
    */
    void setDebug(bool deb) {
        debug = deb;
    };

    /*! \brief Get a pointer to the geometry named \a Name.

     This function returns a pointer to the geometry named \a Name,
     if a geometry with such a name exists in the static list of
     geometries, or \c null if no such geometry exists.
     */
    static EGS_BaseGeometry *getGeometry(const string &Name);

    /*! \brief Get a unique geometry name

      This function is used to create a unique name for a nameless
      geometry. Normally geometry names should be specified explicitely
      when constructing geometries.
     */
    static string getUniqueName();

    /*! \brief Set the name of  the geometry from the input \a inp.

      This method looks for a key <code>name</code> in the input
      pointed to by \a inp and sets the name of the geometry to the value
      of the <code>name</code> key. Derived geometry classes should always
      call this function to set their name from the input provided to
      the geometry creation function.
     */
    void   setName(EGS_Input *inp);

    /*! \brief Is the boolean property \a prop set for region \a ireg ?
     */
    virtual bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        if (!bp_array) {
            return (prop & bproperty);
        }
        return ireg >= 0 && ireg < nreg ? prop & bp_array[ireg] : false;
    };

    /*! \brief Set the boolean properties of the entire geometry to \a prop
     *
     * \sa addBooleanProperty(int), setBooleanProperty(EGS_BPType,int,int,int)
     *     addBooleanProperty(int,int,int,int)
     */
    virtual void setBooleanProperty(EGS_BPType prop);

    /*! \brief Add a boolean property for the entire geometry by setting the
     *         bit'th bit
     *
     * \sa setBooleanProperty(EGS_BPType),
     *     setBooleanProperty(EGS_BPType,int,int,int),
     *     addBooleanProperty(int,int,int,int)
     */
    virtual void addBooleanProperty(int bit);

    /*! \brief Set the boolean properties of every \a step'th region between
     *  \a start and \a end (inclusive) to \a prop.
     *
     * \sa setBooleanProperty(EGS_BPType), addBooleanProperty(int),
     * addBooleanProperty(int,int,int,int)
     */
    virtual void setBooleanProperty(EGS_BPType prop, int start, int end,
                                    int step=1);

    /*! \brief Add a boolean property to every \a step'th region between
     * \a start and \a end (inclusive) by setting the bit'th bit
     *
     * \sa setBooleanProperty(EGS_BPType),addBooleanProperty(int),
     * setBooleanProperty(EGS_BPType,int,int,int)
     */
    virtual void addBooleanProperty(int bit, int start, int end, int step=1);

    /*! \brief Print information about this geometry.

     The default implementation of this function outputs a
     separation line, the geometry type (as returned by getType() ),
     the geometry name ( as returned by getName() ) and the number
     of regions in the geometry using egsInformation().
     Derived geometry classes should first call the base class implementation
     of printInfo() and then add additional useful infromation to the
     output using egsInformation().
     */
    virtual void printInfo() const;

    /*! \brief Describes all existing geometries.

      This static function outputs information about all currently existing
      geometries using their printInfo() method
     */
    static void describeGeometries();

    /*! \brief Increase the reference count to this geometry

      Composite geometries that use other geometries should increase
      their reference count using this method. This is needed to prevent
      a geometry being destructed that is still in use by some other geometry.
     */
    inline int ref() {
        return ++nref;
    };


    /*! \brief Decrease the reference count to this geometry

      Composite geometries should use this method to decrease
      the reference count to other geometries they use when
      destructed and delete the geometry, if the return value of this
      function is 0.
     */
    inline int deref() {
        return --nref;
    };

    /*! \brief Set the currently active geometry list.

    */
    static void setActiveGeometryList(int list);

    static int getLastError() {
        return error_flag;
    };

    static void resetErrorFlag() {
        error_flag = 0;
    };

    static void setBoundaryTolerance(EGS_Float btol) {
        epsilon = btol;
    };

    static EGS_Float getBoundaryTolerance() {
        return epsilon;
    };

    /*! \brief Get the list of all regions labeled with \a str */
    virtual void getLabelRegions(const string &str, vector<int> &regs);

    /*! \brief Get the name of the i-th explicit label in the geometry */
    virtual const string &getLabelName(const int i) {
        return labels[i].name;
    }

    /*! \brief Get the number of explicit labels in the geometry */
    virtual int getLabelCount() {
        return labels.size();
    }

    /*! \brief Set the labels from an input block */
    int setLabels(EGS_Input *input);

    /*! \brief Set the labels from an input string */
    int setLabels(const string &inp);

protected:

    /*! \brief Number of local regions in this geometry

    Must be set by derived geometry classes to be equal to the number of
    regions
    */
    int  nreg;

    /*! \brief Name of this geometry.

    Every geometry needs a name, so that it can be refered to by other
    geometries or objects.
    */
    string name;

    /*! \brief Array of media indeces

    This array is only allocated if the geometry has more than 1 region
    \em and not all regions have the same medium index and contains
    the media indeces of all regions in such cases.
    */
    short *region_media;

    /*! \brief Medium index

        This variable holds the medium index for geometries that have a
        single region or geometries with multiple regions filled with
        the same medium.
     */
    int med;


    /*! \brief Does this geometry have relative mass density scvaling?

     */
    bool has_rho_scaling;

    /*! \brief Array with relative mass densities.

     */
    EGS_Float *rhor;

    /*! \brief Number of references to this geometry.

    \sa ref() and deref()
    */
    int nref;

    /*! \brief Set media.

    This function repeaditley takes <code>set media</code> keys from
    the input pointed to by \a inp and assigns media indeces using
    the indeces stored in \a med_ind. Used internally by setMedia(EGS_Input *)
    but left as a protected method just in case it may be useful to
    derived geometry classes.
    */
    virtual void setMedia(EGS_Input *inp, int nmed, const int *med_ind);

    /*! \brief Debugging flag.

    This is handy during the development of a new geometry.
    */
    bool debug;

    /*! \brief Is this geometry convex?

    By default this data member is set to \c true.
    This should be overwritten by derived geometry classes that are not
    convex.

    \sa isConvex().
    */
    bool is_convex;

    /*! \brief A bit mask of boolean properties for the entire geometry
     *
     * Can be set using setBoolenProperty() or addBooleanProperty()
     */
    EGS_BPType   bproperty;

    /*! \brief An array of boolean properties on a region by region basis
     *
     * Only allocated if neede (i.e. not all regions in a geometry have
     * the same boolean properties.
     */
    EGS_BPType   *bp_array;

    /*! \brief Boundary tolerance for geometries that need it */
    static EGS_Float epsilon;

    /*! \brief Set to non-zero status if a geometry problem is encountered */
    static int       error_flag;

    /*! \brief Labels

        This variable holds the list of labels for the geometry. Each label
        consists of a name and a list of regions numbers associated
        with this label name.

        Labels can be associated to regions by using
        the <code>set label = name r1 r2 r3 ...</code> input when defining
        a geometry. This method returns a list of all regions associated
        with this label in the geometry. Notably, for compound geometries the
        labels can be defined locally, but this method returns the global
        region numbers in the final geometry. Labels are therefore useful to
        determine region numbers in complicated geometrical constructs. They
        are also useful to track region numbers upon modyfying the geometry.
     */
    vector<label> labels;

private:

    /*! \brief The currently active list of geometries.

     This varaible is needed when more than one EGS_Application derived
     application classes are active in a simulation. Each such appliccation
     should set the currently active list of geometries using
     setActiveGeometryList() before constructing the geometries, or using
     any of the other static EGS_BaseGeometry functions.
    */

    static int active_glist;

};

struct EGS_GeometryIntersections {
    EGS_Float t;     //!< distance to next region boundary
    EGS_Float rhof;  //!< relative mass density in that region
    EGS_I32   ireg;  //!< region index
    EGS_I32   imed;  //!< medium index
};

/*! \example geometry/egs_box/egs_box.cpp
    \dontinclude geometry/egs_box/egs_box.h

    The following describes the implementation of a box geometry that
    can serve as a guide to users for developing their own geometry classes.

    Our box geometry class will describe a parallelepiped of a user
    defined size that defines a single region filled with 1 medium.

    As our geometry must be derived from EGS_BaseGeometry, we include
    its header file. We also want to be able to transform the box
    from being centered about the origin and having its faces parallel
    to the x-, y- and z-axis to any other location and/or
    orientation. For that we will employ \link EGS_AffineTransform
    affine transformations \endlink and therefore also include the
    transformations header file:
    \skipline #include
    \until #include
    As with other geometries, we will be compiling our box class into
    a shared library. We have to therefore mark the symbols that
    will be exported from the library. For this purpose we define
    a macro \c EGS_BOX_EXPORT, which we define like this
    \until #define EGS_BOX_LOCAL
    \until #define EGS_BOX_LOCAL
    \until #endif
    \until #endif
    We are now ready to develop our box geometry class by deriving
    from EGS_BaseGeometry:
    \until public:
    The data members <code>ax,ay,az</code> are for the box size,
    the \c type static data member will be set to "EGS_Box" and used
    and the \c T data member will point to the affine transformation
    associated with boxes not located about the origin and/or rotated
    boxes. We now need a constructor and a destructor for box objects.
    We provide two different constructors: one for boxes where all
    sides have the same size (\em i.e. cubes)
    \until };
    and one for boxes with 3 different sides
    \until };
    In the destructor we simply deallocate the memory used by the
    affine transformation, if there was a transformation defined:
    \until };
    We can now start implementing the required geometry methods
    \link EGS_BaseGeometry::isInside() isInside() \endlink,
    \link EGS_BaseGeometry::isWhere() isWhere() \endlink,
    \link EGS_BaseGeometry::howfar() howfar() \endlink and
    \link EGS_BaseGeometry::hownear() hownear() \endlink.

    A point is inside a box about the origin if its x-position
    is between \c -ax/2 and \c ax/2, its y-position between
    \c -ay/2 and \c ay/2, etc. For a transformed box, the easiest way
    to determine if the position is inside the box is to apply the
    inverse transformation to the position and to then use the simple
    method for boxes about the origin. The \c isInside implementation
    therefore looks like this
    \until };
    (multiplying the position vector with \c *T from the right is an
    operation equivalent to transforming \c x with the inverse of \c *T)

    The implementation of the \c isWhere method is very easy: as we have
    a single region this function should return 0 if the position is
    inside and -1 if it is outside:
    \until };

    The implementation of the \c howfar method is also easiest if
    we first transform the position and direction to a frame where
    the box is centered about the origin and its sides are parallel
    to the axis:
    \skip int howfar
    \until if( T )
    The \c howfar logic is different for positions inside (\em i.e.
    \c ireg=0 ) and outside. For inside points, we must calculate the
    distances to the x-planes, y-planes and z-planes from the transformed
    point \c xp
    along the transformed direction \c up and take the minimum
    \c t1 of the three.
    If \c t1 is less than the intended step \c t,
    we must set \c t to \c t1 and return -1 (\em i.e. the particle
    will exit the box at the end of the step). If \c normal is not
    \c null, our \c howfar function is being called from the visualization
    utility and we must also calculate the normal of the plane being
    intersected to exit the box with the normal pointing towards the
    position of the particle. The implementation therefore looks like this
    \until return inew
    \until }
    When the position is outside, the logic is
    -# if the particle intersects the top or bottom side, check if
       the intersection point is between the left-right and east-west
       sides.
    -# If yes, this is the position where the trajectory is entering
       the box => return the required information
    -# If no, repeat 1-2 for left-right sides and if necessary for the
       east-west sides.
    -# If all of the above fails, the particle does not enter the box.

    The code is lengthy and not reproduced here.

    The next step is the implementation of the \c hownear function.
    As with the other methods, we first inverse transform the position
    and then calculate the minimum distance to a box about the origin
    \skip hownear
    \until xp
    If the position is inside the boxe (\em i.e. \c ireg=0), the
    minimum distance to a boundary is the minimum distance to any
    of the 6 faces:
    \until }
    If the position is outside, the algorithm is slightly more
    difficult and is left to the reader to understand from the
    implementation:
    \until };

    We reimplement the \c getType function to return \c type so that
    the type of our geometry class is known:
    \until };

    We also reimplement the \link EGS_BaseGeometry::printInfo()
    printInfo() \endlink function so that users of our
    geometry class can get a better description of the geometry object:
    \until };
    The actual implementation of \c printInfo is in the egs_box.cpp
    file, which includes \c egs_box.h and \c egs_input.h:
    \dontinclude geometry/egs_box/egs_box.cpp
    \until egs_input
    The latter is necessary to gain access to the definition of the
    EGS_Input class that is needed in the \c createGeometry function
    (see below). The \c printInfo implementation is simple:
    \skip ::printInfo
    \until }

    We also need to declare the static data member \c type
    \until string
    According to verious sources, compilers can produce better code
    if text strings are static and local to the library. Hence,
    we define the various error messages and the key we will be
    using to define the size of a box object as such:
    \until ebox_key1

    The remaining task now is to provide a C-style function named
    \c createGeometry for the box
    shared library that will be used to dynamically create box objects
    based on the information stored in an EGS_Input object.
    \until EGS_BOX_EXPORT
    Just to be sure, we check that \c input is not \c null and issue
    a warning and return \c null if it is:
    \until egsWarning
    We then check if the input object contains a <code>box size</code>
    property
    \until int err =
    and if it doesn't (indicated by \c err not being zero), issue
    a warning and return \c null:
    \until }
    Next we check if the input contains a definition if a transformation for
    our box geometry:
    \until EGS_AffineTransform
    Depending on whether there was one or three values to the
    <code>box size</code> key, we use the corresponding constructor
    to create the new box object, but refuse to create it if
    there was some other number of values (because, \em e.g., the user
    made a mistake in the input file):
    \until }
    We must now delete the affine transformation pointed to by \c t as
    the box object has made its own copy in the constructor
    \until delete
    We then use the \c setName and \c setMedia functions inherited from
    EGS_BaseGeometry to set the name of the newly created box and fill
    it with a medium
    \until result
    to then return the box
    \until }
    \until }

    Based on this \c createGeometry implementation, users will be able
    to use our box geometry class by having the following input in
    the input file
    \verbatim
    :start geometry:
        name = some_name
        box size = 3.5 1 7
        :start transformation:
            input defining the transformation
        :stop transformation
        :start media input:
            media = H2O
        :stop media input
    :stop geometry:
    \endverbatim
    The transformation related input can be of course missing in which
    case the box will be centered about the origin and has its faces
    parallel to the x-, y- and z-axis.

    If we would have been nice to the users of our box geometry class,
    we could have provided an easier way for defining the medium of
    the box by re-implementing the \link
    EGS_BaseGeometry::setMedia(EGS_Input *) setMedia() \endlink
    function, \em e.g. something along these lines:
    \verbatim
    void EGS_Box::setMedia(EGS_Input *inp) {
        string medname; int err = inp->getInput("medium",medname);
        if( !err ) setMedium(medname);
    }
    \endverbatim
    That way, the user could define the medium of the box by simply
    using
    \verbatim
    medium = H2O
    \endverbatim
    instead of <code>:start media input: ... :stop media input:</code>
    which is necessary for geometries with several regions and potentialy
    different media and therefore provided as default implementation.

    The final step is to provide a Makefile for our geometry DSO.
    \dontinclude geometry/egs_box/Makefile
    We include the config files containing various definitions for
    the make system:
    \skipline EGS_CONFIG
    \until my_machine
    We then must add the \c BUILD_BOX_DLL macro to the list of defined
    macros,
    \until DEFS
    set the name of the DSO,
    \until library
    define the source files needed to create the DSO,
    \until lib_files
    and tell the make system that our class depends on the
    egs_transformations.h header file,
    \until extra_dep
    We can then use the generic rules for building the DSO:
    \until include
    and because our only object file needed to build the library
    depends on \c %egs_box.cpp and \c %egs_box.h, we can also
    use the generic dependence rule
    \until make_depend
    That's all.


    Here is the complete source code of the EGS_Box class.<br>
    The header file:
    \include geometry/egs_box/egs_box.h
    The .cpp file:
    \include geometry/egs_box/egs_box.cpp
    The Makefile:
    \include geometry/egs_box/Makefile

*/

/* \example geometry/example1/geometry_example1.cpp

  Suppose that you frequently use the same complex geometry and you
  are tired of always having to include the long definition of this geometry
  into your input file. In this case it may be worthwhile to implement your
  own geometry DSO that constructs the geometry of interest with very little
  input (or even no input at all). This example illustrates how to do this
  programatically.

  As this is just an example, the geometry we will be constructing is not very
  complex: it consists of a cylinder inscribed in a sphere and the sphere
  inscribed in a box as shown in the figure ???. We want to be able to define
  the media of the cylinder, the sphere and the box in the input file
  (so that we can easily use different PEGS data sets, for example). All other
  dimensions of our example geometry are fixed.

  To create our new geometry, we make a new subdirectory in the egspp
  geometries subdirectory called \c example1. To be able to use the
  predefined rules for building geometry libraries we create a dummy
  header file \c %geometry_example1.h that does nothing else except to
  define the \c EGS_EXAMPLE1_DLL
  macro:
  \include geometry/example1/geometry_example1.h
  and put the implementation in \c %geometry_example1.cpp.
  \dontinclude geometry/example1/geometry_example1.cpp
  In this file we include the just created header file and the EGS_Input
  class header file to get access to its definition,
  needed in the \c %createGeometry() function (see below):
  \until egs_input
  We also need the declarations of the various geometry classes that we
  will use to construct our geometry:
  \until egs_envelope_geometry.h
  We then define the dimensions of the various geometrical structures
  \until box_size

  The remaining task is to implement a C-style \c %createGeometry() function
  exported by the DSO
  \until EGS_EXAMPLE1_EXPORT
  We check that the input object is valid and return \c null if it is not
  \until }
  We then look for a definition of the cylinder medium and print a warning if
  such a definition is missing:
  \skipline cyl_medium
  \until }
  We repeat basically the same code to obtain the sphere and the box
  media
  \until }
  \until }
  and return \c null if any of the media definition was missing:
  \until if
  Now we are ready to construct our geometry. We first create a set of
  parallel z-planes
  \until EGS_ZProjector
  As the planes will only be used by our geometry object internally, we don't
  care how the plane set is named and therefore give it a name using
  the static function EGS_BaseGeometry::getUniqueName().
  In a similar way we create a set of z-cylinders consisting of a single
  cylinder
  \until ZProjector
  and make a closed cylinder modeled as a 2D-geometry from the planes and the
  cylinder surface:
  \until getUniqueName
  We then set the medium of the cylinder
  \until setMedium
  The next step is to create the sphere as a set of spheres consisting of
  a single sphere
  \until getUniqueName
  and to fill it with the sphere medium defined in the input
  \until setMedium
  We then use an envelope geometry to inscribe the cylinder into the just
  created sphere:
  \until getUniqueName
  The next step is to create the outer box and fill it with the box medium
  defined in the input
  \until setMedium
  (as our geometry will always be centered about the origin, we pass a
   \c null affine transformation to the constructor of the box).
  We now inscribe the \c the_sphere geometry object (which is the sphere
  with the cylinder inscribed in it) into the just created box using
  again an envelope geometry,
  \until the_box
  set the name of \c the_box from the input using the inherited
  \link EGS_BaseGeometry::setName(EGS_Input*) setName() \endlink method,
  \until setName
  and return a pointer to the just created geometry
  \until }
  \until }
  That's all.

  In the Makefile we include the egspp make config files,
  \dontinclude geometry/example1/Makefile
  \skipline EGS_CONFIG
  \until my_machine
  set the C-preprocessor defines to the standard set of defines
  for our egspp configuration,
  \until DEFS
  define the name of our DSO,
  \until library
  and and the set of files needed to build the DSO
  \until lib_files
  Because we are using planes, cylinders, spheres, a ND-geometry, a box
  geometry and an envelope geometry to construct our geometry, we must
  link against the geometry libraries containing these geometries.
  We accomplish this by seting the \c link2_libs make variable to the
  list of needed libraries
  \until egs_genvelope
  We then include the standard rules for building a DSO
  \until include
  and set the dependencies of our object files
  \until make_depend
  We must also add the dependencies on the various geometry header files
  that we are including
  \until egs_envelope_geometry.h
  We can now build our geometry DSO by typing \c make and use
  it by including input like this in the input file
  \verbatim
  :start geometry:
      library = geometry_example1
      name = some_name
      cylinder medium = H2O521ICRU
      sphere medium = AL521ICRU
      box medium = AIR521ICRU
  :stop geometry:
  \endverbatim
  It is worth noting that we could have constructed this geometry as the
  \link EGS_UnionGeometry union \endlink of the cylinder pointed to
  by \c the_cyl, the sphere pointed to by \c sphere and the
  box pointed to by \c box. However, such an implementation would be
  much slower at run time because for each invocation of a geometry
  method the union will have to interogate the geometry methods of all
  3 objects. In contrast, the envelope geometry will be checking only
  against the box for points outside of the box, the box and the sphere for
  points inside the box but outside the sphere, the sphere and the cylinder
  for points inside the sphere but outside the cylinder and the cylinder
  only for points inside the cylinder.

  The complete source code of this example geometry is below
  \include geometry/example1/Makefile
  \include geometry/example1/geometry_example1.h

*/

/* \example geometry/example2/geometry_example2.cpp

  This example illustrates how to create the same geometry as
  described in
  <a href="geometry_2example1_2geometry__example1_8cpp-example.html">
  this example</a> but using EGS_BaseGeometry::createSingleGeometry
  to create the geometry from definitions stored in an EGS_Input objects.
  The header file is similar as in the previous
  <a href="geometry_2example1_2geometry__example1_8cpp-example.html">
  example</a>, but unlike in the previous example we don't need the
  header files of the various geometries being used.
  Our implementation consists of a single C-style function
  \c %createGeometry()
  \dontinclude geometry/example2/geometry_example2.cpp
  \skipline createGeometry
  As with the previous
  <a href="geometry_2example1_2geometry__example1_8cpp-example.html">
  example</a>, we check for valid input and extract the
  media of the cylinder, sphere and the box from the input
  \until !ok
  We will create the various geometry objects needed in our composite
  geometry by passing EGS_Input objects containing geometry definitions
  to the EGS_BaseGeometry::createSingleGeometry function.
  We start with the set of planes needed to close the cylinder.
  We first create an EGS_Input object named \c geometry that we will
  be passing to the geometry creation function:
  \until plane_input
  We then create properties for the library name,
  \until plane_library
  the plane-set type,
  \until plane_type
  the name of the set of planes,
  \until plane_name
  and the plane positions
  \until plane_positions
  and add them to the geometry definition
  \until plane_positions
  We then create the set of planes
  \until createSingleGeometry
  The definition and creation of the set of cylinders is very similar
  \until createSingleGeometry
  We now make a closed cylinder as a 2D geometry
  \until createSingleGeometry
  and set its medium
  \until setMedium
  In a similar fashion we create the sphere, use an envelope geometry
  to inscribe the cylinder into the sphere, create a box, and use again
  an envelope to inscribe the sphere into the box. The code is not shown
  here as it is lengthy and boring but can be found at the end of this page.
  Finally we set the name of the newly created geometry \c the_box and
  return it:
  \skipline the_box->setName
  \until }
  \until }

  The Makefile is now simpler as we don't need to link against the
  various geometry DSOs as they are loaded dynamically at run time
  by the EGS_BaseGeometry::createSingleGeometry function and
  our implementation also does not depend on the header files of the
  various geometry classes being used:
  \include geometry/example2/Makefile

  For completeness here is the header file:
  \include geometry/example2/geometry_example2.h
  and the complete implementation:
*/


#endif
