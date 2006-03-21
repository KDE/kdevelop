/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef SHARED_GLOBAL_H
#define SHARED_GLOBAL_H

#include <QtCore/qglobal.h>

#define QT_SHARED_EXTERN Q_DECL_EXPORT
#define QT_SHARED_IMPORT Q_DECL_IMPORT

#ifndef QT_NO_SHARED_EXPORT
#  ifdef QT_SHARED_LIBRARY
#    define QT_SHARED_EXPORT QT_SHARED_EXTERN
#  else
#    define QT_SHARED_EXPORT QT_SHARED_IMPORT
#  endif
#else
#  define QT_SHARED_EXPORT
#endif

#endif // SHARED_GLOBAL_H
