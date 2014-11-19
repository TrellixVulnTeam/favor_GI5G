#include "messagequery.h"

namespace favor{
    template<typename T>
    MessageQuery<T>::MessageQuery(const AccountManager* account, Key k, time_t fDate, time_t uDate, sqlite3 *database) :
            fromDate(fDate), untilDate(uDate), addresses(account->contactAddresses()), db(database)  {
        if (fromDate > untilDate){
            logger::error("Could not construct valid query with fromDate "+as_string(fromDate)+" as it is > untilDate "+as_string(untilDate));
            throw queryException("Cannot run query with fromDate > untilDate");
        }

        empty = (addresses->size()==0);

        keys = k;
    }

    template<typename T>
    CollationQuery<T>::CollationQuery(const AccountManager *account, Key k, time_t fDate, time_t uDate, sqlite3 *database, CollationType op, bool sent) :
    MessageQuery<T>(account, k, fDate, uDate, database), operation(op), tableName(account->getTableName(sent)){}

    template<typename T>
    void CollationQuery<T>::finalize(){
        sql = "SELECT ";
        switch(operation){
            case SUM: sql+="SUM(";
            case AVERAGE: sql += "AVG(";
            case COUNT: sql += "COUNT(";
        }
        sql += keyString() +") FROM "+tableName+" ";
        appendWhereClause();
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
        bindWhereClause();


        sqlv(sqlite3_step(stmt));
        if (operation == SUM || operation == COUNT) result = sqlite3_column_int64(stmt, 0);
        else result = sqlite3_column_double(stmt, 0);
        sqlv(sqlite3_finalize(stmt));
    }

    template<typename T>
    void MessageQuery<T>::setGetAll() {
        getAll = true;
    }

    template<typename T>
    bool MessageQuery<T>::isEmpty(){
        return empty;
    }

    template<typename T>
    string MessageQuery<T>::keyString() {
//        ADDRESS = 1 << 0, //0 0001
//                DATE = 1 << 1, //0 0010
//                CHARCOUNT = 1 << 2, //0 0100
//                MEDIA = 1 << 3, //0 1000
//                BODY = 1 << 4, //1 0000
        //TODO: we need to keep information about the indices of certain keys so we know how to read them back
        string keystring;
        if (keys & ADDRESS){
            keystring += "address,";
            keyIndices[ADDRESS] = currentKeyIndex++;
        }
        if (keys & DATE){
            keystring += "date,";
            keyIndices[DATE] = currentKeyIndex++;
        }
        if (keys & CHARCOUNT){
            keystring += "charcount,";
            keyIndices[CHARCOUNT] = currentKeyIndex++;
        }
        if (keys & MEDIA){
            keystring += "media,";
            keyIndices[MEDIA] = currentKeyIndex++;
        }
        if (keys & BODY){
            keystring += "body,";
            keyIndices[BODY] = currentKeyIndex;
        }
        keystring = keystring.substr(0, keystring.length()-1); //Slice off the last comma
        return keystring;
    }

    template<typename T>
    void MessageQuery<T>::appendWhereClause(){
        sql += "WHERE ";
        bool fresh = true;
        if (!getAll){
            sql += "(";
            fresh = false;
            bindingIndices[ADDRESS_START] = currentBindingIndex;
            //TODO: vectorify
            for (int i = 0; i < addresses->size(); ++i){
                ++currentBindingIndex;
                sql += "address = ?";
                if (i < addresses->size() - 1) sql += " OR ";
            }
            sql += ")";
        }
        if (fromDate != -1){
            if (fresh) sql += " AND ";
            fresh = false;
            bindingIndices[FROMDATE] = currentBindingIndex++;
            sql += "date >= ?";
        }
        if (untilDate != -1){
            if (fresh) sql += " AND ";
            fresh = false;
            bindingIndices[UNTILDATE] = currentBindingIndex++;
            sql += "date <= ?";
        }
    }
    template<typename T>
    void MessageQuery<T>::bindWhereClause() {
        if (fromDate != -1) sqlv(sqlite3_bind_int64(stmt, bindingIndices[FROMDATE], fromDate));
        if (untilDate != -1) sqlv(sqlite3_bind_int64(stmt, bindingIndices[UNTILDATE], untilDate));
        for (int i = 0; i < addresses->size(); ++i){
            sqlite3_bind_text(stmt, bindingIndices[ADDRESS_START]+i, (*addresses)[i].addr.c_str(), (*addresses)[i].addr.length(), SQLITE_STATIC);
        }
    }

    template <typename T>
    T MessageQuery<T>::getResult() {
        return result;
    }

}