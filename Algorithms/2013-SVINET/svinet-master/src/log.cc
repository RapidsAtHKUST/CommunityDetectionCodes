#include "log.hh"

string Logger::_log_fname = "mmsb.log";
string Logger::_prefix = "";
FILE *Logger::_logfd = 0;
bool Logger::_initialized = false;
Logger::Level Logger::_level = DEBUG;

int
Logger::initialize(string prefix, string name, bool force,
		   Level level)
{
  if (!_initialized) {
    _prefix = prefix;
    _log_fname = name;
    _level = level;
    if (setup_logfd(force) < 0)
      return -1;
    
    _initialized = true;
    info("Setting log to %s", name.c_str());
  }
  return 0;
}

int
Logger::xlog(Level level, const char *format, ...)
{
  if (!_initialized) {
    initialize(_prefix, string("/mmsb.log"), false);
  }
  if (level < _level)
    return 0;

  assert(_logfd);
  va_list ap;
  va_start (ap, format);
  string ts = get_time();
  string l = get_level_str(level);

  // send errors to terminal and the log
  if (level == Logger::ERROR)  {
    fprintf(stderr, "%s ", l.c_str());
    fflush(stdout);
  }

  fprintf(_logfd, "[%s] [%d] [%3s] ", ts.c_str(), getpid(), l.c_str());
  vfprintf(_logfd, format, ap);
  fprintf(_logfd, "\n\n");
  fflush(_logfd);
  va_end(ap);
  return 0;
}

string
Logger::get_time()
{
  time_t now = time(0);
  struct tm p;
  localtime_r(&now, &p);
  char buf[512];
  strftime(buf, 512, "%b %e %T", &p);
  return string(buf);
}

string
Logger::get_level_str(Level level)
{
  string s;
  switch (level) {
  case INFO:  s =  "INF"; break;
  case DEBUG: s =  "DBG"; break;
  case WARN:  s =  "WRN"; break;
  case ERROR: s =  "ERR"; break;
  case FATAL: s =  "FTL"; break;
  case TEST: s =  "TST"; break;
  default:
    s = "unknown";
  }
  return s;
}

int
Logger::setup_logfd(bool force)
{
  if (setup_log_dir(force) < 0)
    return -1;
  
  if (_logfd) {
    fclose(_logfd);
    _logfd = 0;
  }
  string fn = log_fname();
  _logfd = fopen(fn.c_str(), "w");
  if (_logfd) {
    fprintf(stdout, "+ Writing log to %s\n", fn.c_str());
    fflush(stdout);
  }
  if (!_logfd) {
    _logfd = fopen("/dev/null", "w");
    fprintf(stderr, "+ writing log to /dev/null\n");
  }
  return 0;
}

int
Logger::setup_log_dir(bool force)
{
  struct stat dirstat;
  if (stat(dirname().c_str(), &dirstat) != 0) {
    if (errno == ENOENT) {
      mkdir(dirname().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      if (stat(dirname().c_str(), &dirstat) != 0) {
	fprintf(stderr, "Warning: could not create dir %s\n", 
		dirname().c_str());
	return -1;
      }
    } else {
      fprintf(stderr, "Warning: could not stat dir %s\n", dirname().c_str());
      return -1;
    }
  } else if (!force) {
    fprintf(stderr, "Error: dir %s already exists\n", dirname().c_str());
    return -1;
  }
  return 0;
}
