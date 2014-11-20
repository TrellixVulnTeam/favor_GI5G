#include "message.h"

namespace favor {
    Message::Message(MessageType t, bool s, long int i, std::time_t d, string a, short m, long cc, const string& b) :
            type(t), sent(s), id(i), date(d), address(a), mediaVal(m), bodyPtr(std::make_shared<const string>(b)), charCount(cc) {}

    Message::Message(MessageType t, bool s, long int i, std::time_t d, string a, short m, long cc) :
            type(t), sent(s), id(i), date(d), address(a), mediaVal(m), bodyPtr(NULL), charCount(cc) {}

    Message Message::createFailure(MessageType t, bool s, long int i, string a) {
        return Message(t, s, i, FAIL_VALUE, a, 0, FAIL_VALUE);
    }

    bool Message::failure() const {
        return (date == FAIL_VALUE && charCount == FAIL_VALUE);
    }

    const string& Message::body() const{
        return (*bodyPtr);
    }

    bool Message::media() const{
        return mediaVal > 0;
    }

    bool Message::isSentKonwn() const{
        return true; //This is just here for completeless, we should always always know
    }

    bool Message::isIdKnown() const{
        return id != UNKNOWN_NUMERIC_VALUE; //TODO: this doesn't work, we're signed up for negative IDS on android
    }

    bool Message::isDateKnown() const{
        return date != UNKNOWN_NUMERIC_VALUE;
    }

    bool Message::isMediaKnown() const{
        return mediaVal != UNKNOWN_NUMERIC_VALUE;
    }

    bool Message::isCharCountKnown() const{
        return charCount != UNKNOWN_NUMERIC_VALUE;
    }

    bool Message::isAddressKnown() const{
        return address != UNKNOWN_ADDRESS_VALUE;
    }

    bool Message::isBodyKnown() const {
        return bodyPtr != NULL;
    }


    string Message::prettyDate() const {
        //TODO: NYI
        return as_string(date);
    }
}