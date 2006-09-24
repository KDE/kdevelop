/* This file is part of the KDE project
   Copyright (C) xxxx KFile Authors
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KBOOKMARKHANDLER_H_
#define _KBOOKMARKHANDLER_H_

#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>
#include "fileselector_widget.h"
//Added by qt3to4:
#include <QTextStream>

class QTextStream;
class KMenu;
class KActionMenu;

class KBookmarkHandler : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    KBookmarkHandler( KDevFileSelector *parent, KMenu *kpopupmenu=0 );
    ~KBookmarkHandler();

    // KBookmarkOwner interface:
    virtual void openBookmarkURL( const QString& url ) { emit openURL( url ); }
    virtual QString currentUrl() const;

    KMenu *menu() const { return m_menu; }

signals:
    void openURL( const QString& url );

private slots:
    void slotNewBookmark( const QString& text, const QByteArray& url,
                          const QString& additionalInfo );
    void slotNewFolder( const QString& text, bool open,
                        const QString& additionalInfo );
    void newSeparator();
    void endFolder();

protected:
    virtual void virtual_hook( int id, void* data );

private:
    KDevFileSelector *mParent;
    KMenu *m_menu;
    KBookmarkMenu *m_bookmarkMenu;

    QTextStream *m_importStream;

    //class KBookmarkHandlerPrivate *d;
};


#endif // _KBOOKMARKHANDLER_H_
