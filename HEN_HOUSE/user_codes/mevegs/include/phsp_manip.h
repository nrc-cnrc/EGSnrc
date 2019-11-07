//////////////////////////////////////////////////////////////////////////////
// MevEGS - (C) 2018 Mevex Corp.
//
// obtain phase space file data
//
// MXO
//
// because we changed the way geometry input works for tet collections,
// we can't specify phase space scoring in input files the usual way
// instead, we trick the phase space ausgab object initialization function
// by making a second input file formatted how EGS is expecting
//////////////////////////////////////////////////////////////////////////////

#ifndef PHSP_MANIP
#define PHSP_MANIP

#include "../../egs++/egs_input.h" // EGS_Input class, we use setContentFromString

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
namespace phsp_manip{

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //public data to define options at application level
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // GEOMETRY
  enum class geometry_option{ Xplane, Yplane, Zplane };
  const std::map<geometry_option, std::string> geometryName {{geometry_option::Xplane, "EGS_Xplanes"},
                                                             {geometry_option::Yplane, "EGS_Yplanes"},
                                                             {geometry_option::Zplane, "EGS_Zplanes"}};
  const std::vector<std::string> geo_opt_strs{ geometryName.at(geometry_option::Xplane),
                                               geometryName.at(geometry_option::Yplane),
                                               geometryName.at(geometry_option::Zplane) };

  //////////////////////////////////////////////////////////////////////////////
  // PHSP FORMAT
  const std::vector<std::string> phsp_format_opts { "IAEA" , "EGSnrc" };
  // singleton object name used at application level to retrieve the phase space object after creation
  static constexpr auto phsp_ausgab_obj_name = "phase_space_ausgab_obj";
  const std::map<std::string, std::string> constant_key {{"EGS_Xplanes", "constant X"},
                                                         {"EGS_Yplanes", "constant Y"},
                                                         {"EGS_Zplanes", "constant Z"}};

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //anon namespace to hide the inner fns
  namespace {
    std::string make_geo_string(std::string scoring_name, std::pair<std::string, double> pos){
      //the output sstream
      std::ostringstream geo_spec;
      auto geo_opt = pos.first;
      auto geo_pos = pos.second;

      geo_spec << ":start geometry definition:" << "\n"
               << ":start geometry:" << "\n"
               << "     library = egs_planes" << "\n"
               // get geo name matched to enum of geometry option
               << "     type = " << geo_opt << "\n"
               // get position of the plane
               << "     positions = " << geo_pos << "\n"
               // set the name to match that of the ausgab section
               << "     name = " << scoring_name << "\n"
               << ":stop geometry:" << "\n"
               << ":stop geometry definition:" << "\n";

      return geo_spec.str();
    }

    ////////////////////////////////////////////////////////////////////////////
    // make the ausgab input file item as expected by the egsinp parser
    std::string make_ausgab_string(std::string scoring_name,
                                   std::string output_type,
                                   std::pair<std::string, double> pos){
      auto geo_opt = pos.first;
      auto geo_pos = pos.second;

      std::ostringstream ausgab_spec;
      ausgab_spec << ":start ausgab object definition:" << "\n"
                  << "    :start ausgab object:" << "\n"
                  << "        library = egs_phsp_scoring" << "\n"
                  << "        name = " << phsp_ausgab_obj_name << "\n"
                  << "        phase space geometry = " << scoring_name << "\n"
                  << "        output format = " + output_type << "\n";

                  // add constant parameter for IAEA format stuff
                  // if (output_type == "IAEA"){
                    ausgab_spec << "        ";
                    ausgab_spec << constant_key.at(geo_opt) << " = " << geo_pos << std::endl;
                  // }

     ausgab_spec  << "    :stop ausgab object:" << "\n"
                  << ":stop ausgab object definition:" << "\n";

      return ausgab_spec.str();
    }

    ////////////////////////////////////////////////////////////////////////////
    //create a string that matches the egsinp format
    std::string make_string_for_egsinp(std::string output_type,    // either EGSnrc or IAEA
                                       std::pair<std::string, double> pos){

      const std::string scoring_name = "scoreplane";
      return make_geo_string(scoring_name, pos)
             + make_ausgab_string(scoring_name, output_type, pos);
    }


  } // anon namespace

  //////////////////////////////////////////////////////////////////////////////
  ////DEBUG
  // void test(){
  //   auto opt = "EGS_Xplanes";
  //   auto pos = 0.0;
  //   std::cout << make_string_for_egsinp("IAEA", std::make_pair(opt, pos)) << std::endl;
  // }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //only public fn of interface, called to make a dummy egsinp file only used for phase space output
  EGS_Input get_egsinp(std::string output_type,    // either EGSnrc or IAEA
                       std::pair<std::string, double> pos){

    EGS_Input res;
    auto phsp_egsinp = make_string_for_egsinp(output_type, pos);
    res.setContentFromString(phsp_egsinp);
    res.print(0, std::cout);
    return res;
  }

} // phsp_manip ns

#endif
