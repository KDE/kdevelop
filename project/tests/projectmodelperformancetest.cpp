/***************************************************************************
 *   Copyright 2010 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "projectmodelperformancetest.h"
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QDebug>

#include <projectmodel.h>
#include <path.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testplugincontroller.h>
#include <QTimer>
#include <QElapsedTimer>

// Knobs to increase/decrease the amount of items being generated
#define SMALL_DEPTH 2
#define SMALL_WIDTH 10
#define BIG_DEPTH 3
#define BIG_WIDTH 10
#define INIT_WIDTH 10
#define INIT_DEPTH 3
#include <QInputDialog>


using KDevelop::ProjectModel;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectFileItem;
using KDevelop::Path;

void generateChilds( ProjectBaseItem* parent, int count, int depth )
{
    for( int i = 0; i < 10; i++ ) {
        if( depth > 0 ) {
            ProjectFolderItem* item = new ProjectFolderItem( QString( "f%1" ).arg( i ), parent );
            generateChilds( item, count, depth - 1 );
        } else {
            new ProjectFileItem( QString( "f%1" ).arg( i ), parent );
        }
    }
}

ProjectModelPerformanceTest::ProjectModelPerformanceTest(QWidget* parent )
    : QWidget(parent)
{
    QGridLayout * l = new QGridLayout( this );
    setLayout( l );
    view = new QTreeView( this );
    // This is used so the treeview layout performance is not influencing the test
    view->setUniformRowHeights( true );

    QPushButton* b = new QPushButton( "Expand All", this );
    connect( b, SIGNAL(clicked()), view, SLOT(expandAll()) );
    l->addWidget( b, 0, 0 );
    b = new QPushButton( "Collapse All", this );
    connect( b, SIGNAL(clicked()), view, SLOT(collapseAll()) );
    l->addWidget( b, 0, 1 );
    b = new QPushButton( "Add Small Subtree", this );
    connect( b, SIGNAL(clicked()), SLOT(addSmallTree()) );
    l->addWidget( b, 0, 2 );
    b = new QPushButton( "Add Big Subtree", this );
    connect( b, SIGNAL(clicked()), SLOT(addBigTree()) );
    l->addWidget( b, 0, 3 );
    b = new QPushButton( "Add Big Subtree in Chunks", this );
    connect( b, SIGNAL(clicked()), SLOT(addBigTreeDelayed()) );
    l->addWidget( b, 0, 4 );

    l->addWidget( view, 1, 0, 1, 6 );
}

void ProjectModelPerformanceTest::init()
{
    QElapsedTimer timer;
    timer.start();
    KDevelop::AutoTestShell::init();
    KDevelop::TestCore* core = new KDevelop::TestCore;
    core->setPluginController(new KDevelop::TestPluginController(core));
    core->initialize();

    qDebug() << "init core" << timer.elapsed();
    timer.start();

    model = new KDevelop::ProjectModel( this );

    qDebug() << "create model" << timer.elapsed();
    timer.start();

    for( int i = 0; i < INIT_WIDTH; i++ ) {
        ProjectFolderItem* item = new ProjectFolderItem( 0, Path( QUrl::fromLocalFile( QString( "/f%1" ).arg( i ) ) ) );
        generateChilds( item, INIT_WIDTH, INIT_DEPTH );
        model->appendRow( item );
    }

    qDebug() << "init model" << timer.elapsed();
    timer.start();

    view->setModel( model );
    qDebug() << "set model" << timer.elapsed();
    timer.start();

}

ProjectModelPerformanceTest::~ProjectModelPerformanceTest()
{
    KDevelop::TestCore::shutdown();
    QApplication::quit();
}

void ProjectModelPerformanceTest::addBigTree()
{
    QElapsedTimer timer;
    timer.start();
    for( int i = 0; i < BIG_WIDTH; i++ ) {
        ProjectFolderItem* item = new ProjectFolderItem( 0, Path( QUrl::fromLocalFile( QString( "/f%1" ).arg( i ) ) ) );
        generateChilds( item, BIG_WIDTH, BIG_DEPTH );
        model->appendRow( item );
    }
    qDebug() << "addBigTree" << timer.elapsed();
}

void ProjectModelPerformanceTest::addBigTreeDelayed()
{
    originalWidth = model->rowCount();
    QTimer::singleShot( 0, this, SLOT(addItemDelayed()) );
}

void ProjectModelPerformanceTest::addItemDelayed()
{
    QElapsedTimer timer;
    timer.start();
    ProjectBaseItem* parent = 0;
    Path path;
    if( !currentParent.isEmpty() ) {
        parent = currentParent.top();
        path = Path(parent->path(), QString("f%1").arg(parent->rowCount()));
    } else {
        path = Path(QUrl::fromLocalFile(QString("/f%1").arg(model->rowCount())));
    }
    ProjectBaseItem* item = 0;
    if( currentParent.size() < BIG_DEPTH ) {
        item = new ProjectFolderItem(0, path, parent);
    } else {
        item = new ProjectFileItem( 0, path, parent );
    }
    if( currentParent.isEmpty() ) {
        model->appendRow( item );
    }

    // Abort/Continue conditions are:
    // Go one level deeper (by pushing item on stack) as long as we haven't reached the max depth or the max width
    // else if we've reached the max width then pop, i.e go one level up
    // else the next run will add a sibling to the just-generated item
    if( currentParent.size() < BIG_DEPTH && ( currentParent.isEmpty() || currentParent.top()->rowCount() < BIG_WIDTH ) ) {
        currentParent.push( item );
    } else if( !currentParent.isEmpty() && currentParent.top()->rowCount() >= BIG_WIDTH ) {
        currentParent.pop();
    }
    if( ( currentParent.isEmpty() && ( model->rowCount() - originalWidth ) < BIG_WIDTH ) || !currentParent.isEmpty() ) {
        QTimer::singleShot( 0, this, SLOT(addItemDelayed()) );
    }
    qDebug() << "addBigTreeDelayed" << timer.elapsed();
}

void ProjectModelPerformanceTest::addSmallTree()
{
    QElapsedTimer timer;
    timer.start();
    for( int i = 0; i < SMALL_WIDTH; i++ ) {
        ProjectFolderItem* item = new ProjectFolderItem( 0, Path(QUrl::fromLocalFile( QString( "/f%1" ).arg( i ) )) );
        generateChilds( item, SMALL_WIDTH, SMALL_DEPTH );
        model->appendRow( item );
    }
    qDebug() << "addSmallTree" << timer.elapsed();
}

int main( int argc, char** argv )
{
    QApplication a( argc, argv );
    ProjectModelPerformanceTest* w = new ProjectModelPerformanceTest;
    w->show();
    w->setAttribute(Qt::WA_DeleteOnClose);

    QMetaObject::invokeMethod(w, "init");
    return a.exec();
}

