#ifndef favor_def_include
#define favor_def_include


//Types
#define TYPE_EMAIL 1
#define TYPE_ANDROIDTEXT 2
#define TYPE_LINE 3
#define TYPE_SKYPE 4

//Database
#define DB_NAME "favor.db"


//Accounts table
#define ACCOUNT_TABLE "accounts"
#define ACCOUNT_TABLE_SCHEMA "(name TEXT NOT NULL, type INTEGER NOT NULL, details_json TEXT)"


#endif