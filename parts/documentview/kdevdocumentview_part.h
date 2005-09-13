/* This file is part of KDevelop
  Copyright (C) 2005 Adam Treat <treat@kde.org>

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

#ifndef __KDEVPART_KDEVDOCUMENTVIEW_H__
#define __KDEVPART_KDEVDOCUMENTVIEW_H__

#include <QtCore/QPointer>

#include <kdevplugin.h>

class QModelIndex;
class KURL;
class KDevDocumentView;
class KDevDocumentModel;
class KDevDocumentItem;

class KDevDocumentViewPart: public KDevPlugin
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
    KDevDocumentViewPart( QObject *parent, const char *name, const QStringList & );
    virtual ~KDevDocumentViewPart();

    void import( RefreshPolicy policy = Refresh );

signals:
    void refresh();

private slots:
    void saveFile( const KURL &url );
    void loadFile( const KURL &url );
    void closeFile( const KURL &url );
    void dirtyFile( const KURL &url );
    void filePressed( const QModelIndex & index );

private:
    KDevDocumentModel *m_documentModel;
    KDevDocumentItem *m_documentItem;
    KDevDocumentView *m_documentView;
    QPointer<QWidget> m_widget;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
