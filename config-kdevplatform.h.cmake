// Provide wrapper for noexcept: Q_DECL_NOEXCEPT (also part of Qt5's qglobal.h)
// TODO: Just use QtCore/qglobal.h when porting to KF5
#include <QtCore/qglobal.h>

#ifndef Q_DECL_NOEXCEPT
#ifdef Q_OS_WIN
#define Q_DECL_NOEXCEPT
#else
#define Q_DECL_NOEXCEPT noexcept
#endif
#endif

#define KDEV_ITEMREPOSITORY_VERSION @KDEV_ITEMREPOSITORY_VERSION@