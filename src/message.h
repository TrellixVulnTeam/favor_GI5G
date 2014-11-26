#ifndef favor_message_include
#define favor_message_include

#include "favor.h"

namespace favor {
    class Message {
    private:

        const std::shared_ptr<string> bodyPtr;
        const short mediaVal; //Represents a bool, but we need a little breathing room to know when we don't know
    public:
        static constexpr long FAIL_VALUE = -2;
        static constexpr short UNKNOWN_NUMERIC_VALUE = -1;
        static constexpr const char* UNKNOWN_ADDRESS_VALUE = ""; //Message bodies can be empty, but an empty string address would be meaningless
        //What about strings?


        const MessageType type;
        const bool sent;
        const long id;
        const std::time_t date;
        const string address;
        const long charCount;
        const string& body() const;
        bool media() const;

        Message(MessageType t, bool s, long int i, std::time_t d, string a, short m, long cc, const string& b);
        Message(MessageType t, bool s, long int i, std::time_t d, string a, short m, long cc);

        string prettyDate() const;

        bool failure() const;


        bool isSentKonwn() const; //Should always be true
        bool isIdKnown() const; //Most often false for messages read back from the database
        bool isDateKnown() const;
        bool isAddressKnown() const; //Usually true, though not always if we're only pulling from one address anyway
        bool isMediaKnown() const;
        bool isBodyKnown()const;
        bool isCharCountKnown() const;

        static Message createFailure(MessageType t, bool s, long int i, string a);

    };
}

#endif