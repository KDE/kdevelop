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
#include "kdevplugin.h"


class KDialogBase;
class DocTreeViewWidget;
//class DocIndexDialog;
class Context;
class QPopupMenu;


class DocTreeViewPart : public KDevPlugin
{
    Q_OBJECT

public:
    DocTreeViewPart( QObject *parent, const char *name, const QStringList &args );
    ~DocTreeViewPart();

private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void configWidget(KDialogBase *dlg);
    void projectOpened();
    void projectClosed();
    void contextMenu(QPopupMenu *popup, const Context *context);
//  void slotDocumentationIndex();
    void slotSearchDocumentation();
    void slotManpage();
    void slotRaiseWidget();
    void slotContextGotoManpage();
//  void slotContextLookupIndex();
    void slotContextFulltextSearch();
    
private:
    QGuardedPtr<DocTreeViewWidget> m_widget;
//    QGuardedPtr<DocIndexDialog> m_indexDialog;
    QString m_popupstr;

    friend class DocTreeViewWidget;
};

#endif
