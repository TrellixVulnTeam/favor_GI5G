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



#include "favor.h"
#include "worker.h"
#include "reader.h"
#include "logger.h"
#include "message.h"

using namespace std;

namespace favor {
    const char *MessageTypeName[] = {NAME_EMAIL, NAME_ANDROIDTEXT, NAME_LINE, NAME_SKYPE};
    const char* dbPath = ".";
    const char* dbName = "/favor.db";

    void initialize() {
        assert(sqlite3_config(SQLITE_CONFIG_MULTITHREAD)==SQLITE_OK);
        assert(sqlite3_initialize()==SQLITE_OK);
        assert(sqlite3_threadsafe()); //We just want this to not be 0, not for it to be ==SQLITE_OK
        assert(sqlite3_enable_shared_cache(1)==SQLITE_OK);
        //The worker must be initialized first, because it will create the database if there is none
        worker::initialize();
        reader::initialize();
    }

    void cleanup() {
        worker::cleanup();
        reader::cleanup();
        sqlite3_shutdown();
    }


    void sqlite3_validate(int result, sqlite3 *db, bool constraintFailureOK) {
        switch (result) {
            case SQLITE_OK:
                break;
            case SQLITE_ROW:
                break;
            case SQLITE_DONE:
                break;
            case SQLITE_CONSTRAINT:
                if (!constraintFailureOK){
                    logger::error("SQLite error #" + as_string(result) + ", db says \"" + sqlite3_errmsg(db) + "\"");
                    throw constraintViolationException();
                }
                else {
                    logger::warning("SQLite constraint failed, db says \"" + string(sqlite3_errmsg(db)) + "\"");
                    break;
                }
            default:
                logger::error("SQLite error #" + as_string(result) + ", db says \"" + sqlite3_errmsg(db) + "\"");
                throw sqliteException();
        }
    }

    bool compareAddress(const Address &lhs, const Address &rhs) {
        //Results in a largest first array
        if (lhs.contactId > -1){
            if (rhs.contactId > -1){
                lhs.count == rhs.count ? lhs.addr > rhs.addr : lhs.count > rhs.count;
            }
            else return true;
        }
        else if (rhs.contactId > -1) return false;
        else{
            return lhs.count == rhs.count ? lhs.addr > rhs.addr : lhs.count > rhs.count;
        }
    }

    //On Android, these methods will not behave well with numbers whose digit count exceeds the DIGIT_MAX
    string as_string(long int l) {
    #ifdef ANDROID
    #define LONG_DIGIT_MAX 20
    char result[LONG_DIGIT_MAX] = {0};
    sprintf(result, "%ld", l);
    return string(result);
    #else
        return to_string(l);
    #endif
    }

    string as_string(int i) {
    #ifdef ANDROID
    #define INT_DIGIT_MAX 10
    char result[INT_DIGIT_MAX] = {0};
    sprintf(result, "%d", i);
    return string(result);
    #else
        return to_string(i);
    #endif
    }

    string as_string(float f){
    #ifdef ANDROID
    #define FLOAT_DIGIT_MAX 10
    char result[FLOAT_DIGIT_MAX] = {0};
    sprintf(result, "%.7g", f);
    return string(result);
    #else
        return to_string(f);
    #endif
    }

    string as_string(double d){
    #ifdef ANDROID
	#define DOUBLE_DIGIT_MAX 20
    char result[DOUBLE_DIGIT_MAX] = {0};
    sprintf(result, "%.15g", d);
    return string(result);
    #else
        return to_string(d);
    #endif
    }

    string as_string(const rapidjson::Document &json) {
        rapidjson::StringBuffer buff;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
        json.Accept(writer);
        return buff.GetString();
    }

    string as_string(const Message &m) {
        string result = "[Message ID: " + (m.isIdKnown() ? as_string(m.id) : "<unknown>");
        result += " | Sent? " + (m.isSentKonwn()? as_string(m.sent): "<unknown>");
        result += " | Date: " + (m.isDateKnown()? m.prettyDate() : "<unknown>");
        result += " | Address: " + (m.isAddressKnown()? m.address : "<unknown>");
        result += " | Media? " + (m.isMediaKnown()? as_string(m.media()) : "<unknown>");
        result += " | Body Length: "+ m.isCharCountKnown()? as_string((long)m.charCount) : "<unknown>";
        result += ("| Body: <<" + (m.isBodyKnown() ? m.body() : " <unknown> ")) + ">>]";
        return result;
    }

    string as_string(const Address& a){
        return "[Address: "+a.addr+" | Count: " + as_string(a.count) + " | Contact ID: " + as_string(a.contactId)+"]";

    }

    string as_string(const Contact& c){
        string result = "[Contact Display Name: " +c.displayName + " | ID: "+as_string(c.id)+"| Addresses: (";
        if (c.getAddresses().size() == 0) result += ")]";
        else for (int i = 0; i < c.getAddresses().size(); ++i){
            if (i == c.getAddresses().size() - 1) result += as_string(c.getAddresses()[i]) + ")]";
            else result += as_string(c.getAddresses()[i]) + ", ";
        }
        return result;
    }

    string as_string(const AccountManager& a){
        string result = "[Account Name: "+a.accountName+ "| Type: "+as_string((int)a.type)+" | JSON: \""+a.getJson()+"\"]";
        return result;
    }

    string lowercase(const string &s){
        string ret = s;
        transform(s.begin(), s.end(), ret.begin(), ::tolower); //Only compiles specifically with "::tolower"
        return ret;
    }


}