#ifndef favor_address_include
#define favor_address_include

#include "favor.h"

namespace favor {
    class Address {
    public:
        const string addr;
        const int count;
        const int contactId;

        Address(string addr, int c, int contact) : addr(addr), count(c), contactId(contact){}


        //The comparison and inequality operators are defined differently because we need them for different things
        //TODO: at this point maybe it should just be a separate function, but addresses with a contactId >-1 should ALWAYS
        //be greater than those with -1 because it means they were explicitly chosen
        inline bool operator==(const Address & rhs){return addr == rhs.addr; }
        inline bool operator!=(const Address & rhs){return !operator==(rhs);}
        inline bool operator< (const Address & rhs){return count < rhs.count; }
        inline bool operator> (const Address & rhs){return  count > rhs.count;}
        inline bool operator<=(const Address & rhs){return !operator> (rhs);}
        inline bool operator>=(const Address & rhs){return !operator< (rhs);}

    };
}

#endif