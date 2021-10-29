/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectmodelperformancetest.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QGridLayout>
#include <QPushButton>
#include <QTimer>
#include <QTreeView>

#include <projectmodel.h>
#include <path.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testplugincontroller.h>

// Knobs to increase/decrease the amount of items being generated
#define SMALL_DEPTH 2
#define SMALL_WIDTH 10
#define BIG_DEPTH 3
#define BIG_WIDTH 10
#define INIT_WIDTH 10
#define INIT_DEPTH 3

using KDevelop::ProjectModel;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectFileItem;
using KDevelop::Path;

void generateChilds( ProjectBaseItem* parent, int count, int depth )
{
    for( int i = 0; i < 10; i++ ) {
        if( depth > 0 ) {
            auto* item = new ProjectFolderItem( QStringLiteral( "f%1" ).arg( i ), parent );
            generateChilds( item, count, depth - 1 );
        } else {
            new ProjectFileItem( QStringLiteral( "f%1" ).arg( i ), parent );
        }
    }
}

ProjectModelPerformanceTest::ProjectModelPerformanceTest(QWidget* parent )
    : QWidget(parent)
{
    auto * l = new QGridLayout( this );
    setLayout( l );
    view = new QTreeView( this );
    // This is used so the treeview layout performance is not influencing the test
    view->setUniformRowHeights( true );

    auto* b = new QPushButton(QStringLiteral("Expand All"), this);
    connect( b, &QPushButton::clicked, view, &QTreeView::expandAll );
    l->addWidget( b, 0, 0 );
    b = new QPushButton( QStringLiteral("Collapse All"), this );
    connect( b, &QPushButton::clicked, view, &QTreeView::collapseAll );
    l->addWidget( b, 0, 1 );
    b = new QPushButton( QStringLiteral("Add Small Subtree"), this );
    connect( b, &QPushButton::clicked, this, &ProjectModelPerformanceTest::addSmallTree );
    l->addWidget( b, 0, 2 );
    b = new QPushButton( QStringLiteral("Add Big Subtree"), this );
    connect( b, &QPushButton::clicked, this, &ProjectModelPerformanceTest::addBigTree );
    l->addWidget( b, 0, 3 );
    b = new QPushButton( QStringLiteral("Add Big Subtree in Chunks"), this );
    connect( b, &QPushButton::clicked, this, &ProjectModelPerformanceTest::addBigTreeDelayed );
    l->addWidget( b, 0, 4 );

    l->addWidget( view, 1, 0, 1, 6 );
}

void ProjectModelPerformanceTest::init()
{
    QElapsedTimer timer;
    timer.start();
    KDevelop::AutoTestShell::init();
    auto* core = new KDevelop::TestCore;
    core->setPluginController(new KDevelop::TestPluginController(core));
    core->initialize();

    qDebug() << "init core" << timer.elapsed();
    timer.start();

    model = new KDevelop::ProjectModel( this );

    qDebug() << "create model" << timer.elapsed();
    timer.start();

    for( int i = 0; i < INIT_WIDTH; i++ ) {
        auto* item = new ProjectFolderItem( nullptr, Path( QUrl::fromLocalFile( QStringLiteral( "/f%1" ).arg( i ) ) ) );
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
        auto* item = new ProjectFolderItem( nullptr, Path( QUrl::fromLocalFile( QStringLiteral( "/f%1" ).arg( i ) ) ) );
        generateChilds( item, BIG_WIDTH, BIG_DEPTH );
        model->appendRow( item );
    }
    qDebug() << "addBigTree" << timer.elapsed();
}

void ProjectModelPerformanceTest::addBigTreeDelayed()
{
    originalWidth = model->rowCount();
    QTimer::singleShot( 0, this, &ProjectModelPerformanceTest::addItemDelayed );
}

void ProjectModelPerformanceTest::addItemDelayed()
{
    QElapsedTimer timer;
    timer.start();
    ProjectBaseItem* parent = nullptr;
    Path path;
    if( !currentParent.isEmpty() ) {
        parent = currentParent.top();
        path = Path(parent->path(), QStringLiteral("f%1").arg(parent->rowCount()));
    } else {
        path = Path(QUrl::fromLocalFile(QStringLiteral("/f%1").arg(model->rowCount())));
    }
    ProjectBaseItem* item = nullptr;
    if( currentParent.size() < BIG_DEPTH ) {
        item = new ProjectFolderItem(nullptr, path, parent);
    } else {
        item = new ProjectFileItem( nullptr, path, parent );
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
        QTimer::singleShot( 0, this, &ProjectModelPerformanceTest::addItemDelayed );
    }
    qDebug() << "addBigTreeDelayed" << timer.elapsed();
}

void ProjectModelPerformanceTest::addSmallTree()
{
    QElapsedTimer timer;
    timer.start();
    for( int i = 0; i < SMALL_WIDTH; i++ ) {
        auto* item = new ProjectFolderItem( nullptr, Path(QUrl::fromLocalFile( QStringLiteral( "/f%1" ).arg( i ) )) );
        generateChilds( item, SMALL_WIDTH, SMALL_DEPTH );
        model->appendRow( item );
    }
    qDebug() << "addSmallTree" << timer.elapsed();
}

int main( int argc, char** argv )
{
    QApplication a( argc, argv );
    auto* w = new ProjectModelPerformanceTest;
    w->show();
    w->setAttribute(Qt::WA_DeleteOnClose);

    QMetaObject::invokeMethod(w, "init");
    return a.exec();
}

