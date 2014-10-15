#include "message.h"

namespace favor {
    message::message(MessageType t, bool s, long int i, std::time_t d, string a, bool m, const string& b, size_t cc) :
            type(t), sent(s), id(i), date(d), address(a), media(m), body(b), charCount(cc) {
    }

    string message::prettyDate() {
        //TODO: NYI

    }

    string message::logString() {
        //TODO: use prettydate, and update this to account for missing values (which there will be sometimes when we pull from the DB)
        const char* bar = "--------------------";
        string result = bar;
        result += "\n";
        result += "Sent: "+as_string(sent) +"\n";
        result += "Id: "+as_string(id) +"\n";
        result += "Date: "+ as_string(date) +"\n";
        result += "Address: "+address +"\n";
        result += "Media: "+as_string(media)+"\n";
        result += "Body: ----\n";
        result += body;
        result += "\n----------\n";
        result += "Body length: " +as_string((long)charCount)+"\n";

        result += bar;
        return result;
    }
}