
/**
 * @fn const std::string trim (const std::string& pString, const std::string& pWhitespace)
 * Remove beginning and trailing "whitespace" from the string
 *
 * @param pString String to trim
 * @param pWhitespace Characters to trim at the ends of the given strings.
 * @return string with whitespace characters stripped from beginning and end
 */
 #include "StringEx.h"
 
const std::string trim(const std::string& pString,
                       const std::string& pWhitespace)
{
  if ((pString).compare("") == 0)
    return pString;
  
    const size_t beginStr = pString.find_first_not_of(pWhitespace);
    if (pString[beginStr] == 0)
      return "";
    
    if (beginStr == std::string::npos)
    {
        // no content
        return "";
    }

    const size_t endStr = pString.find_last_not_of(pWhitespace);
    const size_t range = endStr - beginStr + 1;

    return pString.substr(beginStr, range);
}

void replaceAll(string& str, const char& x, const char& y){
  for (int i = 0; i < str.size(); i++){
    if ( str[i] == x )
      str[i] = y;
  }
}

/**
 * @fn replaceStrChar(string str, const string& replace, char ch)
 * 
 * @param str String to consider
 * @param replace The substring to remove
 * @param ch The character to replace found substrings with
 * 
 * @return The changed version of the string
 */
string replaceStrChar(string str, const string& replace, char ch)
{
  size_t found = str.find_first_of(replace);

  while (found != string::npos) {
    str[found] = ch; 
    found = str.find_first_of(replace, found+1); 
  }

  return str; 
}
