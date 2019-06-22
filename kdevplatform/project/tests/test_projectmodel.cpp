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

#include "test_projectmodel.h"

#include <QTest>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QMimeType>
#include <QMimeDatabase>
#include <QSignalSpy>

#include <projectmodel.h>
#include <projectproxymodel.h>
#include <tests/modeltest.h>
#include <tests/testproject.h>
#include <tests/kdevsignalspy.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <util/path.h>

using namespace KDevelop;

void debugItemModel(QAbstractItemModel* m, const QModelIndex& parent=QModelIndex(), int depth=0)
{
    Q_ASSERT(m);
    qDebug() << QByteArray(depth*2, '-') << m->data(parent).toString();
    for(int i=0; i<m->rowCount(parent); i++) {
        debugItemModel(m, m->index(i, 0, parent), depth+1);
    }
}

void TestProjectModel::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    qRegisterMetaType<QModelIndex>("QModelIndex");
    model = ICore::self()->projectController()->projectModel();
    new ModelTest( model, this );
    proxy = new ProjectProxyModel( model );
    new ModelTest(proxy, proxy);
    proxy->setSourceModel(model);
}

void TestProjectModel::init()
{
    model->clear();
}

void TestProjectModel::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestProjectModel::testCreateFileSystemItems()
{
    QFETCH( int, itemType );
    QFETCH( Path, itemPath );
    QFETCH( Path, expectedItemPath );
    QFETCH( QString, expectedItemText );
    QFETCH( QStringList, expectedRelativeItemPath );
    QFETCH( int, expectedItemRow );

    ProjectBaseItem* newitem = nullptr;
    switch( itemType ) {
        case ProjectBaseItem::Folder:
            newitem = new ProjectFolderItem( nullptr, itemPath );
            break;
        case ProjectBaseItem::BuildFolder:
            newitem = new ProjectBuildFolderItem( nullptr, itemPath );
            break;
        case ProjectBaseItem::File:
            newitem = new ProjectFileItem( nullptr, itemPath );
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

void TestProjectModel::testCreateFileSystemItems_data()
{
    QString testRootDir = QDir::rootPath() + QStringLiteral("rootdir");
    QString testRootFile = QDir::rootPath() + QStringLiteral("rootfile");
    QTest::addColumn<int>( "itemType" );
    QTest::addColumn<Path>( "itemPath" );
    QTest::addColumn<Path>( "expectedItemPath" );
    QTest::addColumn<QString>( "expectedItemText" );
    QTest::addColumn<QStringList>( "expectedRelativeItemPath" );
    QTest::addColumn<int>( "expectedItemRow" );

    QTest::newRow("RootFolder")
        << (int)ProjectBaseItem::Folder
        << Path(QUrl::fromLocalFile(testRootDir))
        << Path(QUrl::fromLocalFile(testRootDir))
        << QStringLiteral("rootdir")
        << ( QStringList() << QStringLiteral("rootdir") )
        << 0;

    QTest::newRow("RootBuildFolder")
        << (int)ProjectBaseItem::BuildFolder
        << Path(QUrl::fromLocalFile(testRootDir))
        << Path(QUrl::fromLocalFile(testRootDir))
        << QStringLiteral("rootdir")
        << ( QStringList() << QStringLiteral("rootdir") )
        << 0;

    QTest::newRow("RootFile")
        << (int)ProjectBaseItem::File
        << Path(QUrl::fromLocalFile(testRootFile))
        << Path(QUrl::fromLocalFile(testRootFile))
        << QStringLiteral("rootfile")
        << ( QStringList() << QStringLiteral("rootfile") )
        << 0;

}

void TestProjectModel::testCreateTargetItems()
{
    QFETCH( int, itemType );
    QFETCH( QString, itemText );
    QFETCH( QString, expectedItemText );
    QFETCH( QStringList, expectedItemPath );
    QFETCH( int, expectedItemRow );

    ProjectBaseItem* newitem = nullptr;
    switch( itemType ) {
        case ProjectBaseItem::Target:
            newitem = new ProjectTargetItem( nullptr, itemText );
            break;
        case ProjectBaseItem::LibraryTarget:
            newitem = new ProjectLibraryTargetItem( nullptr, itemText );
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

void TestProjectModel::testCreateTargetItems_data()
{
    QTest::addColumn<int>( "itemType" );
    QTest::addColumn<QString>( "itemText" );
    QTest::addColumn<QString>( "expectedItemText" );
    QTest::addColumn<QStringList>( "expectedItemPath" );
    QTest::addColumn<int>( "expectedItemRow" );

    QTest::newRow("RootTarget")
        << (int)ProjectBaseItem::Target
        << "target"
        << QStringLiteral("target")
        << ( QStringList() << QStringLiteral("target") )
        << 0;

    QTest::newRow("RootLibraryTarget")
        << (int)ProjectBaseItem::LibraryTarget
        << "libtarget"
        << QStringLiteral("libtarget")
        << ( QStringList() << QStringLiteral("libtarget") )
        << 0;
}

void TestProjectModel::testChangeWithProxyModel()
{
    QString projectFolderPath = QDir::rootPath() + QStringLiteral("folder1");
    QString projectFilePath = QDir::rootPath() + QStringLiteral("folder1/file1");
    auto* proxy = new QSortFilterProxyModel( this );
    proxy->setSourceModel( model );
    ProjectFolderItem* root = new ProjectFolderItem( nullptr, Path(QUrl::fromLocalFile(projectFolderPath)) );
    root->appendRow( new ProjectFileItem( nullptr, Path(QUrl::fromLocalFile(projectFilePath)) ) );
    model->appendRow( root );

    QCOMPARE( model->rowCount(), 1 );
    QCOMPARE( proxy->rowCount(), 1 );

    model->removeRow( 0 );

    QCOMPARE( model->rowCount(), 0 );
    QCOMPARE( proxy->rowCount(), 0 );
}

void TestProjectModel::testCreateSimpleHierarchy()
{
    QString folderName = QStringLiteral("rootfolder");
    QString fileName = QStringLiteral("file");
    QString targetName = QStringLiteral("testtarged");
    QString cppFileName = QStringLiteral("file.cpp");
    ProjectFolderItem* rootFolder = new ProjectFolderItem( nullptr, Path(QUrl::fromLocalFile( QDir::rootPath() + folderName )) );
    QCOMPARE(rootFolder->baseName(), folderName);
    auto* file = new ProjectFileItem( fileName, rootFolder );
    QCOMPARE(file->baseName(), fileName);
    auto* target = new ProjectTargetItem( nullptr, targetName );
    rootFolder->appendRow( target );
    ProjectFileItem* targetfile = new ProjectFileItem( nullptr, Path(rootFolder->path(), cppFileName), target );

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
    file = nullptr;

    // Check that we also find a folder with the fileName
    new ProjectFolderItem( fileName, rootFolder );
    QVERIFY( rootFolder->hasFileOrFolder( fileName ) );

    delete rootFolder;
    QCOMPARE( model->rowCount(), 0 );
}

void TestProjectModel::testItemSanity()
{
#ifdef Q_OS_WIN
    QString child3Path = QStringLiteral("file:///c:/bcd");
    QString child4Path = QStringLiteral("file:///c:/abcd");
#else
    QString child3Path = QStringLiteral("file:///bcd");
    QString child4Path = QStringLiteral("file:///abcd");
#endif
    QString newtestPath = QDir::rootPath() + QStringLiteral("newtest");
    ProjectBaseItem* parent = new ProjectBaseItem( nullptr, QStringLiteral("test") );
    ProjectBaseItem* child = new ProjectBaseItem( nullptr, QStringLiteral("test"), parent );
    ProjectBaseItem* child2 = new ProjectBaseItem( nullptr, QStringLiteral("ztest"), parent );
    ProjectFileItem* child3 = new ProjectFileItem( nullptr, Path(QUrl(child3Path)), parent );
    ProjectFileItem* child4 = new ProjectFileItem(  nullptr, Path(QUrl(child4Path)), parent  );

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
    parent->setPath( Path(newtestPath) );
    QCOMPARE( s.count(), 1 );
    QCOMPARE( model->data( parent->index() ).toString(), QStringLiteral("newtest") );

    parent->removeRow( child->row() );
}

void TestProjectModel::testTakeRow()
{
    QScopedPointer<ProjectBaseItem> parent(new ProjectBaseItem( nullptr, QStringLiteral("test") ));
    QScopedPointer<ProjectBaseItem> child(new ProjectBaseItem( nullptr, QStringLiteral("test"), parent.data() ));
    QScopedPointer<ProjectBaseItem> subchild(new ProjectBaseItem( nullptr, QStringLiteral("subtest"), child.data() ));

    model->appendRow( parent.data() );

    QCOMPARE( parent->model(), model );
    QCOMPARE( child->model(), model );
    QCOMPARE( subchild->model(), model );

    parent->takeRow( child->row() );

    QCOMPARE( child->model(), static_cast<ProjectModel*>(nullptr) );
    QCOMPARE( subchild->model(), static_cast<ProjectModel*>(nullptr) );
}

void TestProjectModel::testRename()
{
    QString projectFolderPath = QDir::rootPath() + QStringLiteral("dummyprojectfolder");
    QFETCH( int, itemType );
    QFETCH( QString, itemText );
    QFETCH( QString, newName );
    QFETCH( bool, datachangesignal );
    QFETCH( QString, expectedItemText );
    QFETCH( int, expectedRenameCode );

    const Path projectFolder = Path(QUrl::fromLocalFile(projectFolderPath));
    QScopedPointer<TestProject> proj(new TestProject());
    auto* rootItem = new ProjectFolderItem( proj.data(), projectFolder, nullptr);
    proj->setProjectItem( rootItem );

    new ProjectFileItem(QStringLiteral("existing"), rootItem);

    ProjectBaseItem* item = nullptr;
    if( itemType == ProjectBaseItem::Target ) {
        item = new ProjectTargetItem( proj.data(), itemText, rootItem );
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

void TestProjectModel::testRename_data()
{
    QTest::addColumn<int>( "itemType" );
    QTest::addColumn<QString>( "itemText" );
    QTest::addColumn<QString>( "newName" );
    QTest::addColumn<bool>( "datachangesignal" );
    QTest::addColumn<QString>( "expectedItemText" );
    QTest::addColumn<int>( "expectedRenameCode" );

    QTest::newRow("RenameableTarget")
    << (int)ProjectBaseItem::Target
    << QStringLiteral("target")
    << QStringLiteral("othertarget")
    << true
    << QStringLiteral("othertarget")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("RenameableFile")
    << (int)ProjectBaseItem::File
    << QStringLiteral("newfile.cpp")
    << QStringLiteral("otherfile.cpp")
    << true
    << QStringLiteral("otherfile.cpp")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("SourceAndDestinationFileEqual")
    << (int)ProjectBaseItem::File
    << QStringLiteral("newfile.cpp")
    << QStringLiteral("newfile.cpp")
    << false
    << QStringLiteral("newfile.cpp")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("RenameableFolder")
    << (int)ProjectBaseItem::Folder
    << QStringLiteral("newfolder")
    << QStringLiteral("otherfolder")
    << true
    << QStringLiteral("otherfolder")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("SourceAndDestinationFolderEqual")
    << (int)ProjectBaseItem::Folder
    << QStringLiteral("newfolder")
    << QStringLiteral("newfolder")
    << false
    << QStringLiteral("newfolder")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("RenameableBuildFolder")
    << (int)ProjectBaseItem::BuildFolder
    << QStringLiteral("newbfolder")
    << QStringLiteral("otherbfolder")
    << true
    << QStringLiteral("otherbfolder")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("SourceAndDestinationBuildFolderEqual")
    << (int)ProjectBaseItem::BuildFolder
    << QStringLiteral("newbfolder")
    << QStringLiteral("newbfolder")
    << false
    << QStringLiteral("newbfolder")
    << (int)ProjectBaseItem::RenameOk;

    QTest::newRow("ExistingFileError")
    << (int)ProjectBaseItem::Folder
    << QStringLiteral("mynew")
    << QStringLiteral("existing")
    << false
    << QStringLiteral("mynew")
    << (int)ProjectBaseItem::ExistingItemSameName;

    QTest::newRow("InvalidNameError")
    << (int)ProjectBaseItem::File
    << QStringLiteral("mynew")
    << QStringLiteral("other/bash")
    << false
    << QStringLiteral("mynew")
    << (int)ProjectBaseItem::InvalidNewName;
}

void TestProjectModel::testWithProject()
{
    QString projectFolderPath = QDir::rootPath() + QStringLiteral("dummyprojectfolder");
    QScopedPointer<TestProject> proj(new TestProject());
    ProjectFolderItem* rootItem = new ProjectFolderItem( proj.data(), Path(QUrl::fromLocalFile(projectFolderPath)), nullptr);
    proj->setProjectItem( rootItem );
    ProjectBaseItem* item = model->itemFromIndex( model->index( 0, 0 ) );
    QCOMPARE( item, rootItem );
    QCOMPARE( item->text(), proj->name() );
    QCOMPARE( item->path(), proj->path() );
}

void TestProjectModel::testItemsForPath()
{
    QFETCH(Path, path);
    QFETCH(ProjectBaseItem*, root);
    QFETCH(int, matches);

    model->appendRow(root);

    const auto items = model->itemsForPath(IndexedString(path.pathOrUrl()));
    QCOMPARE(items.size(), matches);
    for (ProjectBaseItem* item : items) {
        QVERIFY(item->path() == path);
    }

    model->clear();
}

void TestProjectModel::testItemsForPath_data()
{
    QTest::addColumn<Path>("path");
    QTest::addColumn<ProjectBaseItem*>("root");
    QTest::addColumn<int>("matches");

    {
        ProjectFolderItem* root = new ProjectFolderItem(nullptr, Path(QUrl::fromLocalFile(QDir::tempPath())));
        ProjectFileItem* file = new ProjectFileItem(QStringLiteral("a"), root);
        QTest::newRow("find one") << file->path() << static_cast<ProjectBaseItem*>(root) << 1;
    }

    {
        ProjectFolderItem* root = new ProjectFolderItem(nullptr, Path(QUrl::fromLocalFile(QDir::tempPath())));
        ProjectFolderItem* folder = new ProjectFolderItem(QStringLiteral("a"), root);
        ProjectFileItem* file = new ProjectFileItem(QStringLiteral("foo"), folder);
        ProjectTargetItem* target = new ProjectTargetItem(nullptr, QStringLiteral("b"), root);
        ProjectFileItem* file2 = new ProjectFileItem(nullptr, file->path(), target);
        Q_UNUSED(file2);
        QTest::newRow("find two") << file->path() << static_cast<ProjectBaseItem*>(root) << 2;
    }
}

void TestProjectModel::testProjectProxyModel()
{
    ProjectFolderItem* root = new ProjectFolderItem(nullptr, Path(QUrl::fromLocalFile(QDir::tempPath())));
    new ProjectFileItem(QStringLiteral("b1"), root);
    new ProjectFileItem(QStringLiteral("a1"), root);
    new ProjectFileItem(QStringLiteral("d1"), root);
    new ProjectFileItem(QStringLiteral("c1"), root);
    model->appendRow(root);

    QModelIndex proxyRoot = proxy->mapFromSource(root->index());
    QCOMPARE(model->rowCount(root->index()), 4);
    QCOMPARE(proxy->rowCount(proxyRoot), 4);
    QCOMPARE(proxy->index(0, 0, proxy->index(0, 0)).data().toString(), QStringLiteral("a1"));
    QCOMPARE(proxy->index(1, 0, proxy->index(0, 0)).data().toString(), QStringLiteral("b1"));
    QCOMPARE(proxy->index(2, 0, proxy->index(0, 0)).data().toString(), QStringLiteral("c1"));
    QCOMPARE(proxy->index(3, 0, proxy->index(0, 0)).data().toString(), QStringLiteral("d1"));

    model->clear();
}

void TestProjectModel::testProjectFileSet()
{
    QScopedPointer<TestProject> project(new TestProject());

    QVERIFY(project->fileSet().isEmpty());
    Path path(QUrl::fromLocalFile(QDir::tempPath() + "/a"));
    auto* item = new ProjectFileItem(project.data(), path, project->projectItem());
    QCOMPARE(project->fileSet().size(), 1);
    qDebug() << path << project->fileSet().toList().at(0).toUrl();
    QCOMPARE(Path(project->fileSet().toList().at(0).toUrl()), path);
    delete item;
    QVERIFY(project->fileSet().isEmpty());
}

void TestProjectModel::testProjectFileIcon()
{
    QMimeDatabase db;

    QScopedPointer<ProjectFileItem> item(new ProjectFileItem(nullptr, Path(QDir::tempPath() + "/foo.txt")));
    const QString txtIcon = db.mimeTypeForUrl(item->path().toUrl()).iconName();
    QCOMPARE(item->iconName(), txtIcon);
    item->setPath(Path(QDir::tempPath() + "/bar.cpp"));
    QCOMPARE(item->iconName(), db.mimeTypeForUrl(item->path().toUrl()).iconName());
    QVERIFY(item->iconName() != txtIcon);
}

QTEST_MAIN(TestProjectModel)
