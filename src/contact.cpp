#include "contact.h"

namespace favor{
    Contact::Contact(long ident, string name) : id(ident), displayName(name), types(FLAG_EMPTY){}
    Contact::Contact(long ident, string name, const Address &addr) : id(ident), displayName(name), types(MessageTypeFlags[addr.type]) {
        addresses.push_back(addr);
    }
    Contact::Contact(long ident, string name, const vector <Address> &addrs) : id(ident), displayName(name),
                                                                                             types(FLAG_EMPTY), addresses(addrs) {
        for (auto it = addrs.begin(); it != addrs.end(); ++it){
            types = types | MessageTypeFlags[it->type];
        }
    }

    bool Contact::operator==(const Contact& rhs) const{
        //logger::info("Comparing "+as_string(*this)+" to "+as_string(rhs));
        //Eventually the vector comparison here may have to be adjusted to something more complicated, for example
        //in cases where vectors have similar conents and are out of order. Seems okay for now, though.
        return id == rhs.id && displayName == rhs.displayName && types == rhs.types && addresses == rhs.addresses;
    }

    const vector<Address>& Contact::getAddresses() const {
        return addresses;
    }

    bool Contact::hasType(MessageType type) const {
        return MessageTypeFlags[type] & types;
    }

    MessageTypeFlag Contact::typeFlags() const {
        return types;
    }
}