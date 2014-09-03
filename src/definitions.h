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
/*Note:
 * Frankly, the accounts table doesn't seem worth indexing. We almost exclusively read all the records from it anyway, and it's tiny.
 * The only performance gains would be on deletes, which are very infrequent.
 */

//Contacts table
#define CONTACT_TABLE(type) "contacts_" + string(MessageTypeName[type]) + ""
#define CONTACT_TABLE_SCHEMA "(name TEXT NOT NULL, display_name TEXT NOT NULL, PRIMARY KEY(name))"

/* No need to index this table as its only element is its primary key */

//Addresses table
#define ADDRESS_TABLE(type) "addresses_" +string(MessageTypeName[type]) + ""
#define ADDRESS_TABLE_SCHEMA(type) "(address TEXT NOT NULL, contact_name TEXT NOT NULL REFERENCES " CONTACT_TABLE(type) "(name))"

#define ADDRESS_INDEX "i_" ADDRESS_TABLE
#define ADDRESS_INDEX_SCHEMA "(contact_name)"


//Messages tables, name has to be determined at runtime from type and accountname
/*Note:
 * http://www.sqlite.org/lang_createtable.html
 * "But the following declaration does not result in "x" being an alias for the rowid:
 * CREATE TABLE t(x INTEGER PRIMARY KEY DESC, y, z);"
 */
#define RECEIVED_TABLE_SCHEMA "(id INTEGER, address TEXT NOT NULL, date INTEGER NOT NULL, charcount INTEGER NOT NULL, media INTEGER NOT NULL, PRIMARY KEY(id))"
#define SENT_TABLE_SCHEMA "(id INTEGER, address TEXT NOT NULL, date INTEGER NOT NULL, charcount INTEGER NOT NULL, media INTEGER NOT NULL, PRIMARY KEY(id, address))"
/*Note:
 * Date is more selective than address and in that sense might seem like a better candidate for
 * being first in the index, but we will commonly query specifying an address and no date at all.
 * In this case, if address was not first, we could not even use the index at all. Also, queries are
 * sorted DESC.
 */
#define MESSAGE_INDEX_SCHEMA "(address DESC, date DESC)" 


#endif