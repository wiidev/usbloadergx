/* libxml++/libxml++config.h.  Generated from libxml++config.h.in by configure.  */
#ifndef _LIBXMLPP_CONFIG_H
#define _LIBXMLPP_CONFIG_H 1

//#include <glibmmconfig.h>

#ifdef GLIBMM_CONFIGURE
/* compiler feature tests that are used during compile time and run-time
   by libxml++ only. */
#define LIBXMLCPP_EXCEPTIONS_ENABLED 1
#endif /* GLIBMM_CONFIGURE */

#ifdef GLIBMM_MSC
#define LIBXMLCPP_EXCEPTIONS_ENABLED 1
#endif /* GLIBMM_MSC */

#ifdef GLIBMM_DLL
  #if defined(LIBXMLPP_BUILD) && defined(_WINDLL)
    // Do not dllexport as it is handled by gendef on MSVC
    #define LIBXMLPP_API 
  #elif !defined(LIBXMLPP_BUILD)
    #define LIBXMLPP_API __declspec(dllimport)
  #else
    /* Build a static library */
    #define LIBXMLPP_API
  #endif /* LIBXMLPP_BUILD - _WINDLL */
#else
  #define LIBXMLPP_API
#endif /* GLIBMM_DLL */

#endif /* _LIBXMLPP_CONFIG_H */

