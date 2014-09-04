#include "logger.h"

#ifdef ANDROID
#include <android/log.h>
#define LOG_TAG "favor"
#define LOGE(s) __android_log_write(ANDROID_LOG_ERROR, LOG_TAG, s.c_str())
#define LOGW(s) __android_log_write(ANDROID_LOG_WARN, LOG_TAG, s.c_str())
#define LOGI(s) __android_log_write(ANDROID_LOG_INFO, LOG_TAG, s.c_str())
#else
#define LOGE(s) log("[error] "+s, LOG_ERROR)
#define LOGW(s) log("[warning] "+s, LOG_WARN)
#define LOGI(s) log("[info] "+s, LOG_INFO)
#endif

namespace favor{
  namespace logger{
    namespace {
      enum LogPriority {LOG_INFO, LOG_WARN, LOG_ERROR};
      inline void log(string s, LogPriority l){
	//TODO: this should check verbosity level vs log priority and write to an actual log file
	
	//dd/mm/yyyy hh:mm:ss
	char timestring[20];
	time_t t = time(NULL);
	strftime(timestring, sizeof(timestring), "%d/%m/%y %H:%M:%S", localtime(&t));
	cout << "[" << timestring << "] " << s << endl;
      }
    }
    void error(string s)
    {
      LOGE(s);
    }
    
  }
}