/*
Copyright (C) 2015  Joshua Tanner (mindful.jt@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



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

        bool operator==(const Message& other) const;
        bool operator!=(const Message& other) const;

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