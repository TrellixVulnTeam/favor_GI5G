#ifndef favor_exception_include
#define favor_exception_include
#include <exception>

namespace favor{
  class sqliteException : public std::exception{
    virtual const char* what() const throw()
      {
	return "An internal SQLite3 operation has failed unexpectedly";
      }
  };
  class networkException : public std::exception{
    virtual const char* what() const throw()
      {
	return "Unable to perform normal network operations";
      }
  };
}
#endif