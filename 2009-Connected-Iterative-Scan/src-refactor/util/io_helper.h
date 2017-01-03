#ifndef ICE_IOX
#define ICE_IOX

#include "string_helper.h"
#include <string>
#include <iostream>
#include <vector>
#include <fstream>

bool fline_tr(ifstream* fin, vector<string>* fields, string delim);
bool openFile(ifstream* fin, string filename);
void openFileHarsh(ifstream* fin, string filename);
void split_tr(string input, vector<string>* fields, string delim);
void FileError(const string& filename);
#endif
