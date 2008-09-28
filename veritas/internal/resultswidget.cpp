/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "resultswidget.h"
#include <QTreeView>
#include <QHeaderView>

using Veritas::ResultsWidget;

ResultsWidget::ResultsWidget(QWidget* parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setWordWrap(true);
    setIndentation(30);
    setAlternatingRowColors(true);
    setWindowIcon(QIcon(":/icons/qxrunner_16x16"));

    header()->setStretchLastSection(false);
    header()->setMovable(false);
    header()->setClickable(false);

    tree()->setStyleSheet(
        "QTreeView::branch{"
        "image: none;"
        "border-image: none"
        "}");
}

ResultsWidget::~ResultsWidget()
{
}

void ResultsWidget::setResizeMode()
{
    header()->setResizeMode(QHeaderView::Fixed);
    header()->setResizeMode(0, QHeaderView::ResizeToContents); // test name
    header()->setResizeMode(1, QHeaderView::Stretch); // message
    header()->setResizeMode(2, QHeaderView::Interactive); // file name
    header()->setResizeMode(3, QHeaderView::ResizeToContents); // line number
}

QTreeView* ResultsWidget::tree()
{
    return this;
}

