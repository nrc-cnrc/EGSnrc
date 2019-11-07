// MevEGS - (C) 2018 Mevex Corp.
//
// JBT -> Header with functions representing equations for all of the quantities we want to output.
// I don't know that much about the physics and copied these equations from DMac's reverse translator.
//Eq'ns verified with MJR and against Tutor7pp.

#ifndef DOSEMATH
#define DOSEMATH

#include "../../egs++/egs_vector.h"
#include <vector>
#include <cassert>
#include <stdlib.h>

//contains pretty much everything related to calculating MevEGS's output quantities.
namespace dosemath {

  //a nice type for interacting with our data
  typedef std::vector<std::pair<std::string, std::vector<double>>> namedResults;

  //physics constants
  constexpr double joulesPerMeV = 1.60218e-13;
  constexpr double electronsPerCoulomb = 6.24150e18;

  //calculate volume of a tet defined by four vectors. I used egs_vectors because
  //they already have defined cross and dot products, but we can maybe get
  //speedups by moving away from them
  inline double getTetVolume(EGS_Vector a, EGS_Vector b, EGS_Vector c, EGS_Vector d) {
    auto vol = std::abs(((a - d) * ((b - d) % (c - d))) / 6) / 1000.0;
    //assert(std::abs(((a - d) * ((b - d) % (c - d))) / 6) > 0);
    //assert (vol > 0);
    return vol;
  }

  //overly simple function that just does a single divide, but Matt says we
  //might want to calculate this from the input spectrum later so I'm making
  //it a function anyways so we can just edit the body if need be
  inline double getEA(long long nParticles, double eTotal) {
    return eTotal / nParticles;
  }

  inline double getTetMass(double vol, double density) {
    return vol * density;
  }

  //The next few functions are calculations for output quantities.
  //Comments above each are matching lines of code from DMac's Reverse Translator

  //TetData(itet).EDensityPerCoulomb = TetData(itet).EFraction * Ea * 1000000 / TetData(itet).Vol
  inline double eDensityPerCoulomb(double eFraction, double Ea, double vol) {
    return eFraction * Ea * joulesPerMeV * electronsPerCoulomb / vol;
    //return eFraction * 1000000 / vol;
  }

  //TetData(itet).DosePerCoulomb = TetData(itet).EDensityPerCoulomb * TetData(itet).Vol / (1000 * TetData(itet).Mass)
  inline double dosePerCoulomb(double eFraction, double Ea, double mass) {
    return  eFraction * Ea * joulesPerMeV * electronsPerCoulomb / mass;
  }

  //Uncertainty = 100 * TetData(itet).Uncertainty / TetData(itet).EFraction
  inline double uncertaintyPercentage(double uncertainty, double eFraction) {
      if(eFraction > std::numeric_limits<double>::epsilon()) {
          return 100 * uncertainty / eFraction;
      }
      else {
        return 100;
      }
  }

  //Given all the tets for a mesh, calculates the volume of each tet. This is needed for
  //calculating other output quantities.
  std::vector<double> getTetVols(const std::vector<double>& coords) {
      auto numTets = coords.size() / 12;
      std::vector<double> volumes;
      volumes.reserve(numTets);

      //coords is xyz xyz xyz xyz
      for(std::size_t i = 0; i < coords.size(); i+=12) {
        volumes.emplace_back(getTetVolume(
          EGS_Vector(coords[i], coords[i+1], coords[i+2]),
          EGS_Vector(coords[i+3], coords[i+4], coords[i+5]),
          EGS_Vector(coords[i+6], coords[i+7], coords[i+8]),
          EGS_Vector(coords[i+9], coords[i+10], coords[i+11])
        ));

        if(volumes.back() == 0) std::cout << "[DOSEMATH] Tet " << i/12 << " has a volume of 0" << std::endl;
      }
      return volumes;
  }

  //Gets the [relative] density of each tet, used for calculating other output quantities
  std::vector<double> getTetDensities(std::vector<double> rhor, const double* densities, const std::vector<int>& indices, int count) {

    std::vector<double> densitiesVec;
    densitiesVec.reserve(count);

    for(int i = 0; i < count; i++) {
      double d = densities[indices[i]] * rhor[i];
      if(d == 0) std::cout << "[DOSEMATH] Tet " << i << " with media " << indices[i] << " has a density of 0" << std::endl;

      densitiesVec.emplace_back(d);
    }

    return densitiesVec;
  }

  //calculates the mass of each tet, used for calculating other output quantities
  std::vector<double> getTetMasses(std::vector<double> tetVols, std::vector<double> tetDensities) {

    std::vector<double> masses;
    masses.reserve(tetVols.size());

    assert(tetVols.size() == tetDensities.size());

    for(size_t i = 0; i < tetVols.size(); i++) {

      double mass = getTetMass(tetVols[i], tetDensities[i]);

      if(mass == 0) std::cout << "[DOSEMATH] Tet " << i << " has a mass of 0" << std::endl;

      masses.emplace_back(mass);
    }

    return masses;

  }

  //Vector functions for calculating outputs for the whole mesh

  //Gets Energy Density for every tet in the mesh.
  std::vector<double> getEDensities(std::vector<double> eFractions, double Ea, std::vector<double> tetVols){

    std::vector<double> densities;

    assert(eFractions.size() == tetVols.size());

    densities.reserve(eFractions.size());

    for(size_t i = 0; i < eFractions.size(); i++) {

      double density = eDensityPerCoulomb(eFractions[i], Ea, tetVols[i]);

      //TODO potentially add checks for nonfinite numbers if that problem occurs again,
      //or maybe flipping negatives if that's really what we have to do.

      densities.emplace_back(density);
    }

    return densities;

  }

  //Gets Dose Per Coulomb for every tet in the mesh.
  std::vector<double> getDoses(std::vector<double> eFractions, double Ea, std::vector<double> masses){

    std::vector<double> doses;

    assert(eFractions.size() == masses.size());

    doses.reserve(eFractions.size());

    for(size_t i = 0; i < eFractions.size(); i++) {

      doses.emplace_back(dosePerCoulomb(eFractions[i], Ea, masses[i]));
    }

    return doses;


  }

  //Gets uncertainty as a percentage for every tet in the mesh.
  std::vector<double> getUncertaintyPercentages(std::vector<double> uncertainties,
                                                std::vector<double> eFractions) {
    std::vector<double> percentages;
    assert(eFractions.size() == uncertainties.size());
    percentages.reserve(eFractions.size());

    for(size_t i = 0; i < eFractions.size(); i++) {
      percentages.emplace_back(uncertaintyPercentage(uncertainties[i], eFractions[i]));
    }

    return percentages;
  }
} // namespace dosemath

#endif
