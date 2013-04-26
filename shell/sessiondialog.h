/* This file is part of KDevelop
Copyright 2008 Andreas Pakulat <apaku@gmx.de>

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

#ifndef KDEVPLATFORM_SESSIONDIALOG_H
#define KDEVPLATFORM_SESSIONDIALOG_H

#include <kdialog.h>
#include <QtCore/QAbstractListModel>

namespace Ui
{
class SessionDialog;
}

class QModelIndex;
class QItemSelection;
class QVariant;

namespace KDevelop
{
class Session;

class SessionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    SessionModel( QObject* parent = 0 );
    int rowCount( const QModelIndex& = QModelIndex() ) const;
    QVariant data( const QModelIndex&, int = Qt::DisplayRole ) const;
    QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const;
    bool setData( const QModelIndex&, const QVariant&, int = Qt::DisplayRole );
    Qt::ItemFlags flags( const QModelIndex& ) const;
    void deleteSessions( const QList<QModelIndex>& );
    void activateSession( const QModelIndex& );
    void addSession();
    void cloneSession( const QModelIndex& );
};

class SessionDialog : public KDialog
{
    Q_OBJECT
public:
    SessionDialog( QWidget* = 0 );
    virtual ~SessionDialog();
private Q_SLOTS:
    void createSession();
    void deleteSession();
    void activateSession();
    void cloneSession();
    void enableButtons( const QModelIndex&, const QModelIndex& );
    void enableButtons( const QItemSelection&, const QItemSelection& );
    void enableButtons();
private:
    Ui::SessionDialog* m_ui;
    SessionModel* m_model;
};

}

#endif
