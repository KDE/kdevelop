/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCTREEVIEWPART_H_
#define _DOCTREEVIEWPART_H_

#include <qguardedptr.h>
#include "kdevpart.h"


class DocTreeViewWidget;
class DocIndexDialog;
class Context;


class DocTreeViewPart : public KDevPart
{
    Q_OBJECT

public:
    DocTreeViewPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~DocTreeViewPart();

private slots:
    void configWidget(KDialogBase *dlg);
    void projectOpened();
    void projectClosed();
    void contextMenu(QPopupMenu *popup, const Context *context);
    void slotDocumentationIndex();
    void slotSearchDocumentation();
    void slotRaiseWidget();
    void slotGotoManpage();
    void slotSearchDocumentationTerm();
    
private:
    QGuardedPtr<DocTreeViewWidget> m_widget;
    QGuardedPtr<DocIndexDialog> m_indexDialog;
    QString popupstr;

    friend class DocTreeViewWidget;
    friend class DocTreeConfigWidget;
};

#endif
