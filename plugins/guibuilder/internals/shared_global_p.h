/****************************************************************************
**
** Copyright (C) 1992-2006 Trolltech AS. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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

#ifdef QT_DESIGNER_STATIC
#define QDESIGNER_SHARED_EXTERN
#define QDESIGNER_SHARED_IMPORT
#else
#define QDESIGNER_SHARED_EXTERN Q_DECL_EXPORT
#define QDESIGNER_SHARED_IMPORT Q_DECL_IMPORT
#endif

#ifndef QT_NO_SHARED_EXPORT
#  ifdef QDESIGNER_SHARED_LIBRARY
#    define QDESIGNER_SHARED_EXPORT QDESIGNER_SHARED_EXTERN
#  else
#    define QDESIGNER_SHARED_EXPORT QDESIGNER_SHARED_IMPORT
#  endif
#else
#  define QDESIGNER_SHARED_EXPORT
#endif

#endif // SHARED_GLOBAL_H
