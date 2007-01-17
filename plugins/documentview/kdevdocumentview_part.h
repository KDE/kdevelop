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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef __KDEVPART_KDEVDOCUMENTVIEW_H__
#define __KDEVPART_KDEVDOCUMENTVIEW_H__

#include <QtCore/QHash>
#include <QtCore/QPointer>

#include <kdevplugin.h>
#include <kdevdocumentcontroller.h>

class KUrl;
class QModelIndex;
class KDevDocumentView;
class KDevDocumentModel;
class KDevDocumentItem;

class KDevDocumentViewPart: public Koncrete::Plugin
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
    KDevDocumentViewPart( QObject *parent, const QStringList & );
    virtual ~KDevDocumentViewPart();

    // Koncrete::Plugin methods
    virtual QWidget *pluginView() const;
    virtual Qt::DockWidgetArea dockWidgetAreaHint() const;
    virtual bool isCentralPlugin() const;

private slots:
    void activated( Koncrete::Document* document );
    void saved( Koncrete::Document* document );
    void loaded( Koncrete::Document* document );
    void closed( Koncrete::Document* document );
    void externallyModified( Koncrete::Document* document );
    void urlChanged( Koncrete::Document* document, const KUrl &oldurl, const KUrl &newurl );
    void stateChanged( Koncrete::Document* document );

private:
    KDevDocumentModel *m_documentModel;
    KDevDocumentItem *m_documentItem;
    QPointer<KDevDocumentView> m_documentView;
    QHash< Koncrete::Document*, QModelIndex > m_doc2index;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
