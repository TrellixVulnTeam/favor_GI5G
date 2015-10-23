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
    namespace {
        int utcOffset;
    }


    /* http://stackoverflow.com/a/9088549
    returns the utc timezone offset
    (e.g. -8 hours for PST)
    */
    void refresh_utc_offset() {

        time_t zero = 24*60*60L;
        struct tm * timeptr;
        int gmtime_hours;

        /* get the local time for Jan 2, 1900 00:00 UTC */
        timeptr = localtime( &zero );
        gmtime_hours = timeptr->tm_hour;

        /* if the local time is the "day before" the UTC, subtract 24 hours
          from the hours to get the UTC offset */
        if( timeptr->tm_mday < 2 )
            gmtime_hours -= 24;

        utcOffset = gmtime_hours;
    }


    void initialize() {
        assert(sqlite3_config(SQLITE_CONFIG_MULTITHREAD)==SQLITE_OK);
        assert(sqlite3_initialize()==SQLITE_OK);
        assert(sqlite3_threadsafe()); //We just want this to not be 0, not for it to be ==SQLITE_OK
        assert(sqlite3_enable_shared_cache(1)==SQLITE_OK);
        //The worker must be initialized first, because it will create the database if there is none
        worker::initialize();
        reader::initialize();
        refresh_utc_offset();
    }



    void cleanup() {
        worker::cleanup();
        reader::cleanup();
        sqlite3_shutdown();
    }


    //TODO: since this actually generates a DB file, it either needs its own very comprehensive test or to be tested by hand
    void rebuildDatabase() {
        worker::cleanup();
        reader::cleanup();
        if (remove(DB_PATH_FULL) != 0){
            logger::error("Unable to delete database");
        }
        worker::initialize();
        worker::buildDatabase();
        reader::initialize();
    }


    void sqlite3_validate(int result, sqlite3 *db) {
        switch (result) {
            case SQLITE_OK:
                break;
            case SQLITE_ROW:
                break;
            case SQLITE_DONE:
                break;
            default:
                DLOG("Sqlite failure with error code "+as_string(result));
                if (result == 14) logger::error("SQlite error #14, database failed to open");
                else logger::error("SQLite error #" + as_string(result) + ", db says \"" + sqlite3_errmsg(db) + "\"");
                throw sqliteException();
        }
    }

    string sqlite3_get_string(sqlite3_stmt* stmt, int col){
        const unsigned char *ptr = sqlite3_column_text(stmt, col);
        if (ptr == NULL) return "";
        else return string(reinterpret_cast<const char *>(ptr));
    }

    //thanks to http://www.blackdogfoundry.com/blog/supporting-regular-expressions-in-sqlite/
    static void sqlite3_regexp(sqlite3_context *context, int argc, sqlite3_value **argv) {
        //TODO: this work? why is it static?
        int numberOfMatches = 0;
        //regex is "^\+\d+$"
        if (argc == 2) {
            const char* patternChars = (const char*)sqlite3_value_text(argv[0]);
            const char* valueChars = (const char*)sqlite3_value_text(argv[1]);

            if (patternChars != NULL && valueChars != NULL){
                std::regex pattern(patternChars, std::regex::ECMAScript | std::regex::icase);
                string value(valueChars);
                if (regex_match(value, pattern)) numberOfMatches = 1;
            }

        } else logger::warning("Sqlite3 regexp function called with improper number of arguments ("+as_string(argc)+")");
        sqlite3_result_int(context, numberOfMatches);
    }

    void sqlite3_bind_regexp_function(sqlite3* db){
        sqlite3_create_function_v2(db, "REGEXP", 2, SQLITE_ANY | SQLITE_DETERMINISTIC, 0, sqlite3_regexp, NULL, NULL, NULL);
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



    double round(double d)
    {
        #ifdef ANDROID
        return floor(d + 0.5);
        #else
        return std::floor(d);
        #endif
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

    string to_utf8(const string& s, const string& inputEncoding){
        if (s.length() == 0) return "";
        string out;
        try {
            iconvpp::converter conv("UTF-8",   // output encoding
                                    inputEncoding,  // input encoding
                                    false,      // ignore errors
                                    (size_t)(s.size() * 1.5));     // buffer size. magic number is not good, but CPP wrapper seems to handle buffer size smartly, so probably okay
            conv.convert(s, out);
        } catch (std::runtime_error &e){
            throw badMessageDataException("Conversion to UTF-8 failed: "+string(e.what()));
        }
        return out;
    }


    string as_string(const Message &m) {
        //The seemingly redundant parenthesis around the inline conditions are actually very necessary
        string result = "[Message ID: " + (m.isIdKnown() ? as_string(m.id) : "<unknown>");
        result += " | Sent? " + (m.isSentKonwn()? as_string(m.sent): "<unknown>");
        result += " | Date: " + (m.isDateKnown()? m.prettyDate() : "<unknown>");
        result += " | Address: " + (m.isAddressKnown()? m.address : "<unknown>");
        result += " | Media? " + (m.isMediaKnown()? as_string(m.media()) : "<unknown>");
        result += " | Body Length: "+ (m.isCharCountKnown()? as_string((long)m.charCount) : "<unknown>");
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

    //For lack of a generalizable timegm()
    time_t to_time_t_utc( struct tm* timeptr ) {
        return mktime( timeptr ) + utcOffset * 3600;
    }


}