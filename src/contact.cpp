#include "contact.h"

namespace favor{
    Contact::Contact(long ident, string name, MessageTypeFlag ts) : id(ident), displayName(name), types(ts){}
    Contact::Contact(long ident, string name, MessageTypeFlag ts,  const Address &addr) : id(ident), displayName(name), types(ts) {
        addresses.push_back(addr);
    }
    Contact::Contact(long ident, string name, MessageTypeFlag ts, const vector <Address> &addrs) : id(ident), displayName(name),
                                                                                             types(ts), addresses(addrs) {}

    bool Contact::operator==(const Contact& rhs) const{
        //logger::info("Comparing "+as_string(*this)+" to "+as_string(rhs));
        //Eventually the vector comparison here may have to be adjusted to something more complicated, for example
        //in cases where vectors have similar conents and are out of order. Seems okay for now, though.
        return id == rhs.id && displayName == rhs.displayName && types == rhs.types && addresses == rhs.addresses;
    }

    const vector<Address>& Contact::getAddresses() const {
        return addresses;
    }

    void Contact::addAddress(const Address &addr) {
        types = types | MessageTypeFlags[addr.type];
        addresses.push_back(addr);
    }

    bool Contact::hasType(MessageType type) const {
        return MessageTypeFlags[type] & types;
    }

    MessageTypeFlag Contact::typeFlags() const {
        return types;
    }
}