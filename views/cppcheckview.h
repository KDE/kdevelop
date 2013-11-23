/* This file is part of KDevelop
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#ifndef CPPCHECKVIEW_H
#define CPPCHECKVIEW_H

#include <QObject>
#include <QTreeView>

#include <KTextEditor/MovingRange>

#include "iview.h"

namespace cppcheck
{
class CppcheckView : public QTreeView, public cppcheck::IView
{
    Q_OBJECT

public:
    CppcheckView();
    ~CppcheckView();

    using QTreeView::setModel;
    using QTreeView::model;

    void setModel(cppcheck::Model* m);
    cppcheck::Model* model(void);

private:
    QList<KTextEditor::MovingRange*> ErrorLinesMakerList;

private Q_SLOTS:
    void openDocument(const QModelIndex& index);
    void doubleClicked(const QModelIndex&);
};
}
#endif // CPPCHECKVIEW_H
