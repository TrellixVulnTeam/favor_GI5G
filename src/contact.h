#ifndef favor_contact_include
#define favor_contact_include

#include "favor.h"

namespace favor {
    class contact{
    public:
        const string address;
        const string suggestedName;
        const int count;

        contact(string addr, string suggName, int c) : address(addr), suggestedName(suggName), count(c) {}


        //The comparison and inequality operators are defined differently because we need them for different things
        inline bool operator==(const contact& rhs){ return address == rhs.address; }
        inline bool operator!=(const contact& rhs){return !operator==(rhs);}
        inline bool operator< (const contact& rhs){ return count < rhs.count; }
        inline bool operator> (const contact& rhs){return  count > rhs.count;}
        inline bool operator<=(const contact& rhs){return !operator> (rhs);}
        inline bool operator>=(const contact& rhs){return !operator< (rhs);}

    };
}

#endif