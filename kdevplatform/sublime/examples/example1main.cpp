/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "example1main.h"

#include <QMenu>
#include <QMenuBar>
#include <QListView>
#include <QTextEdit>
#include <QLayout>
#include <QPushButton>
#include <QUrl>
#include <QDebug>

#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/sublimedefs.h>
#include <sublime/tooldocument.h>
#include <sublime/urldocument.h>
#include <sublime/controller.h>
#include <sublime/mainwindow.h>

Example1Main::Example1Main()
    :KXmlGuiWindow(nullptr)
{
    //documents
    m_controller = new Sublime::Controller(this);
    Sublime::Document *doc1 = new Sublime::UrlDocument(m_controller, QUrl::fromLocalFile(QStringLiteral("~/foo.cpp")));
    Sublime::Document *doc2 = new Sublime::UrlDocument(m_controller, QUrl::fromLocalFile(QStringLiteral("~/boo.cpp")));
    Sublime::Document *doc3 = new Sublime::UrlDocument(m_controller, QUrl::fromLocalFile(QStringLiteral("~/moo.cpp")));

    //documents for tool views
    Sublime::Document *tool1 = new Sublime::ToolDocument(QStringLiteral("ListView"), m_controller,
        new Sublime::SimpleToolWidgetFactory<QListView>(QStringLiteral("ListView")));
    Sublime::Document *tool2 = new Sublime::ToolDocument(QStringLiteral("TextEdit"), m_controller,
        new Sublime::SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("TextEdit")));

    //areas (aka perspectives)
    qDebug() << "constructing area 1";
    m_area1 = new Sublime::Area(m_controller, QStringLiteral("Area 1"));
    m_controller->addDefaultArea(m_area1);
    m_area1->addView(doc1->createView());
    m_area1->addView(doc2->createView());
    m_area1->addView(doc3->createView());
    m_area1->addToolView(tool1->createView(), Sublime::Left);
    m_area1->addToolView(tool2->createView(), Sublime::Bottom);

    qDebug() << "constructing area 2";
    m_area2 = new Sublime::Area(m_controller, QStringLiteral("Area 2"));
    m_controller->addDefaultArea(m_area2);
    Sublime::View *view1 = doc1->createView();
    m_area2->addView(view1);
    Sublime::View *view2 = doc2->createView();
    m_area2->addView(view2, view1, Qt::Vertical);
    m_area2->addView(doc3->createView(), view2, Qt::Horizontal);
    m_area2->addToolView(tool1->createView(), Sublime::Bottom);
    m_area2->addToolView(tool2->createView(), Sublime::Right);

    //example main window stuff
    QWidget *w = new QWidget(this);
    setCentralWidget(w);
    auto *l = new QVBoxLayout(w);
    QMenu *areaMenu = menuBar()->addMenu(QStringLiteral("Areas"));
    areaMenu->addAction(QStringLiteral("Area 1"), this, SLOT(selectArea1()));
    areaMenu->addAction(QStringLiteral("Area 2"), this, SLOT(selectArea2()));
    QPushButton *b1 = new QPushButton(QStringLiteral("Area 1"), this);
    connect(b1, &QPushButton::clicked, this, &Example1Main::selectArea1);
    l->addWidget(b1);
    QPushButton *b2 = new QPushButton(QStringLiteral("Area 2"), this);
    connect(b2, &QPushButton::clicked, this, &Example1Main::selectArea2);
    l->addWidget(b2);
}

void Example1Main::selectArea1()
{
    auto *main = new Sublime::MainWindow(m_controller);
    connect(main, &Sublime::MainWindow::areaChanged, this, &Example1Main::updateTitle);
    m_controller->showArea(m_area1, main);
    main->show();
}

void Example1Main::selectArea2()
{
    auto *main = new Sublime::MainWindow(m_controller);
    connect(main, &Sublime::MainWindow::areaChanged, this, &Example1Main::updateTitle);
    m_controller->showArea(m_area2, main);
    main->show();
}

void Example1Main::updateTitle(Sublime::Area *area)
{
    auto *main = qobject_cast<Sublime::MainWindow*>(sender());
    main->setWindowTitle(area->objectName());
}


