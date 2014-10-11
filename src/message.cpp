#include "message.h"

namespace favor {
    message::message(MessageType t, bool s, long int i, std::time_t d, string a, bool m, string b) :
            type(t), sent(s), id(i), date(d), address(a), media(m), body(b) {
    }
}