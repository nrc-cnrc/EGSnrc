#ifndef EGS_HEXAGONAL_LATTICE_
#define EGS_HEXAGONAL_LATTICE_

#include "egs_base_geometry.h"
#include "egs_transformations.h"
#include "geometry/egs_gtransformed/egs_gtransformed.h"
#include "egs_rndm.h"

#ifdef WIN32

#ifdef BUILD_HEXAGONAL_LATTICE_DLL
#define EGS_HEXAGONAL_LATTICE_EXPORT __declspec(dllexport)
#else
#define EGS_HEXAGONAL_LATTICE_EXPORT __declspec(dllimport)
#endif
#define EGS_HEXAGONAL_LATTICE_LOCAL 

#else

#ifdef HAVE_VISIBILITY
#define EGS_HEXAGONAL_LATTICE_EXPORT __attribute__ ((visibility ("default")))
#define EGS_HEXAGONAL_LATTICE_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define EGS_HEXAGONAL_LATTICE_EXPORT
#define EGS_HEXAGONAL_LATTICE_LOCAL
#endif

#endif

/* Input Example
:start geometry:
	library            = egs_hexagonal_lattice
	name               = phantom_w_microcavity
	base geometry      = phantom
	subgeometry        = microcavity
	subgeometry index  = 0
	volumetric density = 4.94782e+11
    largest spacing    = 10000
:stop geometry:
*/

class EGS_DummyGeometry : 
            public EGS_BaseGeometry {

protected:

    EGS_BaseGeometry    *g;   //!< The geometry being transformed
    string              type; //!< The geometry type

public:

    /*! \brief Construct a geometry that is a copy of the geometry \a G 
    transformed by \a t
    */
    EGS_AffineTransform T;    //!< The affine transformation, no longer protected for quick debug
    EGS_DummyGeometry(EGS_BaseGeometry *G, const EGS_AffineTransform &t, 
            const string &Name = "") : EGS_BaseGeometry(Name), g(G), T(t) {
        type = g->getType(); type += "T"; nreg = g->regions();
        is_convex = g->isConvex();
        has_rho_scaling = g->hasRhoScaling();
    };

    ~EGS_DummyGeometry() { if( !g->deref() ) delete g; };

    void setTransformation(const EGS_AffineTransform &t) { T = t; };

    int computeIntersections(int ireg, int n, const EGS_Vector &x,
            const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        EGS_Vector xt(x), ut(u);
        T.inverseTransform(xt); T.rotateInverse(ut);
        return g->computeIntersections(ireg,n,xt,ut,isections);
    };
    bool isRealRegion(int ireg) const {
        return g->isRealRegion(ireg);
    };
    bool isInside(const EGS_Vector &x) { 
        EGS_Vector xt(x); T.inverseTransform(xt);
        return g->isInside(xt);
        //return g->isInside(x*T); 
    };
    int isWhere(const EGS_Vector &x) { 
        EGS_Vector xt(x); T.inverseTransform(xt);
        return g->isWhere(xt); 
        //return g->isWhere(x*T); 
    };
    int inside(const EGS_Vector &x) { return isWhere(x); };

    int medium(int ireg) const { return g->medium(ireg); };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x, 
            const EGS_Vector &u) {
        return ireg >= 0 ? g->howfarToOutside(ireg,x*T,u*T.getRotation()) : 0;
    };
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u, 
           EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) { 
        EGS_Vector xt(x), ut(u);
        T.inverseTransform(xt); T.rotateInverse(ut);
        int inew = g->howfar(ireg,xt,ut,t,newmed,normal); 
        //int inew = g->howfar(ireg,x*T,u*T.getRotation(),t,newmed,normal); 
        if( inew != ireg && normal ) 
            *normal = T.getRotation()*(*normal);
        return inew;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Vector xt(x); T.inverseTransform(xt);
        return g->hownear(ireg,xt);
        //return g->hownear(ireg,x*T);
    };

    int getMaxStep() const { return g->getMaxStep(); };

    // Not sure about the following.
    // If I leave the implementation that way, all transformed copies of a 
    // geometry share the same boolean properties. But that may not be 
    // what the user wants. So, I should implement both options: 
    // all copies share the same properties and the user has the options 
    // to define separate properties for each copy.
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

    const string &getType() const { return type; };

    EGS_Float getRelativeRho(int ireg) const {
        return g->getRelativeRho(ireg);
    }
    void setRelativeRho(int start, int end, EGS_Float rho);
    void setRelativeRho(EGS_Input *);

protected:

    /*! \brief Don't define media in the transformed geometry definition.

    This function is re-implemented to warn the user to not define 
    media in the definition of a transformed geometry. Instead, media should 
    be defined when specifying the geometry to be transformed.
    */
    void setMedia(EGS_Input *inp,int,const int *);

};

class EGS_HEXAGONAL_LATTICE_EXPORT EGS_SubGeometry : 
            public EGS_BaseGeometry {
protected:

    EGS_BaseGeometry*  base;     //!< The geometry within which the sub geometry appears
    EGS_DummyGeometry* sub;      //!< The sub geometry that could appear within base 
	int                ind;      //!< The region in base geom where we could encounter sub geom
	int                maxStep;  //!< The maximum number of steps
	EGS_Float          spacing;  //!< The closest distance between two neighbouring sub geoms
	EGS_Float          gap;      //!< The closest distance between two non-neighbouring sub geoms
	EGS_Float          halfGap;  //!< Half the closest distance between two non-neighbouring sub geoms
	bool               virt;     //!< Tells us whether or not we are in the sub geom
    string             type;     //!< The geometry type

public:

    EGS_SubGeometry(EGS_BaseGeometry *B, EGS_BaseGeometry *S, int i, /*EGS_Float r,*/ EGS_Float vd, EGS_Float width, const string &Name = "")
		: EGS_BaseGeometry(Name), base(B), ind(i), spacing(pow(4.0*sqrt(2.0)*vd,-1.0/3.0)*2.0)
	{
		sub             = new EGS_DummyGeometry(S,EGS_Vector(0,0,0));
        type            = base->getType(); type += " with "; type += sub->getType(); type += " in a hexagonal lattice";
		nreg            = base->regions() + sub->regions();
        is_convex       = false; //base->isConvex();
        has_rho_scaling = base->hasRhoScaling();
		virt            = false;
		maxStep         = width;
		gap             = spacing*sqrt(3.0);
    };

    ~EGS_SubGeometry()
	{
		if(!sub->deref())
			delete sub;
		if(!base->deref())
			delete base;
	};
	
	EGS_Vector closestPoint(const EGS_Vector &x)
	{
		double i1, j1, k1, i2, j2, k2, j3, j4;
		
		i1 = spacing*(round(x.x/spacing));
		j1 = gap*(round(x.y/gap));
		k1 = gap*(round(x.z/gap));
		i2 = spacing*(0.5+round(x.x/spacing-0.5));
		j2 = gap*(0.5+round(x.y/gap-0.5));
		k2 = gap*(0.5+round(x.z/gap-0.5));
		j3 = gap*(-0.25+round(x.y/gap+0.25));
		j4 = gap*(0.25+round(x.y/gap-0.25));
		
		EGS_Vector p1(i1,j1,k1);
		EGS_Vector p2(i1,j3,k2);
		EGS_Vector p3(i2,j2,k1);
		EGS_Vector p4(i2,j4,k2);
		
		EGS_Float dist1 = (x-p1).length2();
		EGS_Float dist2 = (x-p2).length2();
		EGS_Float dist3 = (x-p3).length2();
		EGS_Float dist4 = (x-p4).length2();
		
		if (dist1 < dist2 && dist1 < dist3 && dist1 < dist4)
		{
			//egsWarning("For %f,%f,%f nearest point is %f,%f,%f\n",x.x,x.y,x.z,p1.x,p1.y,p1.z);
			return p1;
		}
		if (dist2 < dist3 && dist2 < dist4)
		{
			//egsWarning("For %f,%f,%f nearest point is %f,%f,%f\n",x.x,x.y,x.z,p2.x,p2.y,p2.z);
			return p2;
		}
		if (dist3 < dist4)
		{
			//egsWarning("For %f,%f,%f nearest point is %f,%f,%f\n",x.x,x.y,x.z,p3.x,p3.y,p3.z);
			return p3;
		}
		//egsWarning("For %f,%f,%f nearest point is %f,%f,%f\n",x.x,x.y,x.z,p4.x,p4.y,p4.z);
		return p4;
	};
	
    int computeIntersections(int ireg, int n, const EGS_Vector &x, const EGS_Vector &u, EGS_GeometryIntersections *isections)
	{
		return base->computeIntersections(ireg,n,x,u,isections);
    };
	
    bool isRealRegion(int ireg) const
	{
		if (ireg < base->regions()) // If ireg is less than base regions
			return base->isRealRegion(ireg); // check against base regions
		return sub->isRealRegion(ireg - base->regions()); // then check subregions
    };
	
    bool isInside(const EGS_Vector &x)
	{
		return base->isInside(x);
    };
	
    int isWhere(const EGS_Vector &x)
	{
		int temp = base->isWhere(x);
		// Are we in the subgeom?
		if (temp == ind)
		{
			//base->printInfo();
			//egsWarning("\nChecking position!\n");
			//egsWarning("\t[%e,%e,%e]\n",x.x, x.y, x.z);
			//egsWarning("\twhere r = %e\n", sqrt((x.x*x.x)+(x.y*x.y)+(x.z*x.z)));
			//cout << "\tisWhere invoking setTransformation\n";
			sub->setTransformation(closestPoint(x));
			//egsWarning("\twith region = %d\n", sub->isWhere(x) + base->regions());
			if (sub->isInside(x))
				return sub->isWhere(x) + base->regions();
		}
		return temp; // otherwise base geom		
    };
	
    int inside(const EGS_Vector &x)
	{
		return isWhere(x);
	};

    int medium(int ireg) const
	{
		if (ireg >= base->regions()) // If ireg is greater than base regions
			return sub->medium(ireg-base->regions());
		return base->medium(ireg); // If in base region
	};

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x, const EGS_Vector &u)
	{
		if (ireg < 0) // Error catch, not inside
			return 0;
			
		// Are we in the subgeom
		if (ireg >= base->regions())
			return base->howfarToOutside(ind,x,u);
		return base->howfarToOutside(ireg,x,u);	
    };

	// This is where things get messy, this function will be trimodal, whether we are in non-ind base, ind base or sub
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u, EGS_Float &t, int *newmed=0, EGS_Vector *normal=0)
	{
		//egsWarning("%s howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",getName().c_str(),ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
		
		// Catch t=0 exception, which sometimes causes issues when making a lattice of lattices
		if (t < epsilon)
		{
			t = 0.0;
			return ireg;
		}
		
		// Are we in the subgeom?
		if (ireg >= base->regions())
		{
			sub->setTransformation(closestPoint(x));
		
			// Do the howfar call
			EGS_Float tempT = t;
			//{EGS_Vector xt(x); sub->T.inverseTransform(xt);egsWarning("%s sub->howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",getName().c_str(),ireg-base->regions(), xt.x, xt.y, xt.z, u.x, u.y, u.z, tempT);}
			int tempReg = sub->howfar(ireg-base->regions(),x,u,tempT,newmed,normal);
			
			// If we do leave the subgeom, we want to return the index
			// of the region of the base geometry we would be entering,
			// which is not necessarily ind if subgeom is at a boundary
			EGS_Vector newX = u; newX.normalize(); newX = x + (newX * tempT);
			
			if (base->isWhere(newX) != ind) // If we leave ind region of base
			{
				t = tempT;				
				
				//egsWarning("\tbase->howfar(%6d, [%e,%e,%e], [%e,%e,%e], %e)\n",ind, x.x, x.y, x.z, u.x, u.y, u.z, t);
				tempReg = base->howfar(ind,x,u,t,0,normal);
				if (newmed && tempReg >= 0)
					*newmed = base->medium(tempReg);
				//egsWarning("\treturn %d (%e)\n",tempReg,t);
				//egsWarning("\tnew x should be [%e,%e,%e]\n",x.x+t*u.x,x.y+t*u.y,x.z+t*u.z);
				
				if (t<0)
				{
					egsWarning("Returning negative t from subgeom of %s\n",getName().c_str());
					egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
					//egsFatal("Break 1!\n");
				}
				return tempReg;
			}
			
			if (!(tempReg+1)) // If we leave sub geom entirely
			{
				//egsWarning("\treturn %d (%e)\n",tempReg+base->regions(),t);
				//egsWarning("\tnew x should be [%e,%e,%e]\n",x.x+t*u.x,x.y+t*u.y,x.z+t*u.z);
				if (newmed)
					*newmed = base->medium(ind);
				t = tempT;
				return ind;
			}
			
			// else we stay in sub geom
			t = tempT;
			//egsWarning("\treturn %d (%e)\n",tempReg,t);
			//egsWarning("\tnew x should be [%e,%e,%e]\n",x.x+t*u.x,x.y+t*u.y,x.z+t*u.z);			
			if (t<0)
			{
				egsWarning("Returning negative t from subgeom of %s\n",getName().c_str());
				egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
				//egsFatal("Break 2!\n");
			}
			return tempReg+base->regions();
		}
		else if (ireg == ind) // If we are in the region that could contain subgeoms
		{
			// Determine the path travelled ------------------------------------------------ //
			EGS_Float tempT = t;
			//egsWarning("%s base->howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",getName().c_str(),ireg, x.x, x.y, x.z, u.x, u.y, u.z, tempT);
			base->howfar(ireg,x,u,tempT); // Get how far it is in temp
			EGS_Float max = tempT;
			
			// Iterate through the lattice ------------------------------------------------- //
			EGS_Float min = max, minX = max, minY = max, minZ = max; // Distance to closest subgeom for three different cases
			EGS_Vector unit = u; unit.normalize();
			EGS_Vector xInt = unit*(spacing/8.0/fabs(unit.x)), yInt = unit*(gap/8.0/fabs(unit.y)), zInt = unit*(gap/8.0/fabs(unit.z));
			EGS_Vector tempP; // Temporarily hold indices of subgeom we are testing against
			EGS_Vector finalP, finalX, finalY, finalZ; // Current closest intersecting subgeom
			
			EGS_Vector x0;
			EGS_Float max2 = max*max;
			
			x0 = x;
			while ((x-x0).length2() < max2) // Quit as soon as our testing position is beyond current closest intersection (which could be tempT)
			{
				tempT = minX;
				tempP = closestPoint(x0);
				sub->setTransformation(tempP);
				if (sub->howfar(-1,x,u,tempT)+1) // Intersection!
					if(tempT < minX && tempT > 0)
					{
						finalX = tempP;
						minX = tempT;
						break;
					}
				x0 += xInt;
			}
			
			x0 = x;
			while ((x-x0).length2() < max2) // Quit as soon as our testing position is beyond current closest intersection (which could be tempT)
			{
				tempT = minY;
				tempP = closestPoint(x0);
				sub->setTransformation(tempP);
				if (sub->howfar(-1,x,u,tempT)+1) // Intersection!
					if(tempT < minY && tempT > 0)
					{
						finalY = tempP;
						minY = tempT;
						break;
					}
				x0 += yInt;
			}
			
			x0 = x;
			while ((x-x0).length2() < max2) // Quit as soon as our testing position is beyond current closest intersection (which could be tempT)
			{
				tempT = minZ;
				tempP = closestPoint(x0);
				sub->setTransformation(tempP);
				if (sub->howfar(-1,x,u,tempT)+1) // Intersection!
					if(tempT < minZ && tempT > 0)
					{
						finalZ = tempP;
						minZ = tempT;
						break;
					}
				x0 += zInt;
			}
			
			finalP = finalX; min = minX;
			if (min > minY) {min = minY; finalP = finalY;}
			if (min > minZ) {min = minZ; finalP = finalZ;}
			
			// Check last point
			tempT = max;
			sub->setTransformation(closestPoint(x+unit*tempT));
			sub->howfar(-1,x,u,tempT);
			if (tempT < min  && tempT > 0)
			{
				min = tempT;
				finalP = closestPoint(x+unit*tempT);
			}
			
			// We did intersect subgeom
			if (min < max)
			{
				tempT = t;
				sub->setTransformation(finalP);
				int tempReg = sub->howfar(-1,x,u,t,newmed,normal);
				if (tempReg+1)
				{
					if (newmed && tempReg >= 0)
						*newmed = sub->medium(tempReg);
					//egsWarning("\treturn %d (%f)\n",tempReg+base->regions(),t);
					//egsWarning("\tnew x should be [%e,%e,%e]\n",x.x+t*u.x,x.y+t*u.y,x.z+t*u.z);
					if (t<0)
					{
						egsWarning("Returning negative t from region %d in base geom of %s\n", ind, getName().c_str());
						egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
						//egsFatal("Break 1!\n");
					}
					return tempReg+base->regions();
				}
			}
			
			// We didn't intersect subgeom
			int tempReg = base->howfar(ireg,x,u,t,newmed,normal);
			if (t<0)
			{
				egsWarning("Returning negative t from region %d in base geom of %s\n", ind, getName().c_str());
				egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
				//egsFatal("Break 2!\n");
			}
			if (newmed && tempReg >= 0)
				*newmed = base->medium(tempReg);
			
			//egsWarning("\treturn %d (%f)\n",tempReg,t);
			//egsWarning("\tnew x should be [%e,%e,%e]\n",x.x+t*u.x,x.y+t*u.y,x.z+t*u.z);
			return tempReg;
		}
		else // Not in region containing subgeoms, then it is quite easy
		{    // unless we enter directly into a subgeom when entering
		     // region ind
			int tempReg = base->howfar(ireg,x,u,t,newmed,normal);
			
			// If we enter region ind
			if (tempReg == ind)
			{
				// newX is the point of entrance
				EGS_Vector newX = u; newX.normalize(); newX = x + (newX * t); 
				
				sub->setTransformation(closestPoint(newX));
				int newReg = sub->isWhere(newX); // see what region we are in
				if (newReg+1)                    // in subgeom, if its not -1
				{                                // then return the proper
					if (newmed && newReg >= 0)   // media and region
						*newmed = sub->medium(newReg);
					//egsWarning("\treturn %d (%f)\n",newReg+base->regions(),t);
					//egsWarning("\tnew x should be [%e,%e,%e]\n",x.x+t*u.x,x.y+t*u.y,x.z+t*u.z);
					if (t<0)
					{
						egsWarning("Returning negative t from region not %d in base geom of %s\n", ind, getName().c_str());
						egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e])\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
						//egsFatal("Break 1!\n");
					}
					return newReg + base->regions();
				}
			}
			
			if (newmed && tempReg >= 0)
				*newmed = base->medium(tempReg);
			//egsWarning("\treturn %d (%e)\n",tempReg,t);
			//egsWarning("\tnew x should be [%e,%e,%e]\n",x.x+t*u.x,x.y+t*u.y,x.z+t*u.z);
			if (t<0)
			{
				egsWarning("Returning negative t from region not %d in base geom of %s\n", ind, getName().c_str());
				egsWarning("howfar(%7d,[%e,%e,%e],[%e,%e,%e],%e)\n",ireg, x.x, x.y, x.z, u.x, u.y, u.z, t);
				//egsFatal("Break 2!\n");
			}
			return tempReg;
		}
    };
	
    EGS_Float hownear(int ireg, const EGS_Vector &x)
	{
		//egsWarning("%s hownear(%6d,[%e,%e,%e])\n", getName().c_str(), ireg, x.x, x.y, x.z);
		
		EGS_Float temp, dist = base->hownear(ireg>=base->regions()?ind:ireg,x);
		if (ireg >= base->regions())
		{
			sub->setTransformation(closestPoint(x));
			temp = sub->hownear(ireg-base->regions(),x);
			//{EGS_Vector xt(x); sub->T.inverseTransform(xt);egsWarning("\t%s sub->hownear(%6d,[%e,%e,%e])\n",getName().c_str(), ireg-base->regions(), xt.x, xt.y, xt.z);}
			return (temp<dist)?temp:dist;
		}
		else if (ireg == ind)
		{
			// Check all nearby geometries, in case of weird subgeom shapes
			// with a 10% increased spacing/gap buffer to avoid roundoff
			EGS_Vector x0[7] = {EGS_Vector(x.x-gap*1.1,x.y,x.z),
							    EGS_Vector(x.x+gap*1.1,x.y,x.z),
							    EGS_Vector(x.x,x.y-gap*1.1,x.z),
							    EGS_Vector(x.x,x.y+gap*1.1,x.z),
							    EGS_Vector(x.x,x.y,x.z-gap*1.1),
							    EGS_Vector(x.x,x.y,x.z+gap*1.1),
							    EGS_Vector(x)};
			for (int i = 0; i < 7; i++)
			{
				sub->setTransformation(closestPoint(x0[i]));
				temp = sub->hownear(ireg-base->regions(),x);
				if (temp < dist)
					dist = temp;
			}
			//egsWarning("\treturn %e\n",dist);
			return dist;
		}
		//egsWarning("\treturn %e\n",base->hownear(ireg,x));
		return dist;
    };

    int getMaxStep() const
	{
		return maxStep;
    };

    // Not sure about the following.
    // If I leave the implementation that way, all transformed copies of a 
    // geometry share the same boolean properties. But that may not be 
    // what the user wants. So, I should implement both options: 
    // all copies share the same properties and the user has the options 
    // to define separate properties for each copy.
    bool hasBooleanProperty(int ireg, EGS_BPType prop) const
	{
		return base->hasBooleanProperty(ireg,prop);
    };
	
    void setBooleanProperty(EGS_BPType prop)
	{
		base->setBooleanProperty(prop);
    };
	
    void addBooleanProperty(int bit)
	{
		base->addBooleanProperty(bit);
    };
	
    void setBooleanProperty(EGS_BPType prop, int start, int end, int step=1)
	{
		base->setBooleanProperty(prop,start,end,step);
    };
	
    void addBooleanProperty(int bit, int start, int end, int step=1)
	{
		base->addBooleanProperty(bit,start,end,step);
    };

    const string &getType() const
	{
		return type;
	};

    EGS_Float getRelativeRho(int ireg) const
	{
		return base->getRelativeRho(ireg);
    }
    void setRelativeRho(int start, int end, EGS_Float rho);
    void setRelativeRho(EGS_Input *);

protected:

    /*! \brief Don't define media in the transformed geometry definition.

    This function is re-implemented to warn the user to not define 
    media in the definition of a transformed geometry. Instead, media should 
    be defined when specifying the geometry to be transformed.
    */
    void setMedia(EGS_Input *inp,int,const int *);

};

#endif
