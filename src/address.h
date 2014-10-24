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


        //The comparison and inequality operators are defined differently because we need them for different things
        inline bool operator==(const Address & rhs){return addr == rhs.addr;}
        inline bool operator!=(const Address & rhs){return !operator==(rhs);}
        inline bool operator<=(const Address & rhs){return !operator> (rhs);}
        inline bool operator>=(const Address & rhs){return !operator< (rhs);}

        inline bool operator< (const Address & rhs){
            if (contactId > -1){
                if (rhs.contactId > -1) return count < rhs.count;
                else return false;
            }
            else if (rhs.contactId > -1) return true;
            else return count < rhs.count;
        }

        inline bool operator> (const Address & rhs){
            if (contactId > -1){
                if (rhs.contactId > -1) return count > rhs.count;
                else return true;
            }
            else if (rhs.contactId > -1) return false;
            else return count > rhs.count;
        }

    };
}

#endif