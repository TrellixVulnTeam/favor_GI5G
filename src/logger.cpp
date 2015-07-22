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



#include "logger.h"

#ifdef ANDROID
#include <android/log.h>
#define LOG_TAG "favor"
#define LOGE(s) __android_log_write(ANDROID_LOG_ERROR, LOG_TAG, s.c_str())
#define LOGW(s) __android_log_write(ANDROID_LOG_WARN, LOG_TAG, s.c_str())
#define LOGI(s) __android_log_write(ANDROID_LOG_INFO, LOG_TAG, s.c_str())
#define LOGD(s) __android_log_write(ANDROID_LOG_DEBUG, LOG_TAG, s.c_str())
#else
#define LOGE(s) log("[error] "+s, LOG_ERROR)
#define LOGW(s) log("[warning] "+s, LOG_WARN)
#define LOGI(s) log("[info] "+s, LOG_INFO)
#define LOGD(s) log("[debug] "+s, LOG_DEBUG)
#endif

using namespace std;
namespace favor {
    namespace logger {
        namespace {
            enum LogPriority {
                LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR
            };

            inline void log(string s, LogPriority l) {
                //TODO: this should check verbosity level vs log priority and write to an actual log file

                //dd/mm/yyyy hh:mm:ss
                char timestring[20];
                time_t t = time(NULL);
                strftime(timestring, sizeof(timestring), "%d/%m/%y %H:%M:%S", localtime(&t));
                cout << "[" << timestring << "] " << s << endl;
            }
        }

        void error(string s) {
            LOGE(s);
        }

        void warning(string s) {
            LOGW(s);
        }

        void info(string s) {
            LOGI(s);
        }

        void debug(string s){
            LOGD(s);
        }


    }
}