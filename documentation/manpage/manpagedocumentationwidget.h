/*  This file is part of KDevelop
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>
    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>

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

#ifndef MANPAGEDOCUMENTATIONWIDGET_H
#define MANPAGEDOCUMENTATIONWIDGET_H

#include <QStackedWidget>

class QTreeView;
class QProgressBar;

class ManPageDocumentationWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit ManPageDocumentationWidget(QWidget *parent = 0);
public slots:
    void manIndexLoaded();
    void sectionListUpdated();
    void sectionParsed();
private:
    QWidget* m_loadingWidget;
    QTreeView* m_treeView;
    QProgressBar* m_progressBar;
};

#endif // MANPAGEDOCUMENTATIONWIDGET_H
