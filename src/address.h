#ifndef favor_address_include
#define favor_address_include

#include "favor.h"

namespace favor {
    class Address {
    public:
        const string addr;
        const int count;
        const long contactId;
        const MessageType type;

        Address(string addr, long c, int contact, MessageType t) : addr(addr), count(c), contactId(contact), type(t){}

        bool belongsToContact(){
            return contactId > 1;
        }
    };
}

#endif