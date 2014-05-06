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

#include "projectmodeltest.h"
#include <QtTest/QTest>
#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QThread>
#include <qtest_kde.h>
#include <KMimeType>

#include <projectmodel.h>
#include <projectproxymodel.h>
#include <tests/modeltest.h>
#include <tests/testproject.h>
#include <tests/kdevsignalspy.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

using KDevelop::ProjectModel;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectFileItem;
using KDevelop::ProjectExecutableTargetItem;
using KDevelop::ProjectLibraryTargetItem;
using KDevelop::ProjectTargetItem;
using KDevelop::ProjectBuildFolderItem;
using KDevelop::Path;

using KDevelop::TestProject;

void debugItemModel(QAbstractItemModel* m, const QModelIndex& parent=QModelIndex(), int depth=0)
{
    Q_ASSERT(m);
    qDebug() << QByteArray(depth*2, '-') << m->data(parent).toString();
    for(int i=0; i<m->rowCount(parent); i++) {
        debugItemModel(m, m->index(i, 0, parent), depth+1);
    }
}

void ProjectModelTest::initTestCase()
{
    KDevelop::AutoTestShell::init();
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

    qRegisterMetaType<QModelIndex>("QModelIndex");
    model = KDevelop::ICore::self()->projectController()->projectModel();
    new ModelTest( model, this );
    proxy = new ProjectProxyModel( model );
    new ModelTest(proxy, proxy);
    proxy->setSourceModel(model);
}

void ProjectModelTest::init()
{
    model->clear();
}

void ProjectModelTest::cleanupTestCase()
{
    KDevelop::TestCore::shutdown();
}

void ProjectModelTest::testCreateFileSystemItems()
{
    QFETCH( int, itemType );
    QFETCH( Path, itemPath );
    QFETCH( Path, expectedItemPath );
    QFETCH( QString, expectedItemText );
    QFETCH( QStringList, expectedRelativeItemPath );
    QFETCH( int, expectedItemRow );

    ProjectBaseItem* newitem = 0;
    switch( itemType ) {
        case ProjectBaseItem::Folder:
            newitem = new ProjectFolderItem( 0, itemPath );
            break;
        case ProjectBaseItem::BuildFolder:
            newitem = new ProjectBuildFolderItem( 0, itemPath );
            break;
        case ProjectBaseItem::File:
            newitem = new ProjectFileItem( 0, itemPath );
            break;
    }
    int origRowCount = model->rowCount();
    model->appendRow( newitem );
    QCOMPARE( model->rowCount(), origRowCount+1 );
    QCOMPARE( newitem->row(), expectedItemRow );
    QModelIndex idx = model->index( expectedItemRow, 0, QModelIndex() );
    QVERIFY( model->itemFromIndex( idx ) );
    QCOMPARE( model->itemFromIndex( idx ), newitem );
    QCOMPARE( newitem->text(), expectedItemText );
    QCOMPARE( newitem->path(), expectedItemPath );
    if( itemType == ProjectBaseItem::File ) {
        QCOMPARE( dynamic_cast<ProjectFileItem*>( newitem )->fileName(), expectedItemText );
    }
    if( itemType == ProjectBaseItem::Folder || itemType == ProjectBaseItem::BuildFolder ) {
        QCOMPARE( dynamic_cast<ProjectFolderItem*>( newitem )->folderName(), expectedItemText );
    }
    QCOMPARE( newitem->type(), itemType );
    QCOMPARE( model->data( idx ).toString(), expectedItemText );
    QCOMPARE( model->indexFromItem( newitem ), idx );
    QCOMPARE( model->pathFromIndex( idx ), expectedRelativeItemPath );
    QCOMPARE( model->pathToIndex( expectedRelativeItemPath ), idx );
}

void ProjectModelTest::testCreateFileSystemItems_data()
{
    QTest::addColumn<int>( "itemType" );
    QTest::addColumn<Path>( "itemPath" );
    QTest::addColumn<Path>( "expectedItemPath" );
    QTest::addColumn<QString>( "expectedItemText" );
    QTest::addColumn<QStringList>( "expectedRelativeItemPath" );
    QTest::addColumn<int>( "expectedItemRow" );

    QTest::newRow("RootFolder")
        << (int)ProjectBaseItem::Folder
        << Path(KUrl::fromPath("/rootdir"))
        << Path(KUrl::fromPath("/rootdir/"))
        << QString::fromLatin1("rootdir")
        << ( QStringList() << "rootdir" )
        << 0;

    QTest::newRow("RootBuildFolder")
        << (int)ProjectBaseItem::BuildFolder
        << Path(KUrl::fromPath("/rootdir"))
        << Path(KUrl::fromPath("/rootdir/"))
        << QString::fromLatin1("rootdir")
        << ( QStringList() << "rootdir" )
        << 0;

    QTest::newRow("RootFile")
        << (int)ProjectBaseItem::File
        << Path(KUrl::fromPath("/rootfile"))
        << Path(KUrl::fromPath("/rootfile"))
        << QString::fromLatin1("rootfile")
        << ( QStringList() << "rootfile" )
        << 0;

}

void ProjectModelTest::testCreateTargetItems()
{
    QFETCH( int, itemType );
    QFETCH( QString, itemText );
    QFETCH( QString, expectedItemText );
    QFETCH( QStringList, expectedItemPath );
    QFETCH( int, expectedItemRow );

    ProjectBaseItem* newitem = 0;
    switch( itemType ) {
        case ProjectBaseItem::Target:
            newitem = new ProjectTargetItem( 0, itemText );
            break;
        case ProjectBaseItem::LibraryTarget:
            newitem = new ProjectLibraryTargetItem( 0, itemText );
            break;
    }
    int origRowCount = model->rowCount();
    model->appendRow( newitem );
    QCOMPARE( model->rowCount(), origRowCount+1 );
    QCOMPARE( newitem->row(), expectedItemRow );
    QModelIndex idx = model->index( expectedItemRow, 0, QModelIndex() );
    QVERIFY( model->itemFromIndex( idx ) );
    QCOMPARE( model->itemFromIndex( idx ), newitem );
    QCOMPARE( newitem->text(), expectedItemText );
    QCOMPARE( newitem->type(), itemType );
    QCOMPARE( model->data( idx ).toString(), expectedItemText );
    QCOMPARE( model->indexFromItem( newitem ), idx );
    QCOMPARE( model->pathFromIndex( idx ), expectedItemPath );
    QCOMPARE( model->pathToIndex( expectedItemPath ), idx );
}

void ProjectModelTest::testCreateTargetItems_data()
{
    QTest::addColumn<int>( "itemType" );
    QTest::addColumn<QString>( "itemText" );
    QTest::addColumn<QString>( "expectedItemText" );
    QTest::addColumn<QStringList>( "expectedItemPath" );
    QTest::addColumn<int>( "expectedItemRow" );

    QTest::newRow("RootTarget")
        << (int)ProjectBaseItem::Target
        << "target"
        << QString::fromLatin1("target")
        << ( QStringList() << "target" )
        << 0;

    QTest::newRow("RootLibraryTarget")
        << (int)ProjectBaseItem::LibraryTarget
        << "libtarget"
        << QString::fromLatin1("libtarget")
        << ( QStringList() << "libtarget" )
        << 0;
}

void ProjectModelTest::testChangeWithProxyModel()
{
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel( this );
    proxy->setSourceModel( model );
    ProjectFolderItem* root = new ProjectFolderItem( 0, Path(KUrl::fromPath("/folder1")) );
    root->appendRow( new ProjectFileItem( 0, Path(KUrl::fromPath("/folder1/file1")) ) );
    model->appendRow( root );

    QCOMPARE( model->rowCount(), 1 );
    QCOMPARE( proxy->rowCount(), 1 );

    model->removeRow( 0 );

    QCOMPARE( model->rowCount(), 0 );
    QCOMPARE( proxy->rowCount(), 0 );
}

void ProjectModelTest::testCreateSimpleHierarchy()
{
    QString folderName = "rootfolder";
    QString fileName = "file";
    QString targetName = "testtarged";
    QString cppFileName = "file.cpp";
    ProjectFolderItem* rootFolder = new ProjectFolderItem( 0, Path(KUrl::fromPath("/"+folderName)) );
    QCOMPARE(rootFolder->baseName(), folderName);
    ProjectFileItem* file = new ProjectFileItem( fileName, rootFolder );
    QCOMPARE(file->baseName(), fileName);
    ProjectTargetItem* target = new ProjectTargetItem( 0, targetName );
    rootFolder->appendRow( target );
    ProjectFileItem* targetfile = new ProjectFileItem( 0, Path(rootFolder->path(), cppFileName), target );

    model->appendRow( rootFolder );

    QCOMPARE( model->rowCount(), 1 );
    QModelIndex folderIdx = model->index( 0, 0, QModelIndex() );
    QCOMPARE( model->data( folderIdx ).toString(), folderName );
    QCOMPARE( model->rowCount( folderIdx ), 2 );
    QCOMPARE( model->itemFromIndex( folderIdx ), rootFolder );
    QVERIFY( rootFolder->hasFileOrFolder( fileName ) );

    QModelIndex fileIdx = model->index( 0, 0, folderIdx );
    QCOMPARE( model->data( fileIdx ).toString(), fileName );
    QCOMPARE( model->rowCount( fileIdx ), 0 );
    QCOMPARE( model->itemFromIndex( fileIdx ), file );

    QModelIndex targetIdx = model->index( 1, 0, folderIdx );
    QCOMPARE( model->data( targetIdx ).toString(), targetName );
    QCOMPARE( model->rowCount( targetIdx ), 1 );
    QCOMPARE( model->itemFromIndex( targetIdx ), target );

    QModelIndex targetFileIdx = model->index( 0, 0, targetIdx );
    QCOMPARE( model->data( targetFileIdx ).toString(), cppFileName );
    QCOMPARE( model->rowCount( targetFileIdx ), 0 );
    QCOMPARE( model->itemFromIndex( targetFileIdx ), targetfile );

    rootFolder->removeRow( 1 );
    QCOMPARE( model->rowCount( folderIdx ), 1 );
    delete file;
    file = 0;

    // Check that we also find a folder with the fileName
    new ProjectFolderItem( fileName, rootFolder );
    QVERIFY( rootFolder->hasFileOrFolder( fileName ) );

    delete rootFolder;
    QCOMPARE( model->rowCount(), 0 );
}

void ProjectModelTest::testItemSanity()
{
    ProjectBaseItem* parent = new ProjectBaseItem( 0, "test" );
    ProjectBaseItem* child = new ProjectBaseItem( 0, "test", parent );
    ProjectBaseItem* child2 = new ProjectBaseItem( 0, "ztest", parent );
    ProjectFileItem* child3 = new ProjectFileItem( 0, Path(KUrl::fromPath("file:///bcd")), parent );
    ProjectFileItem* child4 = new ProjectFileItem(  0, Path(KUrl::fromPath("file:///abcd")), parent  );

    // Just some basic santiy checks on the API
    QCOMPARE( parent->child( 0 ), child );
    QCOMPARE( parent->row(), -1 );
    QVERIFY( !parent->child( -1 ) );
    QVERIFY( !parent->file() );
    QVERIFY( !parent->folder() );
    QVERIFY( !parent->project() );
    QVERIFY( !parent->child( parent->rowCount() ) );
    QCOMPARE( parent->iconName(), QString() );
    QCOMPARE( parent->index(), QModelIndex() );

    QCOMPARE( child->type(), (int)ProjectBaseItem::BaseItem );

    QCOMPARE( child->lessThan( child2 ), true );
    QCOMPARE( child3->lessThan( child4 ), false );

    // Check that model is properly emitting data-changes
    model->appendRow( parent );
    QCOMPARE( parent->index(), model->index(0, 0, QModelIndex()) );
    QSignalSpy s( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)) );
    parent->setPath( Path("/newtest") );
    QCOMPARE( s.count(), 1 );
    QCOMPARE( model->data( parent->index() ).toString(), QString("newtest") );

    parent->removeRow( child->row() );
}

void ProjectModelTest::testTakeRow()
{
    ProjectBaseItem* parent = new ProjectBaseItem( 0, "test" );
    ProjectBaseItem* child = new ProjectBaseItem( 0, "test", parent );
    ProjectBaseItem* subchild = new ProjectBaseItem( 0, "subtest", child );

    model->appendRow( parent );

    QCOMPARE( parent->model(), model );
    QCOMPARE( child->model(), model );
    QCOMPARE( subchild->model(), model );

    parent->takeRow( child->row() );

    QCOMPARE( child->model(), static_cast<ProjectModel*>(0) );
    QCOMPARE( subchild->model(), static_cast<ProjectModel*>(0) );
}

void ProjectModelTest::testRename()
{
    QFETCH( int, itemType );
    QFETCH( QString, itemText );
    QFETCH( QString, newName );
    QFETCH( bool, datachangesignal );
    QFETCH( QString, expectedItemText );
    QFETCH( int, expectedRenameCode );

    const Path projectFolder = Path(KUrl::fromPath("/dummyprojectfolder"));
    TestProject* proj = new TestProject;
    ProjectFolderItem* rootItem = new ProjectFolderItem( proj, projectFolder, 0);
    proj->setProjectItem( rootItem );

    new ProjectFileItem("existing", rootItem);

    ProjectBaseItem* item = 0;
    if( itemType == ProjectBaseItem::Target ) {
        item = new ProjectTargetItem( proj, itemText, rootItem );
    } else if( itemType == ProjectBaseItem::File ) {
        item = new ProjectFileItem( itemText, rootItem );
    } else if( itemType == ProjectBaseItem::Folder ) {
        item = new ProjectFolderItem( itemText, rootItem );
    } else if( itemType == ProjectBaseItem::BuildFolder ) {
        item = new ProjectBuildFolderItem( itemText, rootItem );
    }
    Q_ASSERT( item );

    QCOMPARE(item->model(), model);
    QSignalSpy s( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)) );
    ProjectBaseItem::RenameStatus stat = item->rename( newName );
    QCOMPARE( (int)stat, expectedRenameCode );
    if( datachangesignal ) {
        QCOMPARE( s.count(), 1 );
        QCOMPARE( qvariant_cast<QModelIndex>( s.takeFirst().at(0) ), item->index() );
    } else {
        QCOMPARE( s.count(), 0 );
    }
    QCOMPARE( item->text(), expectedItemText );
}

void ProjectModelTest::testRename_data()
{
    QTest::addColumn<int>( "itemType" );
    QTest::addColumn<QString>( "itemText" );
    QTest::addColumn<QString>( "newName" );
    QTest::addColumn<bool>( "datachangesignal" );
    QTest::addColumn<QString>( "expectedItemText" );
    QTest::addColumn<int>( "expectedRenameCode" );
    
    QTest::newRow("RenameableTarget")
    << (int)ProjectBaseItem::Target
    << QString::fromLatin1("target")
    << QString::fromLatin1("othertarget")
    << true
    << QString::fromLatin1("othertarget")
    << (int)ProjectBaseItem::RenameOk;
    
    QTest::newRow("RenameableFile")
    << (int)ProjectBaseItem::File
    << QString::fromLatin1("newfile.cpp")
    << QString::fromLatin1("otherfile.cpp")
    << true
    << QString::fromLatin1("otherfile.cpp")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("SourceAndDestinationFileEqual")
    << (int)ProjectBaseItem::File
    << QString::fromLatin1("newfile.cpp")
    << QString::fromLatin1("newfile.cpp")
    << false
    << QString::fromLatin1("newfile.cpp")
    << (int)ProjectBaseItem::RenameOk;
    
    QTest::newRow("RenameableFolder")
    << (int)ProjectBaseItem::Folder
    << QString::fromLatin1("newfolder")
    << QString::fromLatin1("otherfolder")
    << true
    << QString::fromLatin1("otherfolder")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("SourceAndDestinationFolderEqual")
    << (int)ProjectBaseItem::Folder
    << QString::fromLatin1("newfolder")
    << QString::fromLatin1("newfolder")
    << false
    << QString::fromLatin1("newfolder")
    << (int)ProjectBaseItem::RenameOk;
    
    QTest::newRow("RenameableBuildFolder")
    << (int)ProjectBaseItem::BuildFolder
    << QString::fromLatin1("newbfolder")
    << QString::fromLatin1("otherbfolder")
    << true
    << QString::fromLatin1("otherbfolder")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("SourceAndDestinationBuildFolderEqual")
    << (int)ProjectBaseItem::BuildFolder
    << QString::fromLatin1("newbfolder")
    << QString::fromLatin1("newbfolder")
    << false
    << QString::fromLatin1("newbfolder")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("ExistingFileError")
    << (int)ProjectBaseItem::Folder
    << QString::fromLatin1("mynew")
    << QString::fromLatin1("existing")
    << false
    << QString::fromLatin1("mynew")
    << (int)ProjectBaseItem::ExistingItemSameName;

    QTest::newRow("InvalidNameError")
    << (int)ProjectBaseItem::File
    << QString::fromLatin1("mynew")
    << QString::fromLatin1("other/bash")
    << false
    << QString::fromLatin1("mynew")
    << (int)ProjectBaseItem::InvalidNewName;
}

void ProjectModelTest::testWithProject()
{
    TestProject* proj = new TestProject();
    ProjectFolderItem* rootItem = new ProjectFolderItem( proj, Path(KUrl::fromPath("/dummyprojectfolder")), 0);
    proj->setProjectItem( rootItem );
    ProjectBaseItem* item = model->itemFromIndex( model->index( 0, 0 ) );
    QCOMPARE( item, rootItem );
    QCOMPARE( item->text(), proj->name() );
    QCOMPARE( item->path(), proj->path() );
}

void ProjectModelTest::testItemsForPath()
{
    QFETCH(Path, path);
    QFETCH(ProjectBaseItem*, root);
    QFETCH(int, matches);

    model->appendRow(root);

    QList< ProjectBaseItem* > items = model->itemsForPath(KDevelop::IndexedString(path.pathOrUrl()));
    QCOMPARE(items.size(), matches);
    foreach(ProjectBaseItem* item, items) {
        QVERIFY(item->path() == path);
    }

    model->clear();
}

void ProjectModelTest::testItemsForPath_data()
{
    QTest::addColumn<Path>("path");
    QTest::addColumn<ProjectBaseItem*>("root");
    QTest::addColumn<int>("matches");

    {
        ProjectFolderItem* root = new ProjectFolderItem(0, Path(KUrl::fromPath("/tmp/")));
        ProjectFileItem* file = new ProjectFileItem("a", root);
        QTest::newRow("find one") << file->path() << static_cast<ProjectBaseItem*>(root) << 1;
    }

    {
        ProjectFolderItem* root = new ProjectFolderItem(0, Path(KUrl::fromPath("/tmp/")));
        ProjectFolderItem* folder = new ProjectFolderItem("a", root);
        ProjectFileItem* file = new ProjectFileItem("foo", folder);
        ProjectTargetItem* target = new ProjectTargetItem(0, "b", root);
        ProjectFileItem* file2 = new ProjectFileItem(0, file->path(), target);
        Q_UNUSED(file2);
        QTest::newRow("find two") << file->path() << static_cast<ProjectBaseItem*>(root) << 2;
    }
}

void ProjectModelTest::testProjectProxyModel()
{
    ProjectFolderItem* root = new ProjectFolderItem(0, Path(KUrl::fromPath("/tmp/")));
    new ProjectFileItem("b1", root);
    new ProjectFileItem("a1", root);
    new ProjectFileItem("d1", root);
    new ProjectFileItem("c1", root);
    model->appendRow(root);

    QModelIndex proxyRoot = proxy->mapFromSource(root->index());
    QCOMPARE(model->rowCount(root->index()), 4);
    QCOMPARE(proxy->rowCount(proxyRoot), 4);
    QCOMPARE(proxy->index(0, 0, proxy->index(0, 0)).data().toString(), QString("a1"));
    QCOMPARE(proxy->index(1, 0, proxy->index(0, 0)).data().toString(), QString("b1"));
    QCOMPARE(proxy->index(2, 0, proxy->index(0, 0)).data().toString(), QString("c1"));
    QCOMPARE(proxy->index(3, 0, proxy->index(0, 0)).data().toString(), QString("d1"));

    model->clear();
}

void ProjectModelTest::testProjectFileSet()
{
    TestProject* project = new TestProject;

    QVERIFY(project->fileSet().isEmpty());
    Path path(KUrl::fromPath("/tmp/a"));
    ProjectFileItem* item = new ProjectFileItem(project, path, project->projectItem());
    QCOMPARE(project->fileSet().size(), 1);
    qDebug() << path << project->fileSet().begin()->toUrl();
    QCOMPARE(Path(project->fileSet().begin()->toUrl()), path);
    delete item;
    QVERIFY(project->fileSet().isEmpty());
}

void ProjectModelTest::testProjectFileIcon()
{
    ProjectFileItem* item = new ProjectFileItem(0, Path(KUrl::fromPath("/tmp/foo.txt")));
    const QString txtIcon = KMimeType::iconNameForUrl(item->path().toUrl());
    QCOMPARE(item->iconName(), txtIcon);
    item->setPath(Path(KUrl::fromPath("/tmp/bar.cpp")));
    QCOMPARE(item->iconName(), KMimeType::iconNameForUrl(item->path().toUrl()));
    QVERIFY(item->iconName() != txtIcon);
}

QTEST_KDEMAIN( ProjectModelTest, GUI)
