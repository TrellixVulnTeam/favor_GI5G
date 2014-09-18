#ifndef favor_exception_include
#define favor_exception_include
#include <exception>

namespace favor{
  class sqliteException : public std::exception{
    virtual const char* what() const throw()
      {
	return "An internal SQLite3 operation has failed";
      }
  };
  class networkException : public std::exception{
    virtual const char* what() const throw()
      {
	return "Unable to perform normal network operations";
      }
  };
  #ifdef FAVOR_EMAIL_MANAGER
  class emailException : public std::exception{
    virtual const char* what() const throw()
    {
      return "Error interfacing with the email server";
    }
  };
  #endif
}
#endif