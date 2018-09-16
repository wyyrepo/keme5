#ifndef QDBF_GLOBAL_H
#define QDBF_GLOBAL_H

#include <QtCore/QtGlobal>

/*
#if defined(QDBF_LIBRARY)
#  define QDBF_EXPORT Q_DECL_EXPORT
#else
#  define QDBF_EXPORT Q_DECL_IMPORT
#endif
*/

#if defined(QDBF_LIBRARY)
#  define QDBF_EXPORT
#else
#  define QDBF_EXPORT
#endif


#endif // QDBF_GLOBAL_H
