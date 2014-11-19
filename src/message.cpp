#include "message.h"

namespace favor {
    Message::Message(MessageType t, bool s, long int i, std::time_t d, string a, bool m, const string& b, size_t cc) :
            type(t), sent(s), id(i), date(d), address(a), media(m), body(b), charCount(cc) {
    }

    bool Message::failure() const {
        return (date == 0 && charCount == 0);
    }

    string Message::prettyDate() const {
        //TODO: NYI

    }
}