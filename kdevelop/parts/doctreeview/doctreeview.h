/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCTREEVIEW_H_
#define _DOCTREEVIEW_H_

#include <qguardedptr.h>
#include "kdevcomponent.h"


class DocTreeWidget;

class DocTreeView : public KDevComponent
{
    Q_OBJECT

public:
    DocTreeView( QObject *parent=0, const char *name=0 );
    ~DocTreeView();

    // This is currently called by DocTreeViewConfigWidget if the
    // changes are accepted. Maybe this could be done through
    // the component system (but maybe not ;-)
    void configurationChanged() {};

protected:
    virtual void setupGUI();
    virtual void configWidgetRequested(KDialogBase *dlg);
    virtual void projectSpaceClosed();
    virtual void projectSpaceOpened(ProjectSpace *pProjectSpace);
    
signals:
    void projectAPISelected();
    void projectManualSelected();

private:
    QGuardedPtr<DocTreeWidget> m_widget;
    friend class DocTreeWidget;
};

#endif
