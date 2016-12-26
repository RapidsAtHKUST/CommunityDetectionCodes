#ifndef ICE_STR
#define ICE_STR

#include <string>
#include <cstring>
#include <sstream>

using namespace std;

const string trim(const string& pString,
                       const string& pWhitespace = " \n\t");
 
string replaceStrChar(string str, const string& replace, char ch);
void replaceAll(string& str, char& x, char& y);

/**
 * @fn template <typename T> T str_to(string s)
 * @param s String to convert
 * 
 * Converts a string to some other type T
 */
template <typename T>
T str_to(string s){
  stringstream ss(s);
  
  T res = T();
  ss >> res;
  
  return res;
}

template <typename T>
pair < T, bool > check_str_to(string s){
  stringstream ss(s);
  
  T res = T();
  ss >> res;
  
  return pair < T, bool > (res, !ss.fail());
}

template <typename T>
string to_str(T element){
  stringstream ss;

  ss << element;

  return ss.str();
}
         
#endif
