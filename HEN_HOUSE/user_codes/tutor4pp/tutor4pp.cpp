/*
###############################################################################
#
#  EGSnrc egs++ tutor4pp application
#  Copyright (C) 2024 National Research Council Canada
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
#  Authors:         Frederic Tessier, 2024
#
#  Contributors:
#
###############################################################################
#
#  This application prints detailed history, stack and particle information
#  for tutorial purposes. It was inspired by the "monitor a section in detail"
#  part of an EGSnrc course laboratory that used to rely on tutor4 with the
#  IWATCH option turned on.
#
###############################################################################
*/


#include "egs_advanced_application.h"
#include "egs_interface2.h"

class APP_EXPORT tutor4_Application : public EGS_AdvancedApplication {
    public:
        tutor4_Application(int argc, char **argv) :
            EGS_AdvancedApplication(argc,argv), save_case(-1) {}
        int ausgab(int iarg);
        int initScoring();
        int save_case;
};

// init scoring
int tutor4_Application::initScoring () {
    for (int call=AfterTransport; call<UnknownCall; ++call) setAusgabCall((AusgabCall)call, true);
    return 0;
}

// ausgab
int tutor4_Application::ausgab (int iarg) {

    // index of the current particle and region
    int     np = the_stack->np - 1;
    int     ir = the_stack->ir[np]-2;
    int     iq = the_stack->iq[np];
    double  E  = the_stack->E[np];
    double  x  = the_stack->x[np];
    double  y  = the_stack->y[np];
    double  z  = the_stack->z[np];
    double  u  = the_stack->u[np];
    double  v  = the_stack->v[np];
    double  w  = the_stack->w[np];
    double  wt = the_stack->wt[np];
    int     lt = the_stack->latch[np];
    int     npold = the_stack->npold - 1;

    if (current_case != save_case) {
        egsInformation("\n\nHISTORY #%d\n", current_case);
        egsInformation("============================================================================================================================================\n");
        egsInformation("%5s  %-40s %3s%8s%8s%8s%8s%8s%8s%8s%8s%8s%8s%8s\n", "iarg", "event", "NP", "charge", "energy", "region", "x", "y", "z", "u", "v", "w", "weight", "latch");
        egsInformation("============================================================================================================================================\n");
        egsInformation("\n%5s  %-40s ", "", "Incident particle");
        egsInformation("%3d%8d%8.3f%8d%8.3f%8.3f%8.3f%8.3f%8.3f%8.3f%8.3f%8x\n\n", np+1, iq, E, ir, x, y, z, u, v, w, wt, lt);
        save_case = current_case;
    }

    // Echo selected interactions to terminal
    int echo = true;
    string str;
    switch (iarg) {
        case    BeforeTransport:    echo = false;                                   break;
        case    EgsCut:             str = "Energy below Ecut or Pcut";              break;
        case    PegsCut:            str = "Energy below AE or AP";                  break;
        case    UserDiscard:        str = "User discard";                           break;
        case    ExtraEnergy:        str = "Extra Energy deposited";                 break;
        case    AfterTransport:     echo = false;                                   break;
        case    BeforeBrems:        str = "Bremsstrahlung about to occur";          break;
        case    AfterBrems:         echo = false;                                   break;
        case    BeforeMoller:       str = "Moller scattering about to occur";       break;
        case    AfterMoller:        echo = false;                                   break;
        case    BeforeBhabha:       str = "Bhabha scattering about to occur";       break;
        case    AfterBhabha:        echo = false;                                   break;
        case    BeforeAnnihFlight:  str = "Annihilation in fligth about to occur";  break;
        case    AfterAnnihFlight:   echo = false;                                   break;
        case    BeforeAnnihRest:    str = "Annihilation at rest about to occur";    break;
        case    AfterAnnihRest:     echo = false;                                   break;
        case    BeforePair:         str = "Pair production about to occur";         break;
        case    AfterPair:          echo = false;                                   break;
        case    BeforeCompton:      str = "Compton scattering about to occur";      break;
        case    AfterCompton:       echo = false;                                   break;
        case    BeforePhoto:        str = "Photoelectric effect about to occur";    break;
        case    AfterPhoto:         echo = false;                                   break;
        case    BeforeRayleigh:     str = "Rayleigh scattering about to occur";     break;
        case    AfterRayleigh:      echo = false;                                   break;
        case    FluorescentEvent:   str = "Fluorescence event just occured";        break;
        case    CosterKronigEvent:  str = "Coster-Kronig event just occured";       break;
        case    AugerEvent:         str = "Auger event just occured";               break;
        case    BeforePhotoNuc:     str = "Photonuclear event about to occur";      break;
        case    AfterPhotoNuc:      echo = false;                                   break;
        default:                    echo = false;
    }
    if (echo) {
        egsInformation("--------------------------------------------------------------------------------------------------------------------------------------------\n");
        egsInformation("%5d  %-40s ", iarg, str.c_str());
        egsInformation("%3d%8d%8.3f%8d%8.3f%8.3f%8.3f%8.3f%8.3f%8.3f%8.3f%8x\n", np+1, iq, E, ir, x, y, z, u, v, w, wt, lt);
    }

    // print particle stack after selected interactions
    switch (iarg) {
        case    AfterBrems:
        case    AfterMoller:
        case    AfterBhabha:
        case    AfterAnnihFlight:
        case    AfterAnnihRest:
        case    AfterPair:
        case    AfterCompton:
        case    AfterPhoto:
        case    AfterRayleigh:
        case    FluorescentEvent:
        case    CosterKronigEvent:
        case    AugerEvent:
        case    AfterPhotoNuc:
                    egsInformation("%5d  %-40s \n", iarg, "STACK after interaction:");
                    for (int i=0; i<=np; i++) {
                        ir = the_stack->ir[i]-2;
                        iq = the_stack->iq[i];
                        E  = the_stack->E[i];
                        x  = the_stack->x[i];
                        y  = the_stack->y[i];
                        z  = the_stack->z[i];
                        u  = the_stack->u[i];
                        v  = the_stack->v[i];
                        w  = the_stack->w[i];
                        wt = the_stack->wt[i];
                        lt = the_stack->latch[i];
                        egsInformation("%5s  %13d", ".", i+1);
                        string str = "";
                        if (i == npold) str += " <- NPold";
                        if (i == np)    str += " <- NP";
                        egsInformation("%-28s", str.c_str());
                        egsInformation("%3s%8d%8.3f%8d%8.3f%8.3f%8.3f%8.3f%8.3f%8.3f%8.3f%8x\n", "", iq, E, ir, x, y, z, u, v, w, wt, lt);
                    }
                    break;
        default:    break;
    }

    return 0;
}

APP_MAIN (tutor4_Application);
