#ifndef favor_core_include
#define favor_core_include

#include <string>
#include <vector>
#include <list>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <memory>
#include <iostream>
#include <utility>
#include <unordered_map>

//External libraries
#include "sqlite/sqlite3.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "pugixml/pugixml.hpp"

//Favor core headers
#include "definitions.h"
#include "exceptions.h"
#include "types.h"

namespace favor {
    class message;

    //Basic
    void initialize();

    void cleanup();

    //Utility
    void sqlite3_validate(int result, sqlite3 *db);

    string as_string(int i);

    string as_string(long l);

    string as_string(const favor::message &m);

    string as_string(const rapidjson::Document &json);
}

#endif