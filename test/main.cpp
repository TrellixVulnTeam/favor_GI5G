#include <iostream>
#include "../src/favor.h"
#include "../src/worker.h"
#include "../src/reader.h"
#include "../src/logger.h"
//TODO: this should be replaced by a real testing framework ASAP

using namespace std;
using namespace favor;

int main(int argc, char **argv) {
    initialize();
    worker::buildDatabase();
    reader::refreshAll();

    rapidjson::Document test;
    test.Parse("{\"trackingAddresses\":[\"test1@email.com\", \"test2@email.com\"]}");
    cout << "parse error? " << test.HasParseError() << endl;
    cout << "test member count " << test.MemberCount() << endl;
    cout << "test has trackingAddresses " << test.HasMember("trackingAddresses") << endl;
    string name = "trackingAddresses";
    //vector<string>
    if (test.HasMember(name.c_str())) {

    }


    rapidjson::Value val;
    val = test["trackingAddresses"];
    cout << "trackingAddress array? " << val.IsArray() << endl;
    cout << val.Size() << endl;
    for (auto it = val.Begin(); it!= val.End(); ++it){
        cout << it->GetString() << endl;
    }


    //reader::accountList()->front()->updateMessages();
    //reader::accountList()->front()->updateContacts();
    

    //MessageType t, bool s, long int i, std::time_t d, string a, bool m, const string& b, size_t cc)
    //favor::message m(favor::TYPE_EMAIL, false, 5, 500, "test@test.com", true, "this is a test message", 25);
    //favor::worker::saveMessage(&m, testsql);

    /*
    This seem to be generally good, except it's hard to know because it seems like one of the emails I'm running tests on, after being processed for whitespace,
    starts including a character that's causing printing via cout << to terminate early? It's hard to know, but we'll be able to know for certain ocne we start
    saving results to the database.
     */

    return 0;
}