/* This file is part of KDevelop
 *
 * Copyright 2018 Amish K. Naidu <amhndu@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
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
    EmptyMessageListView(QWidget* parent);
    void setEmptyMessage(const QString& message);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QString m_message;
};

#endif // EMPTYMESSAGELISTVIEW_H
