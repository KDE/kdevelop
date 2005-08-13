/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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

#ifndef ACTIONEDITORIMPL_H
#define ACTIONEDITORIMPL_H

#include "actioneditor.h"

class QAction;
class FormWindow;
class ActionItem;

class ActionEditor : public ActionEditorBase
{
    Q_OBJECT

public:
    ActionEditor( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    void setFormWindow( FormWindow *fw );
    void updateActionName( QAction *a );
    void updateActionIcon( QAction *a );
    FormWindow *form() const { return formWindow; }

    bool wantToBeShown() const { return !explicitlyClosed; }
    void setWantToBeShown( bool b ) { explicitlyClosed = !b; }

    void setCurrentAction( QAction *a );
    QAction *newActionEx();

protected:
    void closeEvent( QCloseEvent *e );

protected slots:
    void currentActionChanged( QListViewItem * );
    void deleteAction();
    void newAction();
    void newActionGroup();
    void newDropDownActionGroup();
    void connectionsClicked();
    void removeConnections( QObject *o );

signals:
    void hidden();
    void removing( QAction * );

private:
    void insertChildActions( ActionItem *i );

private:
    QAction *currentAction;
    FormWindow *formWindow;
    bool explicitlyClosed;

};

#endif // ACTIONEDITORIMPL_H
