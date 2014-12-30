#ifndef favor_jni_processor_include
#define favor_jni_processor_include
#include <jni.h>
#include <processor.h>
#include "jni_globals.h"
#include "jni_exceptions.h"
#include "jni_reader.h"
#include "jni_string.h"

namespace favor{
    namespace jni{
        typedef long(*LongQuery)(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent);
        typedef double(*DoubleQuery)(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent);
        //NintiethPercentile, totalCharcount, totalMessageCount
        LongQuery longQueries[] = {&::favor::processor::responseTimeNintiethPercentile, &::favor::processor::totalCharcount, &::favor::processor::totalMessagecount};
        //averageCharcount, averageConversationalResponsetime
        DoubleQuery doubleQueries[] = {&::favor::processor::averageCharcount, &::favor::processor::averageConversationalResponsetime};
        /*
                double averageCharcount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        double averageConversationalResponsetime(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        long responseTimeNintiethPercentile(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);

        long totalCharcount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        long totalMessagecount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
         */

        JNIEXPORT jlong JNICALL longQuery(JNIEnv* env, jobject callingObj, jint query, jstring account, jint type, jlong contact, jlong fromDate, jlong untilDate, jboolean sent){
            JNIString accountName(env, account);
            AccountManager* accountManager = findAccountManager(accountName, (MessageType) type);
        }

        JNIEXPORT jdouble JNICALL doubleQuery(JNIEnv* env, jobject callingObj, jint query, jstring account, jint type, jlong contact, jlong fromDate, jlong untilDate, jboolean sent){
            JNIString accountName(env, account);
            AccountManager* accountManager = findAccountManager(accountName, (MessageType) type);
        }

        JNIEXPORT jlongArray JNICALL longMultiQuery(JNIEnv* env, jobject callingObj, jint query, jstring account, jint type, jlongArray contacts, jlong fromDate, jlong untilDate, jboolean sent){
            JNIString accountName(env, account);
            AccountManager* accountManager = findAccountManager(accountName, (MessageType) type);
        }

        JNIEXPORT jdoubleArray JNICALL doubleMultiQuery(JNIEnv* env, jobject callingObj, jint query, jstring account, jint type, jlongArray contacts, jlong fromDate, jlong untilDate, jboolean sent){
            JNIString accountName(env, account);
            AccountManager* accountManager = findAccountManager(accountName, (MessageType) type);
        }



        static JNINativeMethod processorMethodTable[] = {
                {"longQuery", "(ILjava/lang/String;IJJJZ)J", (void*) longQuery},
                {"doubleQuery", "(ILjava/lang/String;IJJJZ)D", (void*) doubleQuery},
                {"longMultiQuery", "(ILjava/lang/String;I[JJJZ)[J", (void*) longMultiQuery},
                {"doubleMultiQuery", "(ILjava/lang/String;I[JJJZ)[D", (void*) doubleMultiQuery}
        };
    }
}

#endif