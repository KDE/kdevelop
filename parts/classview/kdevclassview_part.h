/* This file is part of KDevelop
    Copyright (C) 2004,2005 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __KDEVPART_KDEVCLASSVIEW_H__
#define __KDEVPART_KDEVCLASSVIEW_H__

#include "kdevcodemodel.h"

#include <QtCore/QPointer>

#include <kdevplugin.h>

class KDevClassView;
class KDevCodeModel;
class QTimer;
class KURL;

/*
 Please read the README.dox file for more info about this part
 */
class KDevClassViewPart: public KDevPlugin
{
    Q_OBJECT
public:
    enum RefreshPolicy
    {
        Refresh,
        NoRefresh,
        ForceRefresh
    };

public:
    KDevClassViewPart(QObject *parent, const char *name, const QStringList &);
    virtual ~KDevClassViewPart();

    KDevCodeNamespaceItem *currentNamespaceItem() const;
    KDevCodeClassItem *currentClassItem() const;
    KDevCodeFunctionItem *currentFunctionItem() const;
    KDevCodeVariableItem *currentVariableItem() const;

    void import(RefreshPolicy policy = Refresh);

signals:
    void refresh();
    void addedProjectItem(KDevCodeItem *dom);
    void aboutToRemoveProjectItem(KDevCodeItem *dom);

private:
    KDevCodeItem *m_workspace;
    QPointer<KDevClassView> m_classView;
};

#endif
