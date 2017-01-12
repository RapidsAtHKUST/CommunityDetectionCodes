#ifndef LOG_HH
#define LOG_HH

#include <stdarg.h>
#include <sys/file.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

#include <string>
using namespace std;

//#define DEBUG_MODE 1

#define PREFIX "log"

class Logger {
public:
  typedef enum { DEBUG = 0, TEST, INFO, WARN, ERROR, FATAL } Level;
  static int xlog(Level l, const char *format, ...);
  static int initialize(string prefix, string name, 
			bool force_overwrite_dir,
			Level level = DEBUG);
  static string log_fname() { return dirname() + string("/") + _log_fname; }
  static string dirname()   { return _prefix; }
  
private:
  static int setup_log_dir(bool force);
  static int setup_logfd(bool force);
  static string get_time();
  static string get_level_str(Level level);
  
  static FILE *_logfd;
  static string _log_fname;
  static bool _initialized;
  static string _prefix;
  static Level _level;
};


#ifndef DEBUG_MODE
#define debug(X, ...)
#define info(format, ...) 
#define tst(X, ...) 
#define lerr(format, ...) Logger::xlog(Logger::ERROR, format, ## __VA_ARGS__)
#else
#define tst(format, ...) Logger::xlog(Logger::TEST, format, ## __VA_ARGS__)
#define debug(format, ...) Logger::xlog(Logger::DEBUG, format, ## __VA_ARGS__)
#define info(format, ...) Logger::xlog(Logger::INFO, format, ## __VA_ARGS__)
#define lerr(format, ...) Logger::xlog(Logger::ERROR, format, ## __VA_ARGS__)
#endif

#endif
