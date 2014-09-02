#include <string.h>
#include <jni.h>
#include <iostream>
#include <vector>

#include "../favor.h"


int testFunction(){
	std::vector<int> myvector;
	myvector.push_back(1);
	myvector.push_back(3);
	myvector.push_back(5);
	return myvector.size();
}


extern "C" {
      JNIEXPORT jstring JNICALL
      //Java, then package name, then class name, then method name, separated by underscores (also underscores per "." in the package name)
      //Basically if you get an error calling this stuff, the solution is replacing every . in the error with an underscore
      Java_com_example_favortest_MainActivity_stringFromJNI
      (JNIEnv *env, jobject obj)
      {
            return env->NewStringUTF((favor::int_string(testFunction())+", Hello from C++ over JNI!").c_str());
      }
}
