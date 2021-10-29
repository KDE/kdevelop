/*
    SPDX-FileCopyrightText: 2018 Amish K. Naidu <amhndu@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EMPTYMESSAGELISTVIEW_H
#define EMPTYMESSAGELISTVIEW_H

#include <QListView>

// subclass to show a message when the list is empty
class EmptyMessageListView
    : public QListView
{
    Q_OBJECT

public:
    explicit EmptyMessageListView(QWidget* parent);
    void setEmptyMessage(const QString& message);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QString m_message;
};

#endif // EMPTYMESSAGELISTVIEW_H
