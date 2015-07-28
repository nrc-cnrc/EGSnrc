
#ifndef IAEA_CONFIG
#define IAEA_CONFIG

#include "egs_config1.h"

typedef float IAEA_Float;
typedef EGS_I16   IAEA_I16;
typedef EGS_I32   IAEA_I32;
typedef EGS_I64   IAEA_I64;

#ifdef __cplusplus
#define IAEA_EXTERN_C extern "C"
#else
#define IAEA_EXTERN_C extern
#endif

#ifdef WIN32 

#ifdef BUILD_IAEA_DLL
#define IAEA_EXPORT __declspec(dllexport)
#elif defined USE_IAEA_DLL
#define IAEA_EXPORT __declspec(dllimport)
#else
#define IAEA_EXPORT
#endif
#define IAEA_LOCAL

#else

#ifdef HAVE_VISIBILITY
#define IAEA_EXPORT __attribute__ ((visibility ("default")))
#define IAEA_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define IAEA_EXPORT
#define IAEA_LOCAL
#endif

#endif


#endif
