#ifndef LIBHELLO_H
#define LIBHELLO_H

#ifdef _WIN32
  #ifdef TINYAES_EXPORTS
    #define DLL_EXPORT __declspec(dllexport)
  #else
    #define DLL_EXPORT __declspec(dllimport)
  #endif
#else
  #define DLL_EXPORT
#endif

DLL_EXPORT void libhello(void);

#endif
