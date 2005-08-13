 /**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef CLASSBROWSERINTERFACE_H
#define CLASSBROWSERINTERFACE_H

//
//  W A R N I N G  --  PRIVATE INTERFACES
//  --------------------------------------
//
// This file and the interfaces declared in the file are not
// public. It exists for internal purpose. This header file and
// interfaces may change from version to version (even binary
// incompatible) without notice, or even be removed.
//
// We mean it.
//
//

#include <private/qcom_p.h>
#include <qobject.h>
class QListView;

// {4ede3c32-ae96-4b7a-9e38-9f1d93592391}
#ifndef IID_ClassBrowser
#define IID_ClassBrowser QUuid( 0x4ede3c32, 0xae96, 0x4b7a, 0x9e, 0x38, 0x9f, 0x1d, 0x93, 0x59, 0x23, 0x91 )
#endif

struct ClassBrowserInterface : public QUnknownInterface
{
    enum Type { Class, Function };

    virtual QListView *createClassBrowser( QWidget *parent ) const = 0;
    virtual void update( const QString &code ) const = 0;
    virtual void clear() const = 0;

    virtual void onClick( QObject *receiver, const char *slot ) = 0;
};

#endif
