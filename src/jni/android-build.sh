PROJECTDIR="../.."
#http://www.snip2code.com/Snippet/32726/JNI-Build-Wrapper-for-Android-Studio

(	# subshell context for exported variables
export NDK_OUT="$PROJECTDIR/build/android/jni-obj";
export NDK_LIBS_OUT="$PROJECTDIR/build/android/jni-libs";
ndk-build $1
)