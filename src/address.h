#ifndef favor_address_include
#define favor_address_include

#include "favor.h"
#include "reader.h"

namespace favor {
    class Address {
    public:
        const string addr;
        const int count;
        const long contactId;
        const MessageType type;

        Address(string addr, int c, int contact, MessageType t) : addr(addr), count(c), contactId(contact), type(t){}

        bool belongsToContact(){
            return contactId > -1;
        }

        bool operator==(const Address& other) const{
            return (type == other.type && contactId == other.contactId &&
                count == other.count && addr == other.addr);
        }

//        //TODO: This won't compile for reasons unclear, but makes sense as a method here
//        bool inDatabase(){
//            return reader::addressExists(addr, type);
//        }
    };
}

#endif