#ifndef favor_message_include
#define favor_message_include

#include "favor.h"

namespace favor {
  class message {
  public:
    
    const MessageType type;
    const bool sent;
    const long id;
    const std::time_t date;
    const string address;
    const bool media;
    const string body;
    
    message(MessageType t, bool s, long i, std::time_t d, string a, bool m, string b);
    string prettyDate(); 
    
  };
}

#endif