#ifndef favor_def_include
#define favor_def_include

//Macros
/*
 * For ease of use reasons, everywhere we use SQLite in favor, there should be an
 * sqlite* variable available named "db". If there's an exception to this rule, the 
 * perpetrator can call sqlite3_validate directly...
 */
#define sqlv(arg1) sqlite3_validate(arg1, db);
//Types
namespace favor{
  enum MessageType {TYPE_EMAIL, TYPE_ANDROIDTEXT, TYPE_LINE, TYPE_SKYPE, NUMBER_OF_TYPES};
  extern const char* MessageTypeName[];
}

//Database
#define DB_NAME "favor.db"


//Accounts table
#define ACCOUNT_TABLE "accounts"
#define ACCOUNT_TABLE_SCHEMA "(name TEXT NOT NULL, type INTEGER NOT NULL, details_json TEXT, PRIMARY KEY(name, type))"

//Messages tables, name has to be determined at runtime from type and accountname
/*Note:
 * http://www.sqlite.org/lang_createtable.html
 * "But the following declaration does not result in "x" being an alias for the rowid:
 * CREATE TABLE t(x INTEGER PRIMARY KEY DESC, y, z);"
 */
#define RECEIVED_TABLE_SCHEMA "(id INTEGER, address TEXT NOT NULL, date INTEGER NOT NULL, charcount INTEGER NOT NULL, media INTEGER NOT NULL, PRIMARY KEY(id))"
#define SENT_TABLE_SCHEMA "(id INTEGER, address TEXT NOT NULL, date INTEGER NOT NULL, charcount INTEGER NOT NULL, media INTEGER NOT NULL, PRIMARY KEY(id, address))"


#endif