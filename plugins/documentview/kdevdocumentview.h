/* This file is part of KDevelop
   Copyright 2005 Adam Treat <treat@kde.org>

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

#ifndef KDEVDOCUMENTVIEW_H
#define KDEVDOCUMENTVIEW_H

#include <QTreeView>

class KUrl;
class KDevDocumentViewPart;
class KDevDocumentModel;
class KDevDocumentViewDelegate;
class KDevDocumentSelection;
namespace KDevelop
{
    class IDocument;
}

class KDevDocumentModel;
class KDevDocumentItem;

class KDevDocumentView: public QTreeView
{
    Q_OBJECT
public:
    explicit KDevDocumentView( KDevDocumentViewPart *part, QWidget *parent );
    virtual ~KDevDocumentView();

    KDevDocumentViewPart *part() const;

signals:
    void activateURL( const KUrl &url );

private slots:
    void activated( KDevelop::IDocument* document );
    void saved( KDevelop::IDocument* document );
    void loaded( KDevelop::IDocument* document );
    void closed( KDevelop::IDocument* document );
    void contentChanged( KDevelop::IDocument* document );
    void stateChanged( KDevelop::IDocument* document );

protected:
    virtual void mousePressEvent( QMouseEvent * event );
    virtual void contextMenuEvent( QContextMenuEvent * event );

private:
    KDevDocumentViewPart *m_part;
    KDevDocumentModel *m_documentModel;
    KDevDocumentSelection* m_selectionModel;
    KDevDocumentViewDelegate* m_delegate;
//     KDevDocumentItem *m_documentItem;
    QHash< KDevelop::IDocument*, QModelIndex > m_doc2index;
};

#endif // KDEVDOCUMENTVIEW_H

