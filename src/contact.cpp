#include "contact.h"

namespace favor{
    Contact::Contact(long ident, string name, MessageTypeFlag ts) : id(ident), displayName(name), types(ts){}
    Contact::Contact(long ident, string name, MessageTypeFlag ts,  const Address &addr) : id(ident), displayName(name), types(ts) {
        addresses.push_back(addr);
    }
    Contact::Contact(long ident, string name, MessageTypeFlag ts, const vector <Address> &addrs) : id(ident), displayName(name),
                                                                                             types(ts), addresses(addrs) {}

    const vector<Address>& Contact::getAddresses() const {
        return addresses;
    }

    void Contact::addAddress(const Address &addr) {
        addresses.push_back(addr);
    }

    bool Contact::hasType(MessageType type) {
        return MessageTypeFlags[type] & types;
    }
}