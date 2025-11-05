
// Testing framework
#include "catch.hpp"

// The tested class
#include "egs_rndm.h"          // EGS_RandomGenerator

// Auxiliary egs++ classes
#include "egs_config1.h"       // for EGS_Float

TEST_CASE("EGS_RandomGenerator Constructors") {

    // default
    EGS_RandomGenerator* def = EGS_RandomGenerator::defaultRNG();

    // with seed
    EGS_RandomGenerator* seeded = EGS_RandomGenerator::defaultRNG(42);

    // test destructors
    delete def;
    delete seeded;
}


TEST_CASE("EGS_RandomGenerator Uniform Distributions") {

    // default generator
    EGS_RandomGenerator* def = EGS_RandomGenerator::defaultRNG();

    size_t n_trials = 1e5;
    EGS_Float expected_mean = 0.5;
    EGS_Float expected_variance = 1/12.;
    EGS_Float expected_4th_moment = 1/80.;

    EGS_Float expected_unc_mean = sqrt(expected_variance / n_trials);
    EGS_Float expected_unc_variance = sqrt((expected_4th_moment - expected_variance * expected_variance) / n_trials);

    EGS_Float sum = 0, sumsqr = 0;
    // draw sample from the standard uniform distribution
    for (size_t itrial = 0; itrial < n_trials; itrial++) {
        EGS_Float sample = def->getUniform();
        sum += sample;
        sumsqr += sample*sample;
    }

    EGS_Float sample_mean = sum / n_trials;
    EGS_Float sample_variance = (sumsqr - n_trials * sample_mean * sample_mean) / (n_trials-1);

    // require mean to be within 5 standard devations of mu=0.5
    REQUIRE(abs(expected_mean - sample_mean) < 5 * expected_unc_mean);

    // require variance to be within 5 sigma of sigma^2 = 1/12
    REQUIRE(abs(expected_variance - sample_variance) < 5 * expected_unc_mean);

}

TEST_CASE("EGS_RandomGenerator Gaussian Distributions") {

    // default generator
    EGS_RandomGenerator* def = EGS_RandomGenerator::defaultRNG();

    size_t n_trials = 1e5;
    EGS_Float expected_mean = 0.;
    EGS_Float expected_variance = 1.;
    EGS_Float expected_4th_moment = 3.;

    EGS_Float expected_unc_mean = sqrt(expected_variance / n_trials);
    EGS_Float expected_unc_variance = sqrt((expected_4th_moment - expected_variance * expected_variance) / n_trials);

    EGS_Float sum = 0, sumsqr = 0;
    // draw sample from the standard gaussian distribution
    for (size_t itrial = 0; itrial < n_trials; itrial++) {
        EGS_Float sample = def->getGaussian();
        sum += sample;
        sumsqr += sample*sample;
    }

    EGS_Float sample_mean = sum / n_trials;
    EGS_Float sample_variance = (sumsqr - n_trials * sample_mean * sample_mean) / (n_trials-1);

    // require mean to be within 5 standard devations of mu=0.5
    REQUIRE(abs(expected_mean - sample_mean) < 5 * expected_unc_mean);

    // require variance to be within 5 sigma of sigma^2 = 1/12
    REQUIRE(abs(expected_variance - sample_variance) < 5 * expected_unc_mean);

}

TEST_CASE("EGS_RandomGenerator Random Directions on Sphere") {

    // default generator
    EGS_RandomGenerator* def = EGS_RandomGenerator::defaultRNG();

    size_t n_trials = 1e5;

    // mu expected values
    EGS_Float expected_mean_mu = 0.;
    EGS_Float expected_variance_mu = pow(2., 2)/12.;
    EGS_Float expected_4th_moment_mu = pow(2., 4)/80.;

    EGS_Float expected_unc_mean_mu = sqrt(expected_variance_mu / n_trials);
    EGS_Float expected_unc_variance_mu = sqrt((expected_4th_moment_mu
                                               - expected_variance_mu * expected_variance_mu) / n_trials);

    // phi expected values
    EGS_Float expected_mean_phi = 0.;
    EGS_Float expected_variance_phi = pow(2*M_PI, 2)/12.;
    EGS_Float expected_4th_moment_phi = pow(2*M_PI, 4)/80.;

    EGS_Float expected_unc_mean_phi = sqrt(expected_variance_phi / n_trials);
    EGS_Float expected_unc_variance_phiu = sqrt((expected_4th_moment_phi
                                               - expected_variance_phi * expected_variance_phi) / n_trials);

    EGS_Float sum_mu = 0, sumsqr_mu = 0;
    EGS_Float sum_phi = 0, sumsqr_phi = 0;
    // draw sample from the standard r_hat distribution
    for (size_t itrial = 0; itrial < n_trials; itrial++) {
        EGS_Vector sample = def->randomDir();

        EGS_Float cos_theta = sample.z;
        sum_mu += cos_theta;
        sumsqr_mu += cos_theta * cos_theta;

        EGS_Float phi = atan2(sample.y, sample.x);
        sum_phi += phi;
        sumsqr_phi += phi * phi;
    }

    // mu requires ---
    EGS_Float sample_mean_mu = sum_mu / n_trials;
    EGS_Float sample_variance_mu = (sumsqr_mu - n_trials * sample_mean_mu * sample_mean_mu) / (n_trials-1);

    // require mean to be within 5 standard devations of mu=0.5
    REQUIRE(abs(expected_mean_mu - sample_mean_mu) < 5 * expected_unc_mean_mu);

    // require variance to be within 5 sigma of sigma^2 = 1/12
    REQUIRE(abs(expected_variance_mu - sample_variance_mu) < 5 * expected_unc_mean_mu);

    // phi requires ---
    EGS_Float sample_mean_phi = sum_phi / n_trials;
    EGS_Float sample_variance_phi = (sumsqr_phi - n_trials * sample_mean_phi * sample_mean_phi) / (n_trials-1);

    // require mean to be within 5 standard devations of mu=0.5
    REQUIRE(abs(expected_mean_phi - sample_mean_phi) < 5 * expected_unc_mean_phi);

    // require variance to be within 5 sigma of sigma^2 = 1/12
    REQUIRE(abs(expected_variance_phi - sample_variance_phi) < 5 * expected_unc_mean_phi);

}


TEST_CASE("EGS_RandomGenerator Random Directions around other EGS_Vector") {

    // default generator
    EGS_RandomGenerator* def = EGS_RandomGenerator::defaultRNG();

    size_t n_trials = 1e5;

    // around a known vector ------------------------------------------------------------------------------------------
    EGS_Vector zhat(0, 0, 1);

    // phi expected values
    EGS_Float expected_mean_phi = 0.;
    EGS_Float expected_variance_phi = pow(2*M_PI, 2)/12.;
    EGS_Float expected_4th_moment_phi = pow(2*M_PI, 4)/80.;

    EGS_Float expected_unc_mean_phi = sqrt(expected_variance_phi / n_trials);
    EGS_Float expected_unc_variance_phiu = sqrt((expected_4th_moment_phi
                                               - expected_variance_phi * expected_variance_phi) / n_trials);

    EGS_Float sum_phi = 0, sumsqr_phi = 0;
    // draw sample from the standard r_hat distribution
    for (size_t itrial = 0; itrial < n_trials; itrial++) {
        EGS_Vector sample = def->randomDirOrthogonalTo(zhat);

        REQUIRE(abs(sample.dot(zhat)) < 1e-5);

        EGS_Float phi = atan2(sample.y, sample.x);
        sum_phi += phi;
        sumsqr_phi += phi * phi;
    }

    // phi requires ---
    EGS_Float sample_mean_phi = sum_phi / n_trials;
    EGS_Float sample_variance_phi = (sumsqr_phi - n_trials * sample_mean_phi * sample_mean_phi) / (n_trials-1);

    // require mean to be within 5 standard devations of mu=0.5
    REQUIRE(abs(expected_mean_phi - sample_mean_phi) < 5 * expected_unc_mean_phi);

    // require variance to be within 5 sigma of sigma^2 = 1/12
    REQUIRE(abs(expected_variance_phi - sample_variance_phi) < 5 * expected_unc_mean_phi);


    // around a random vector ------------------------------------------------------------------------------------------

    // draw sample from the standard r_hat distribution
    for (size_t itrial = 0; itrial < n_trials; itrial++) {

        EGS_Vector sample_1 = def->randomDir();
        EGS_Vector sample_2 = def->randomDirOrthogonalTo(sample_1);

        REQUIRE(abs(sample_1.dot(sample_2)) < 1e-5);
    }

}
