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
        inline bool operator==(const contact& lhs, const contact& rhs){ return lhs.address == rhs.address; }
        inline bool operator!=(const contact& lhs, const contact& rhs){return !operator==(lhs,rhs);}
        inline bool operator< (const contact& lhs, const contact& rhs){ return lhs.count < rhs.count; }
        inline bool operator> (const contact& lhs, const contact& rhs){return  operator< (rhs,lhs);}
        inline bool operator<=(const contact& lhs, const contact& rhs){return !operator> (lhs,rhs);}
        inline bool operator>=(const contact& lhs, const contact& rhs){return !operator< (lhs,rhs);}

    };
}

#endif