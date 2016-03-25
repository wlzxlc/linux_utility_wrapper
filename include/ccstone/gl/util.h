#ifndef CCSTONE_OPENGL_UTIL_H_
#define CCSTONE_OPENGL_UTIL_H_
#include <stdio.h>
namespace CCStone {

#define NO_ERROR 0
#define NO_MEMORY -22
#define BAD_VALUE -23
#define NAME_NOT_FOUND -24

#ifndef CC_UNLIKELY
#define CC_UNLIKELY(exper) exper
#endif

#ifndef CC_LIKELY
#define CC_LIKELY(exper) exper
#endif 

#ifndef ANDROID_SINGLETON_STATIC_INSTANCE
#define ANDROID_SINGLETON_STATIC_INSTANCE(inst)
#endif 

#ifndef EGL_FRAMEBUFFER_TARGET_ANDROID
#define EGL_FRAMEBUFFER_TARGET_ANDROID 0
#endif


#ifndef LOG_TAG
#define  LOG_TAG "unknow"
#endif

#ifndef CONDITION
#define CONDITION(cond)     (__builtin_expect((cond)!=0, 0))
#endif

#ifndef LOGOUT
#define LOGOUT(fmt,...) printf(LOG_TAG " " fmt "\n", ## __VA_ARGS__)
#endif

#ifndef ALOGD
#define ALOGD(fmt,...) LOGOUT("D/" fmt, ##__VA_ARGS__)
#endif

#ifndef ALOGE
#define ALOGE(fmt,...) LOGOUT("E/" fmt, ##__VA_ARGS__)
#endif

#ifndef ALOGI
#define ALOGI(fmt,...) LOGOUT("I/" fmt,##__VA_ARGS__)
#endif

#ifndef ALOGW
#define ALOGW(fmt,...) LOGOUT("W/" fmt,##__VA_ARGS__)
#endif

#ifndef ALOGE_IF
#define ALOGE_IF(exper, fmt, ...)
#endif

#ifndef ALOGW_IF
#define ALOGW_IF(exper, fmt, ...) \
    ((CONDITION(exper)) \
    ? ((void)LOGOUT("V/" fmt, ##__VA_ARGS__)) \
    : (void)0 )

#endif

inline int __gl_assert()
{
  __builtin_trap();  
  return 0;
}

#ifndef LOG_ALWAYS_FATAL_IF
#define LOG_ALWAYS_FATAL_IF(exper,fmt, ...) \
    ((CONDITION(exper)) \
    ? (void)((LOGOUT("F/" fmt, ##__VA_ARGS__)) && __gl_assert()) \
    : (void)0 )

#endif

#ifndef LOG_ALWAYS_FATAL
#define LOG_ALWAYS_FATAL(fmt, ...) LOG_ALWAYS_FATAL_IF(true,fmt , ## __VA_ARGS__) 
#endif




#ifdef __cplusplus
    template<bool> struct CompileTimeAssert;
    template<> struct CompileTimeAssert<true> {}; 
#define COMPILE_TIME_ASSERT(_exp) \
    template class CompileTimeAssert< (_exp) >;
#endif
#define COMPILE_TIME_ASSERT_FUNCTION_SCOPE(_exp) \
    CompileTimeAssert<( _exp )>();

    // ---------------------------------------------------------------------------

#ifdef __cplusplus
    template<bool C, typename LSH, typename RHS> struct CompileTimeIfElse;
    template<typename LHS, typename RHS> 
        struct CompileTimeIfElse<true,  LHS, RHS> { typedef LHS TYPE; };
    template<typename LHS, typename RHS> 
        struct CompileTimeIfElse<false, LHS, RHS> { typedef RHS TYPE; };
#endif

    enum {
        /* flip source image horizontally (around the vertical axis) */
        HAL_TRANSFORM_FLIP_H    = 0x01,
        /* flip source image vertically (around the horizontal axis)*/
        HAL_TRANSFORM_FLIP_V    = 0x02,
        /* rotate source image 90 degrees clockwise */
        HAL_TRANSFORM_ROT_90    = 0x04,
        /* rotate source image 180 degrees */
        HAL_TRANSFORM_ROT_180   = 0x03,
        /* rotate source image 270 degrees clockwise */
        HAL_TRANSFORM_ROT_270   = 0x07,
        /* don't use. see system/window.h */
        HAL_TRANSFORM_RESERVED  = 0x08,
    };

typedef long long nsecs_t;
typedef int32_t status_t;

inline nsecs_t systemTime() 
{
    return 0;
}

template <class T>
class Singleton {
    public:
  static  T& getInstance() {
      return *(new T);
  } 
};

#ifndef ANDROID_SINGLETON_STATIC_INSTANCE
#define ANDROID_SINGLETON_STATIC_INSTANCE(inst) 
#endif
}
#endif
