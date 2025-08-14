
// Testing framework
#include "catch.hpp"

// The tested class
#include "egs_vector.h"

TEST_CASE("EGS_Vector Constructors"){

    // default
    EGS_Vector def;
    REQUIRE( def.isZero() );

    // value init
    EGS_Vector zhat(0, 0, 1);
    REQUIRE( zhat[0] == 0 );
    REQUIRE( zhat[1] == 0 );
    REQUIRE( zhat[2] == 1 ); 

    // copy constructor
    EGS_Vector copied(zhat);
    REQUIRE( copied == zhat );

    // assignment
    EGS_Vector assigned = zhat;
    REQUIRE( assigned == zhat );

    // pointer with destructor
    EGS_Vector* xhat = new EGS_Vector(1, 0, 0);
    REQUIRE( xhat->x == 1 );
    REQUIRE( xhat->y == 0 );
    REQUIRE( xhat->z == 0 );
    delete xhat;

}

TEST_CASE("EGS_Vector Comparators"){

    // check that floating numbers don't trick the EGS_Vector
    // implementation of equals
    EGS_Vector one(   3,  5,  7);
    EGS_Vector two(  11, 13, 17);
    EGS_Vector three(14, 18, 24);
    REQUIRE ( (one+two) == three );
    REQUIRE (one != two);


    // set tolerance in place
    // (11-3)^2 + (13-5)^2 + (17-7)^2 = 64+64+100 = 228
    REQUIRE( (one-two).setTolerance(230).isZero() );
    REQUIRE( !(-two+one).setTolerance(220).isZero() );


}

TEST_CASE("EGS_Vector Standard math operators"){

    // summation and subtraction
    EGS_Vector one(   3,  5,  7);
    EGS_Vector two(  11, 13, 17);
    EGS_Vector three(14, 18, 24);
    REQUIRE ( (one+two) == three );
    REQUIRE ( (three-two) == one );

    // inplace + and -
    one += two;
    REQUIRE( one == three );
    one -= two;
    REQUIRE( one != three );

    // multiplication and division
    one *= 1.5;
    REQUIRE( one == EGS_Vector(4.5, 7.5, 10.5) );
    REQUIRE( one == 1.5 * EGS_Vector(3, 5, 7) );
    REQUIRE( one == EGS_Vector(3, 5,  7) * 1.5 );
    REQUIRE( one/1.5 == EGS_Vector(3, 5, 7) );
    one /= 1.5;
    REQUIRE( one == EGS_Vector(3, 5,  7) );

    // inner product
    REQUIRE( one.dot(two) == 3*11 + 5*13 + 7*17);
    REQUIRE( one*two == 3*11 + 5*13 + 7*17);

    // cross product
    REQUIRE( one.times(two) == EGS_Vector(5*17-7*13, 7*11-3*17, 3*13-5*11));
    REQUIRE( one%two == EGS_Vector(5*17-7*13, 7*11-3*17, 3*13-5*11));

    // L2 norm
    REQUIRE( one.length2() == 3*3 + 5*5 + 7*7 );
    REQUIRE( one.length() == sqrt(3*3 + 5*5 + 7*7) );

    // normalization
    REQUIRE( one.normalized() == EGS_Vector(3, 5, 7)/sqrt(3*3+5*5+7*7));
    one.normalize();
    REQUIRE( one == EGS_Vector(3, 5, 7)/sqrt(3*3+5*5+7*7));

}



TEST_CASE("EGS_Vector Representation as a string"){

    EGS_Vector one( 3,  5,  7);
    cout << one << endl;
    egsInformation("vector `one` as string = %s\n", one.toString().c_str());

    REQUIRE( one.toString() == "(3.000000, 5.000000, 7.000000)x10^0" );


    EGS_Vector small( 1e-5,  3.141592e-5, 2.718282e-5);
    REQUIRE( small.toString() == "(1.000000, 3.141592, 2.718282)x10^-5" );

    EGS_Vector mixed( 1e-5,  3e1,  7e8);
    REQUIRE( mixed.toString() == "(0.000001, 3.000000, 70000000.000000)x10^1" );

    EGS_Vector large( 1e8,  3.141592e8, 2.718282e8);
    REQUIRE( large.toString() == "(1.000000, 3.141592, 2.718282)x10^8" );

}

TEST_CASE("EGS_Vector Rotation of a vector"){

    EGS_Vector zhat( 0, 0, 1);
    float ctheta = 0, stheta=1;
    float cphi = 1, sphi = 0;

    zhat.rotate(ctheta, stheta, cphi, sphi);
    REQUIRE( zhat == EGS_Vector(1, 0, 0) );

}