/**
* Implementation of the parameters class
*/
#include "parameters_helper.h"

/**
 * @fn void Parameters::Read(int argc, char** input)
 * @param argc Number of char*'s in the array
 * @param input Array of char* (the parameters)
 * 
 * Stores the parameters in a local variable in a < flag, value > mapping. Value is an array and may include more than
 * 	1 'value'
 */
void parameters_helper::Read(int argc, char **input) {
    vector <string> fields, subfields;
    vector<string>::iterator p, q;
    string argIn = "";

    for (sui t = 1; t < argc; t++) {
        fields.clear();
        char *pt = strtok(input[t], "+");
        while (pt != NULL) {
            fields.push_back(pt);
            pt = strtok(NULL, "+");
        }

        string allArgs = "";

        if (fields.size() > 1) {

            size_t pos;
            while ((pos = fields[0].find("-")) != string::npos)
                fields[0].erase(pos, 1);

            while ((pos = fields[0].find("=")) != string::npos)
                fields[0].erase(pos, 1);

            allArgs += fields[fields.size() - 1];

            while (true) {
                if (t + 1 >= argc) {
                    cerr << "Something wrong with args input" << allArgs << endl;
                    exit(3);
                }

                string next = input[t + 1];

                if (next.at(next.size() - 1) == '+') {
                    allArgs += " " + next.substr(0, next.size() - 1);
                    vector <string> temp;
                    temp.push_back(allArgs);
                    lookup.insert(make_pair(fields[0], temp));
                    t++;
                    break;
                } else {
                    allArgs += " " + next;
                    t++;
                }
            }

        } else {
            argIn += (string)(input[t]) + " ";
        }
    }

    /*Split on "-" to find flags*/
    char *pt = strtok((char *) argIn.c_str(), "-");
    fields.clear();
    while (pt != NULL) {
        fields.push_back(pt);
        pt = strtok(NULL, "-");
    }

    /*Get rid of empties*/
    for (p = fields.begin(); p != fields.end(); p++) {
        while (trim(*p).compare("") == 0 && p != fields.end()) {
            q = p;
            fields.erase(q);
        }

        if (p == fields.end())
            break;

        *p = trim(*p);
    }


    for (sui i = 0; i < fields.size(); i++) {
        /*Split flag from value*/
        char *p2 = strtok((char *) (fields[i].c_str()), "= ");
        subfields.clear();
        while (p2 != NULL) {
            subfields.push_back(p2);
            p2 = strtok(NULL, "= ");
        }

        for (p = subfields.begin(); p != subfields.end(); p++) {
            while (trim(*p).compare("") == 0 && p != subfields.end()) {
                q = p;
                subfields.erase(q);
            }

            if (p == subfields.end())
                break;

            *p = trim(*p);
        }

        vector <string> temporary;
        for (sui k = 1; k < subfields.size(); k++)
            temporary.push_back(subfields[k]);

        lookup.insert(make_pair(subfields[0], temporary));
    }
}
