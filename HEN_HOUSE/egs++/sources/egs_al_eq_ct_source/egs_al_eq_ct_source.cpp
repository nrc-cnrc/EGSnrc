/*
###############################################################################
#
#  EGSnrc egs++ egs_al_eq_ct_source
#
###############################################################################
#
#  Author:          Marie-Luise Kuhlmann, 2021
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_al_eq_ct_source.cpp
 *  \brief A aluminium equivalent ct source
 */

#include "egs_al_eq_ct_source.h"
#include "egs_input.h"
#include "egs_math.h"
#include "egs_rndm.h"
#include "egs_alias_table.h"
#include "egs_base_source.h"
#include "smooth_array.h"
#include "egs_spectra.cpp"
#include <map>
// Interpolators
#include "egs_interpolator.h"

#include <cstdio>
#include <fstream>
#ifndef NO_SSTREAM
    #include <sstream>
    #define S_STREAM std::istringstream
#else
    #include <strstream>
    #define S_STREAM std::istrstream
#endif

#include <complex>
#include <limits>
#include <iostream>

EGS_AlEqCtSource::EGS_AlEqCtSource(EGS_Input *input,
                                   EGS_ObjectFactory *f): EGS_BaseSimpleSource(input,f), ctry(0), dist(1), collimation(1), valid(true), table(0) {

    // Read in distance from source to isocenter
    EGS_Float auxd;
    int errd = input->getInput("distance",auxd);
    if (!errd) {
        dist = auxd;
    }
    // Read in collimation
    EGS_Float auxcol;
    int errcol = input->getInput("collimation",auxcol);
    if (!errcol) {
        collimation = auxcol;
    }

    // Read in distribution file
    std::string distribution_file;
    int err_dist= input->getInput("bowtie distribution",distribution_file);
    table = read_distribution_file(distribution_file, false, true);
    if (!table) {
        egsWarning("EGS_AlEqCtSource: Distribution was not created\n");
    }


    // Get basespectrum in the middle of the bowtie filter
    EGS_AliasTable *basespec;
    EGS_TabulatedSpectrum *spectrum;
    spectrum = static_cast<EGS_TabulatedSpectrum *>(s);
    basespec = new EGS_AliasTable(spectrum->get_length(),spectrum->getX(),spectrum->getF(),1);



    // Read in filterthickness file
    std::string filterthickness_file;
    int err_filter = input->getInput("al equivalent bowtie",filterthickness_file);
    EGS_Float *filter = read_filterthickness_from_file(filterthickness_file);

    string AL_ATTENUATION_FILE = "$HEN_HOUSE/egs++/sources/egs_al_eq_ct_source/AL_attenuation_coefficient.txt";
    EGS_Interpolator *mu_table = read_mu_table(AL_ATTENUATION_FILE);


    spectra_dict = get_spectrum_dict(basespec, filter, table, mu_table);



    setUp();
};

void EGS_AlEqCtSource::setUp() {
    otype = "EGS_AlEqCtSource";
    if (!isValid()) {
        description = "Invalid al equivalent ct source";
    }
    else {
        description = "al equivalent ct source";
    }
};

extern "C" {

    EGS_AL_EQ_CT_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_AlEqCtSource>(input,f,"al equivalent ct source");
    }
};

void save_spectrum_file(EGS_Float *energy, EGS_Float *probs, int num_values, string output_file) {

    std::ofstream outdata;

    outdata.open(output_file); // opens the file
    if (!outdata) {  // file couldn't be opened
        std::cerr << "Error: file could not be opened" << std::endl;
        exit(1);
    }

    for (int i=0; i<num_values; ++i) {
        outdata << energy[i]*1000 << " " << probs[i] << std::endl;
    }

    outdata.close();


};

EGS_AliasTable *EGS_AlEqCtSource::calculate_filtered_spectrum(EGS_AliasTable *basespec, EGS_Float al_thickness, int counter, EGS_Interpolator *mu_table) {

    EGS_Float *energy_spec, *prob_array, *new_prob_array, *energy_mu, *mu;
    prob_array = basespec->getF();
    energy_spec = basespec->getX();
    int num_values = basespec->get_length();
    new_prob_array = new EGS_Float [num_values];

    EGS_Float mu_temp;
    for (int i = 0; i < num_values; i++) {
        mu_temp = mu_table->interpolateFast(energy_spec[i]);
        EGS_Float exp_fac= exp(-1 * (mu_temp*AL_DENSITY*al_thickness*0.1));
        new_prob_array[i] = prob_array[i]*exp_fac;
    }

    int itype = 0;
    int nb = num_values;
    std::string counter_string;
    if (counter < 10) {
        counter_string = "00"+std::to_string(counter);
    }
    else if (counter < 100) {
        counter_string = "0"+std::to_string(counter);
    }
    else {
        counter_string = std::to_string(counter);
    }
    //std::string file_name = "/home/malu/EGSnrc/EGSnrc/egs_home/cavity/" + counter_string + '_' + std::to_string(al_thickness) + "_spectrum.dat";
    //save_spectrum_file(energy_spec,new_prob_array, num_values, file_name);
    EGS_AliasTable *filtered_spec;
    filtered_spec = new EGS_AliasTable(nb,energy_spec,new_prob_array,itype);
    delete new_prob_array;

    return filtered_spec;

};


EGS_Interpolator *EGS_AlEqCtSource::read_mu_table(string emuen_file) {

    emuen_file = egsExpandPath(emuen_file);
    EGS_Interpolator *E_Muen_Rho;

    ifstream muen_data(emuen_file.c_str());
    if (!muen_data) {
        egsFatal(
            "\n\n***  Failed to open mu file %s\n"
            "     This is a fatal error\n", emuen_file.c_str());
    }
    else {
        egsInformation(
            "\n\n=============== Calculation of attenuated spectra ===============\n"
            "mu file: %s\n"
            "=============================================\n\n",
            emuen_file.c_str());

    }

    int ndat;

    muen_data >> ndat;
    if (ndat < 2 || muen_data.fail()) egsFatal(
            "\n\n*** Failed to read mu data file\n");
    EGS_Float *xmuen = new EGS_Float [ndat];
    EGS_Float *fmuen = new EGS_Float [ndat];
    for (int j=0; j<ndat; j++) {
        muen_data >> xmuen[j] >> fmuen[j];
    }
    if (muen_data.fail()) egsFatal(
            "\n\n*** Failed to read mu data file\n");

    E_Muen_Rho = new EGS_Interpolator(ndat,log(xmuen[0]),
                                      log(xmuen[ndat-1]),fmuen);

    delete [] xmuen;
    delete [] fmuen;

    return E_Muen_Rho;
}


std::map<EGS_Float, EGS_AliasTable *> EGS_AlEqCtSource::get_spectrum_dict(EGS_AliasTable *basespec, EGS_Float *thickness_list, EGS_AliasTable *distribution, EGS_Interpolator *mu_table) {

    std::map<EGS_Float, EGS_AliasTable *> spectrum_map;
    EGS_Float *angular = distribution->getX();
    int num_angle = distribution->get_length();
    for (int i = 0; i < num_angle; i++) {
        EGS_AliasTable *filtered_spec = calculate_filtered_spectrum(basespec, thickness_list[i], i, mu_table);
        spectrum_map[angular[i]] = filtered_spec;
    }

    return spectrum_map;
};


EGS_Float *EGS_AlEqCtSource::read_filterthickness_from_file(string file) {
    static char thicknessarray_msg1[] = "EGS_AlEqCtSource::read_filterthickness_from_file";

    std::ifstream sdata(file.c_str());
    if (!sdata) {
        egsWarning("%s failed to open thickness file %s\n",
                   thicknessarray_msg1,file.c_str());
        return 0;
    }
    else {
        int num_header_lines = 8;
        char line[1024];
        for (int j = 0; j < num_header_lines; j++) {
            sdata.getline(line, 1023);
            if (sdata.eof() || sdata.fail() || !sdata.good()) {
                egsWarning("%s error while reading title of filter thickness file"
                           "%s\n",thicknessarray_msg1,file.c_str());
                return 0;
            }
        }
        int num_values;
        EGS_Float *thickness_array;
        char c[6];
        sdata.getline(c,5);
        num_values = std::stof(c);

        thickness_array = new EGS_Float[num_values];
        char value[100];
        sdata.getline(value,99);
        if (sdata.eof() || sdata.fail() || !sdata.good()) {
            egsWarning("%s error while reading title of filter thickness file"
                       "%s\n",thicknessarray_msg1,file.c_str());
            return 0;
        }
        for (int i = 0; i<num_values; i++) {
            sdata.getline(value,99);
            if (sdata.eof() || sdata.fail() || !sdata.good()) {
                egsWarning("%s error while reading title of filter thickness file"
                           "%s\n",thicknessarray_msg1,file.c_str());
                return 0;
            }
            EGS_Float val_0;

            EGS_Float val = std::stof(value);
            EGS_Float val_new;
            if (i == 0) {
                val_0 = val;
            }

            if (val - val_0 < 0) {
                val_new = 0;
            }
            else {
                val_new = val - val_0;
            }
            //std::cout << val_new << std::endl;
            thickness_array[i] = val_new;
        }
        //thickness_array[0] = 0;
        return thickness_array;
    }

};

istream &skipsepp(istream &in) {
    char c;
    for (EGS_I64 loopCount=0; loopCount<=loopMax; ++loopCount) {
        if (loopCount == loopMax) {
            egsFatal("skipsep: Too many iterations were required! Input may be invalid, or consider increasing loopMax.");
            return in;
        }
        in.get(c);
        if (in.eof() || in.fail() || !in.good()) {
            break;
        }
        if (c == ',') {
            break;
        }
        if (!isspace(c)) {
            in.putback(c);
            break;
        }
    }
    return in;
};

EGS_AliasTable *EGS_AlEqCtSource::read_spectrum(string spec_file, int err) {
    // Expands FIRST environment variable found in spec_file

    spec_file = egsExpandPath(spec_file);
    EGS_AliasTable *spec;
    if (!err) {
        ifstream sdata(spec_file.c_str());
        if (!sdata) egsWarning("%s failed to open spectrum file %s\n",
                                   spec_file.c_str());
        else {
            char title[1024];
            sdata.getline(title,1023);
            if (sdata.eof() || sdata.fail() || !sdata.good()) {
                egsWarning("%s error while reading title of spectrum file"
                           "%s\n",spec_file.c_str());
                return 0;
            }
            if (sdata.eof() || sdata.fail() || !sdata.good()) {
                egsWarning("%s error while reading spectrum type and "
                           "number of bins in spectrum file %s\n",
                           spec_file.c_str());

                return 0;
            }
            EGS_Float dum;
            int nbin, mode;
            sdata >> nbin >> skipsepp >> dum >> skipsepp >> mode;
            if (sdata.eof() || sdata.fail() || !sdata.good()) {
                egsWarning("%s error while reading spectrum type and "
                           "number of bins in spectrum file %s\n",
                           spec_file.c_str());
                return 0;
            }
            if (nbin < 2) {
                egsWarning("%s nbin in a spectrum must be at least 2\n"
                           "  you have %d in the spectrum file %s\n",
                           nbin,spec_file.c_str());
                return 0;
            }
            if (mode < 0 || mode > 3) {
                egsWarning("%s unknown spectrum type %d in spectrum file"
                           " %s\n",mode,spec_file.c_str());
                return 0;
            }
            EGS_Float *en_array, *f_array;
            int ibin;
            f_array = new EGS_Float [nbin];
            if (mode == 0 || mode == 1) {
                en_array = new EGS_Float [nbin+1];
                en_array[0] = dum;
                ibin=1;
            }
            else {
                en_array = new EGS_Float [nbin];
                ibin=0;
            }
            for (int j=0; j<nbin; j++) {
                sdata >> en_array[ibin++] >> skipsepp >> f_array[j];
                if (sdata.eof() || sdata.fail() || !sdata.good()) {
                    egsWarning("%s error on line %d in spectrum file %s\n",
                               j+2,spec_file.c_str());
                    delete [] en_array;
                    delete [] f_array;
                    return 0;
                }
                if (mode != 2 && ibin > 1) {
                    if (en_array[ibin-1] <= en_array[ibin-2]) {
                        egsWarning("%s energies must be in increasing "
                                   "order.\n   This is not the case for input on "
                                   "lines %d,%d in spectrum file %s\n",
                                   j+2,j+1,spec_file.c_str());
                        return 0;
                    }
                }
                if (mode == 0) {
                    f_array[j]/=(en_array[ibin-1]-en_array[ibin-2]);
                }
            }
            int itype = 1;
            if (mode == 2) {
                itype = 0;
            }
            else if (mode == 3) {
                itype = 2;
            }
            int nb = itype == 1 ? nbin+1 : nbin;

            spec = new EGS_AliasTable(nb,en_array,f_array,itype);
            delete [] en_array;
            delete [] f_array;
        }
    }
    return spec;
}

EGS_AliasTable *EGS_AlEqCtSource::read_distribution_file(string distribution_file, bool print, bool smooth) {

    static char distribution_msg1[] = "EGS_AlEqCtSource::read_distribution_file";

    bool delete_it = false;
    //std::cout << "hallo!" << distribution_file << std::endl;

    EGS_AliasTable *table;
    std::ifstream sdata(distribution_file.c_str());
    if (!sdata) {
        egsWarning("%s failed to open distribution file %s\n",
                   distribution_msg1,distribution_file.c_str());
        return 0;
    }
    else {
        char title[1024];
        sdata.getline(title,1023);
        if (sdata.eof() || sdata.fail() || !sdata.good()) {
            egsWarning("%s error while reading title of distribution file"
                       "%s\n",distribution_msg1,distribution_file.c_str());

            return 0;
        }
        if (sdata.eof() || sdata.fail() || !sdata.good()) {
            egsWarning("%s error while reading distribution type and "
                       "number of bins in distribution file %s\n",
                       distribution_msg1,distribution_file.c_str());

            return 0;
        }
        EGS_Float dum;
        int nbin, mode;
        sdata >> nbin >> skipsepp >> dum >> skipsepp >> mode;
        if (print) {
            std::cout << nbin << std::endl;
            std::cout << dum << std::endl;
            std::cout << mode << std::endl;
        }

        if (sdata.eof() || sdata.fail() || !sdata.good()) {
            egsWarning("%s error while reading distribution type and "
                       "number of bins in distribution file %s\n",
                       distribution_msg1,distribution_file.c_str());

            return 0;
        }
        if (nbin < 2) {
            egsWarning("%s nbin in a distribution must be at least 2\n"
                       "  you have %d in the distribution file %s\n",
                       distribution_msg1,nbin,distribution_file.c_str());

            return 0;
        }
        if (mode < 0 || mode > 3) {
            egsWarning("%s unknown distribution type %d in distribution file"
                       " %s\n",distribution_msg1,mode,distribution_file.c_str());
            return 0;
        }
        EGS_Float *val_array, *prob_array;
        int ibin;
        prob_array = new EGS_Float [nbin];
        if (mode == 0 || mode == 1) {
            val_array = new EGS_Float [nbin+1];
            val_array[0] = dum;
            ibin=0;
        }
        else {
            val_array = new EGS_Float [nbin];
            ibin=0;
        }
        for (int j=0; j<nbin; j++) {
            sdata >> val_array[j] >> skipsepp >> prob_array[j];
            EGS_Float value_temp = val_array[j];
            EGS_Float prob_temp = prob_array[j];
            if (print) {
                std::cout << value_temp << " : " << prob_temp << std::endl;
            }

            if (sdata.eof() || sdata.fail() || !sdata.good()) {
                egsWarning("%s error on line %d in distribution file %s\n",
                           distribution_msg1,j+2,distribution_file.c_str());

                delete [] val_array;
                delete [] prob_array;
                return 0;
            }
            if (mode != 2 && ibin > 1) {
                if (val_array[ibin-1] <= val_array[ibin-2]) {
                    egsWarning("%s values must be in increasing "
                               "order.\n   This is not the case for input on "
                               "lines %d,%d in distribution file %s\n",
                               distribution_msg1,j+2,j+1,distribution_file.c_str());
                    return 0;
                }
            }
        }
        int itype = 1;
        int nb = itype == 1 ? nbin+1 : nbin;
        int con_len = 21;
        if (smooth) {
            EGS_Float *smoothed_array = smooth_array(prob_array, nb, con_len);
            table = new EGS_AliasTable(nb,val_array,smoothed_array,itype);
            delete [] smoothed_array;
        }
        else {
            table = new EGS_AliasTable(nb,val_array,prob_array,itype);
        }
        delete [] val_array;
        delete [] prob_array;


    }
    return table;
};
