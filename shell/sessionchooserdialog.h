/* This file is part of KDevelop

    Copyright 2008 Andreas Pakulat <apaku@gmx.de>
    Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef SESSIONCHOOSERDIALOG_H
#define SESSIONCHOOSERDIALOG_H

#include <KDialog>
#include <QTimer>

class QModelIndex;
class QListView;
class QLineEdit;
class QAbstractItemModel;

namespace KDevelop {

class SessionChooserDialog : public KDialog
{
    Q_OBJECT
public:
    SessionChooserDialog(QListView* view, QAbstractItemModel* model, QLineEdit* filter);
    bool eventFilter(QObject* object, QEvent* event);

public Q_SLOTS:
    void updateState();
    void doubleClicked(const QModelIndex& index);
    void filterTextChanged();

private Q_SLOTS:
    void deleteButtonPressed();
    void showDeleteButton();
    void itemEntered(const QModelIndex& index);

private:
    QListView* m_view;
    QAbstractItemModel* m_model;
    QLineEdit* m_filter;
    QTimer m_updateStateTimer;

    QPushButton* m_deleteButton;
    QTimer m_deleteButtonTimer;
    int m_deleteCandidateRow;
};

}

#endif // SESSIONCHOOSERDIALOG_H
