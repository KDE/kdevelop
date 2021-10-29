/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SESSIONCHOOSERDIALOG_H
#define SESSIONCHOOSERDIALOG_H

#include <QDialog>
#include <QTimer>

class QDialogButtonBox;
class QModelIndex;
class QListView;
class QLineEdit;
class QAbstractItemModel;

namespace KDevelop {

class SessionChooserDialog : public QDialog
{
    Q_OBJECT
public:
    SessionChooserDialog(QListView* view, QAbstractItemModel* model, QLineEdit* filter);

    bool eventFilter(QObject* object, QEvent* event) override;

    QWidget* mainWidget() const;

public Q_SLOTS:
    void updateState();
    void doubleClicked(const QModelIndex& index);
    void filterTextChanged();

private Q_SLOTS:
    void deleteButtonPressed();
    void showDeleteButton();
    void itemEntered(const QModelIndex& index);

private:
    QListView* const m_view;
    QAbstractItemModel* const m_model;
    QLineEdit* const m_filter;
    QTimer m_updateStateTimer;

    QDialogButtonBox* m_buttonBox;
    QWidget* m_mainWidget;
    QPushButton* m_deleteButton;
    QTimer m_deleteButtonTimer;
    int m_deleteCandidateRow;
};

}

#endif // SESSIONCHOOSERDIALOG_H
