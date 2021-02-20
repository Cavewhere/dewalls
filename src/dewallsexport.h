#ifndef DEWALLSEXPORT_H
#define DEWALLSEXPORT_H

#include <QtGlobal>

/**
  These are required defines for exporting symbols in the cavewhere lib for
  windows. These do nothing on other platforms like mac and linux
  */
#if !defined(DEWALLS_STATIC)
#  if defined(DEWALLS_LIB)
#    define DEWALLS_LIB_EXPORT Q_DECL_EXPORT
#  else
#    define DEWALLS_LIB_EXPORT Q_DECL_IMPORT
#  endif
#else
#define DEWALLS_LIB_EXPORT
#endif

#endif // DEWALLSEXPORT_H

