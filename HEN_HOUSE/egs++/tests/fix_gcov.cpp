/* --------------------------------------------------------------------------------------------------------- 120 chars -

This file contains the source code for the `fix_coverage_report` tool,
which attempts to fix the inadequacies of the `gcov` tool for testing coverage reports.

When a function is not used in any test, the compiler saves time by
not instrumenting the function at all. The gcov tool then ignores these lines
in its count of total lines, which serverely overestimates the lie coverage pecentage.

This tool therefore takes the report from gcov (e.g. egs_vector.h.gcov),
identifies lines like

        -:  233:    EGS_Vector getScaled(const EGS_Vector &s) const {
        -:  234:        return EGS_Vector(x*s.x, y*s.y, z*s.z);
        -:  235:    }

changes the dash to a zero, and recalculates the coverage percentage.

*/

// built-in c++ libraries
//
#include <fstream>    // ifstream
#include <sstream>    // stringstream
#include <string>     // erase (c++20)
#include <vector>
#include <tuple>
#include <iostream>

using std::cout, std::endl, std::string, std::vector, std::tuple, std::istream, std::ifstream, std::ofstream;


/*! \brief
  Convenience function that works using str.erase
*/
string &remove_substring(string &s, const string &pattern) { // NOLINT
    size_t pos;
    while ((pos = s.find(pattern)) != std::string::npos) {
        s.erase(pos, pattern.length());  // erase is c++20, see https://stackoverflow.com/a/69550517
    }
    return s;
}

/*! \brief
  Checks if a string contains a given pattern
*/
bool contains(string s, const string &pattern) {
    return s.find(pattern) != std::string::npos;
}

/*! \brief
  Partitions a string into a list of strings.
*/
static vector<string> split(
    string str,
    char delim = ',',
    char extra1 = '\0',
    char extra2 = '\0') {

    std::string replacement = std::string(1, delim);

    // turns all extra1 delimeters into delim
    std::string target = std::string(1, extra1);
    if (extra1 != '\0') {
        while (str.find(target) != std::string::npos) {
            str.replace(str.find(target), target.size(), replacement);
        }
    }

    // turns all extra2 delimeters into delim
    target = std::string(1, extra2);
    if (extra2 != '\0') {
        while (str.find(target) != std::string::npos) {
            str.replace(str.find(target), target.size(), replacement);
        }
    }

    // merges adjacent instances of delim
    target = replacement + replacement;
    while (str.find(extra2) != std::string::npos) {
        str.replace(str.find(target), target.size(), replacement);
    }

    std::stringstream ss(str);
    std::vector<std::string> substrings;
    std::string tmp;

    // do the split
    while (getline(ss, tmp, delim)) {
        if (tmp.size() > 0) {
            substrings.push_back(tmp);
        }
    }
    return substrings;
}

/*! \brief
  Allows cross-platform compatibility of files.
*/
istream &getline_osx_linux(istream &file, string &line) {  // NOLINT
    istream &status = getline(file, line);
    if (!status) {
        return status;
    }
    remove_substring(line, "\r");
    remove_substring(line, "\n");
    return status;
}

string safe_html(const string &content) {

    string safe;

    for (const char &c : content) {
        if (c == '<') {
            safe += "&lt";
        }
        else if (c == '>') {
            safe += "&gt";
        }
        else if (c == '\t') {
            safe += "&nbsp";
        }
        else {
            safe += c;
        }
    }

    // garbage strings from gcov
    remove_substring(safe, "[m[K");
    remove_substring(safe, "[41m[K");
    remove_substring(safe, "[42m[K");
    remove_substring(safe, "[43m[K");

    return safe;
}


/*! \brief
  Return true if the line looks like it's declaring a function
*/
bool pattern_match_function(string line) {

    // get the 3 pieces of a gcov line (count, line number, expression)
    auto pieces = split(line, ':');

    // blank expression
    if (pieces.size() < 3) {
        return false;
    }

    // merge pieces into one expression and from there expect white-space separated tokens
    string expr_str;
    for (size_t ipiece=2; ipiece < pieces.size(); ipiece++) {
        expr_str += pieces[ipiece];
    }
    vector<string> expr = split(expr_str, ' ', '\t');

    // ignore comments
    for (int itoken; itoken < expr.size(); itoken++) {
        if (contains(expr[itoken], "//")) {
            expr = vector<string>(expr.begin(), expr.begin() + itoken);
            break;
        }
        if (contains(expr[itoken], "/*")) {
            expr = vector<string>(expr.begin(), expr.begin() + itoken);
            break;
        }
    }

    // loop over tokens to see if an isolate equals sign comes before an open paren
    for (const string &token : expr) {
        if (token == "=") {
            // the first statement is an assignment, so this is not a function
            return false;
        }
        // first token opens a paren and therefore must be a function
        // if this misses a function, that's on you for bad coding style
        if (contains(token, "(")) {
            return true;
        }
    }

    // possible empty line or comment
    return false;
}

/*! \brief
  Return true if the line looks like it's starting a comment
*/
bool pattern_match_comment(string line) {

    // get the 3 pieces of a gcov line (count, line number, expression)
    auto pieces = split(line, ':');

    // blank expression
    if (pieces.size() < 3) {
        return false;
    }

    // merge pieces into one expression
    string expr_str;
    for (size_t ipiece=2; ipiece < pieces.size(); ipiece++) {
        expr_str += pieces[ipiece];
    }

    // identify comments
    vector<string> expr = split(expr_str, ' ', '\t');
    if (contains(expr[0], "//")) {
        return true;
    }
    if (contains(expr[0], "/*")) {
        return true;
    }

    // line probably blank
    return false;
}

/*! \brief
  Return true if the line, that we already know to be a function, has not been marked with a coverage count
*/
bool unmarked_pattern(const string &line) {

    // get the 3 pieces of a gcov line
    auto pieces = split(line, ':');

    // check if the first piece has a dash. If not, it's a numeral and is a count of line hits
    char last_char = pieces[0][pieces[0].size()-1];
    if (last_char == '-') {
        return true;
    }
    return false;
}


/*! \brief
  Starting with an ifstream `ifile` that has just read `line`, the contents of which
  mark a function that has not been covered, fix the contents of subsequent lines
  so that they properly depict the fact that the function wasn't ever used
*/
int fix_lines_of_function(ifstream *ifile, ofstream *ofile, string line) {

    string fixed;  // the return string
    int n_missed = 0; // how many lines were missed

    // find the end of the function signature
    int paren_count = 0;
    for (int ichar=0; ichar < line.size(); ichar++) {
        if (line[ichar] == '(') {
            paren_count++;
        }
        if (line[ichar] == ')') {
            paren_count--;
        }
    }
    while (paren_count > 0) {
        fixed += "<pre>" + safe_html(line) + "</pre>\n";
        getline_osx_linux(*ifile, line);
        for (int ichar=0; ichar < line.size(); ichar++) {
            if (line[ichar] == '(') {
                paren_count++;
            }
            if (line[ichar] == ')') {
                paren_count--;
            }
        }
    }

    // need to keep track of the brace count to identify when the function is finished
    int brace_count = 0;
    for (int ichar=0; ichar < line.size(); ichar++) {
        if (line[ichar] == '{') {
            brace_count++;
        }
        if (line[ichar] == '}') {
            brace_count--;
        }
    }

    // case when we have a single-line function definition
    if (brace_count == 0) {
        // get the 2+n pieces of a gcov line
        auto pieces = split(line, ':');
        pieces[0][pieces[0].size()-1] = '0';
        fixed += "<pre style=\"background-color:#BB0000;\">";
        for (size_t ipiece=0; ipiece < pieces.size(); ipiece++) {
            if (ipiece < pieces.size()-1) {
                fixed +=  safe_html(pieces[ipiece]) + ":";
            }
            else {
                fixed += safe_html(pieces[ipiece]) + "</pre>\n";
            }
        }
        *ofile << fixed;
        return 1;
    }

    // function is done when brace_count is zero
    while (brace_count > 0) {

        // get the 2+n pieces of a gcov line
        auto pieces = split(line, ':');
        pieces[0][pieces[0].size()-1] = '0';
        fixed += "<pre style=\"background-color:#BB0000;\">";
        n_missed += 1;
        for (size_t ipiece=0; ipiece < pieces.size(); ipiece++) {
            if (ipiece < pieces.size()-1) {
                fixed +=  safe_html(pieces[ipiece]) + ":";
            }
            else {
                fixed +=  safe_html(pieces[ipiece]) + "</pre>\n";
            }
        }

        // not finished with the function, so keep going
        getline_osx_linux(*ifile, line);
        for (int ichar=0; ichar < line.size(); ichar++) {
            if (line[ichar] == '{') {
                brace_count++;
            }
            if (line[ichar] == '}') {
                brace_count--;
            }
        }
    }

    fixed += "<pre>" + safe_html(line) + "</pre>\n";
    *ofile << fixed;
    return n_missed;

}


/*! \brief
  Starting with an ifstream `ifile` that has just read `line`, the contents of which
  have been marked as covered, simply convert the lines to html, and remove the call count
  on the first line (unless the function is a one-liner)
*/
tuple<int, int> consume_lines_of_function(std::ifstream *ifile, std::ofstream *ofile, std::string line) {

    std::string fixed;  // the return string
    int n_covered = 0, n_missed = 0; // how many lines were covered/missed

    // find the end of the function signature
    int paren_count = 0;
    for (int ichar=0; ichar < line.size(); ichar++) {
        if (line[ichar] == '(') {
            paren_count++;
        }
        if (line[ichar] == ')') {
            paren_count--;
        }
    }
    while (paren_count > 0) {
        fixed += "<pre>" +  safe_html(line) + "</pre>\n";
        getline_osx_linux(*ifile, line);
        for (int ichar=0; ichar < line.size(); ichar++) {
            if (line[ichar] == '(') {
                paren_count++;
            }
            if (line[ichar] == ')') {
                paren_count--;
            }
        }
    }

    // need to keep track of the brace count to identify when the function is finished
    int brace_count = 0;
    for (int ichar = 0; ichar < line.size(); ichar++) {
        if (line[ichar] == '{') {
            brace_count++;
        }
        if (line[ichar] == '}') {
            brace_count--;
        }
    }

    // case when we have a single-line function definition
    if (brace_count == 0) {
        // get the 2+n pieces of a gcov line
        auto pieces = split(line, ':');
        fixed += "<pre style=\"background-color:#00BB00;\">";
        for (size_t ipiece=0; ipiece < pieces.size(); ipiece++) {
            if (ipiece < pieces.size()-1) {
                fixed += safe_html(pieces[ipiece]) + ":";
            }
            else {
                fixed += safe_html(pieces[ipiece]) + "</pre>\n";
            }
        }
        *ofile << fixed;
        return {1, 0};
    }

    while (brace_count > 0) {

        // get the 2+n pieces of a gcov line
        auto pieces = split(line, ':');
        char last_char = pieces[0][pieces[0].size()-1];
        if (last_char >= '0' && last_char <= '9') {
            fixed += "<pre style=\"background-color:#00BB00;\">";
            n_covered += 1;
        }
        else if (last_char == '#') {
            fixed += "<pre style=\"background-color:#BB0000;\">";
            n_missed += 1;
        }
        else {
            fixed += "<pre>";
        }
        for (size_t ipiece=0; ipiece < pieces.size(); ipiece++) {
            if (ipiece < pieces.size()-1) {
                fixed +=  safe_html(pieces[ipiece]) + ":";
            }
            else {
                fixed +=  safe_html(pieces[ipiece]) + "</pre>\n";
            }
        }

        // not finished with the function, so keep going
        getline_osx_linux(*ifile, line);
        for (int ichar=0; ichar < line.size(); ichar++) {
            if (line[ichar] == '{') {
                brace_count++;
            }
            if (line[ichar] == '}') {
                brace_count--;
            }
        }
    }

    fixed += "<pre>" +  safe_html(line) + "</pre>\n";
    *ofile << fixed;
    return {n_covered, n_missed};

}


/*! \brief
  Starting with an ifstream `ifile` that has just read `line`, the contents of which
  have been identified as a comment, simply convert the lines to html
*/
void consume_lines_of_comment(std::ifstream *ifile, std::ofstream *ofile, std::string line) {

    std::string fixed;  // the converted contents

    // get the 3 pieces of a gcov line (count, line number, expression)
    auto pieces = split(line, ':');

    // blank expression
    if (pieces.size() < 3) {
        return;
    }

    // merge end pieces into one expression
    string expr_str;
    for (size_t ipiece=2; ipiece < pieces.size(); ipiece++) {
        expr_str += pieces[ipiece];
    }

    // handle one-line comments
    vector<string> expr = split(expr_str, ' ', '\t');
    if (contains(expr[0], "//")) {
        *ofile << line << endl;
    }

    // from here we can assume a multi-line comment

    // find the end of the comment, consuming along the way
    int delim_count = 0;
    for (int ichar=0; ichar < line.size()-1; ichar++) {
        if (line[ichar] == '/' && line[ichar+1] == '*') {
            delim_count++;
        }
        if (line[ichar] == '*' && line[ichar+1] == '/') {
            delim_count--;
        }
    }
    while (delim_count > 0) {
        fixed += "<pre>" +  safe_html(line) + "</pre>\n";
        getline_osx_linux(*ifile, line);
        for (int ichar=0; ichar < line.size()-1; ichar++) {
            if (line[ichar] == '/' && line[ichar+1] == '*') {
                delim_count++;
            }
            if (line[ichar] == '*' && line[ichar+1] == '/') {
                delim_count--;
            }
        }
    }

    fixed += "<pre>" +  safe_html(line) + "</pre>\n";
    *ofile << fixed;

}

/*! \brief
  Return true if the line looks like it's declaring a function but has not had any line counts associated with it
*/
std::string overall_colour(int n_cov, int n_miss) {

    float percentage = n_cov / (1e-5 + n_cov + n_miss);
    int tens = static_cast<int>(percentage*10);
    int ones = static_cast<int>((percentage - 0.1*tens)*100);
    int tenths = static_cast<int>((percentage - 0.1*tens- 0.01*ones)*1'000);
    int hundredths = static_cast<int>((percentage - 0.1*tens - 0.01*ones- 0.001*tenths)*10'000);

    std::string col_str = "<pre style=\"background-color:#";
    if (percentage < 0.5) {
        col_str += "AA0000";
    }
    else if (percentage < 0.6) {
        col_str += "AA3E00";
    }
    else if (percentage < 0.7) {
        col_str += "AA6000";
    }
    else if (percentage < 0.8) {
        col_str += "8AAA00";
    }
    else if (percentage < 0.9) {
        col_str += "4CAA00";
    }
    else {
        col_str += "00AA00";
    }
    col_str += ";\"> Overall percentage "
               + std::to_string(tens) + std::to_string(ones) + "."
               + std::to_string(tenths) + std::to_string(hundredths) + "%"
               + "</pre>\n";

    return col_str;
}

/*! \brief
  Goes through the provided file line-by-line to add html and fix the counts of unistrumented functions
*/
void fix_file(const char *filename) {

    int n_covered = 0, n_missed = 0;

    std::string ifname = std::string(filename);
    std::ifstream ifile(ifname);
    if (!ifile.is_open()) {
        std::cout << "Could not open " << filename << std::endl;
        return;
    }

    std::string ofilename = std::string(ifname).substr(0, ifname.size()-4) + "html";
    std::ofstream ofile(ofilename);
    if (!ofile.is_open()) {
        std::cout << "Could not open " << ofilename << std::endl;
        return;
    }
    ofile << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";
    ofile << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";

    std::string line;
    getline_osx_linux(ifile, line);  // skip the first line

    // reformat line-by-line
    while (getline_osx_linux(ifile, line)) {

        if (pattern_match_function(line)) {
            // do function logic
            if (unmarked_pattern(line)) {
                n_missed += fix_lines_of_function(&ifile, &ofile, line);
            }
            else {
                tuple<int, int> cov_miss = consume_lines_of_function(&ifile, &ofile, line);
                n_covered += std::get<0>(cov_miss);
                n_missed += std::get<1>(cov_miss);
            }
        }
        else if (pattern_match_comment(line)) {
            // do comment logic
            consume_lines_of_comment(&ifile, &ofile, line);
        }
        else {
            // probably a blank line
            ofile << "<pre>" + safe_html(line) + "</pre>\n";
        }
    }

    // final formatting
    ofile << "\n</body></html>\n";
    ofile.seekp(0);
    ofile << "<!DOCTYPE html>\n<html>\n<style>pre {margin: 0;}\n</style>\n<body>\n";
    ofile << overall_colour(n_covered, n_missed);

    ifile.close();
    ofile.close();

    cout << filename << ": " << n_covered << " / " << n_covered + n_missed
         << " = " << 100. * n_covered / (n_covered+n_missed) << "%\n";


}

/*! \brief
  Takes in a list of files and outputs a new html file for each
*/
int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cout << "\nUsage: ./fix_coverage_report class1.h.gov class2.h.gov class2.cpp.gov" << std::endl;
        return 1;
    }

    for (int iname=1; iname < argc; iname++) {
        fix_file(argv[iname]);
    }

    return 0;
}
