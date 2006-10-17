/***************************************************************************
*   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
*   jsgaarde@tdcspace.dk                                                  *
*   Copyright (C) 2002-2003 by Alexander Dymo                             *
*   cloudtemple@mksat.net                                                 *
*   Copyright (C) 2003 by Thomas Hasart                                   *
*   thasart@gmx.de                                                        *
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "projectconfigurationdlg.h"
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <kfiledialog.h>
#include <knotifyclient.h>
#include <klineedit.h>
#include <kinputdialog.h>
#include <kurlrequesterdlg.h>
#include <kurlrequester.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klistview.h>

#include <qdialog.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlistview.h>
#include <qlabel.h>
#include <iostream>
#include <qregexp.h>
#include <qvalidator.h>
#include <qtabwidget.h>
#include <pathutil.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "trollprojectwidget.h"
#include "trollprojectpart.h"
#include "qmakescopeitem.h"
#include "scope.h"

InsideCheckListItem::InsideCheckListItem( QListView *parent, QMakeScopeItem *item, ProjectConfigurationDlg *config ) :
        QCheckListItem( parent, item->relativePath().right( item->relativePath().length() - 1 ), QCheckListItem::CheckBox )
{
    prjItem = item;
    m_config = config;
}

InsideCheckListItem::InsideCheckListItem( QListView *parent, QListViewItem *after, QMakeScopeItem *item, ProjectConfigurationDlg *config ) :
        QCheckListItem( parent,
                        after,
                        item->relativePath().right( item->relativePath().length() - 1 ), QCheckListItem::CheckBox )
{
    prjItem = item;
    m_config = config;
}

//check or uncheck dependency to currently checked or unchecked library
void InsideCheckListItem::stateChange( bool state )
{
    if ( listView() == m_config->insidelib_listview )
    {
        QListViewItemIterator it( m_config->intDeps_view );
        while ( it.current() )
        {
            InsideCheckListItem * chi = dynamic_cast<InsideCheckListItem*>( it.current() );
            if ( chi )
                if ( chi->prjItem == prjItem )
                    chi->setOn( state );
            ++it;
        }
    }
}

ProjectConfigurationDlg::ProjectConfigurationDlg( QListView *_prjList, TrollProjectWidget* _prjWidget, QWidget* parent, const char* name, bool modal, WFlags fl )
        : ProjectConfigurationDlgBase( parent, name, modal, fl | Qt::WStyle_Tool ), myProjectItem(0)
{
    prjList = _prjList;
    prjWidget = _prjWidget;
    //  m_projectConfiguration = conf;
    m_targetLibraryVersion->setValidator( new QRegExpValidator(
                                              QRegExp( "\\d+(\\.\\d+)?(\\.\\d+)" ), this ) );

}

void ProjectConfigurationDlg::updateSubproject( QMakeScopeItem* _item )
{
    if ( myProjectItem && myProjectItem->scope )
    {
        switch ( prjWidget->dialogSaveBehaviour() )
        {
            case TrollProjectWidget::AlwaysSave:
                apply();
                break;
            case TrollProjectWidget::NeverSave:
                break;
            case TrollProjectWidget::Ask:
                if ( !buttonApply->isEnabled() )
                    break;
                if ( KMessageBox::questionYesNo( 0, i18n( "Save the current subprojects configuration?" ),
                                                 i18n( "Save Configuration?" ) ) == KMessageBox::Yes )
                    apply();
                break;
        }
    }
    myProjectItem = _item;
    updateControls();
    buttonApply->setEnabled( false );
}

ProjectConfigurationDlg::~ProjectConfigurationDlg()
{}

void ProjectConfigurationDlg::browseTargetPath()
{
    m_targetPath->setText( getRelativePath( myProjectItem->scope->projectDir(), KFileDialog::getExistingDirectory() ) );
    buttonApply->setEnabled( false );
}

void ProjectConfigurationDlg::updateProjectConfiguration()
{
    // Template
    if ( myProjectItem->scope->scopeType() == Scope::ProjectScope )
    {
        if ( radioApplication->isChecked() )
        {
            myProjectItem->scope->setEqualOp( "TEMPLATE", "app" );
            myProjectItem->setPixmap( 0, SmallIcon( "qmake_app" ) );
        }
        else if ( radioLibrary->isChecked() )
        {
            myProjectItem->scope->setEqualOp( "TEMPLATE", "lib" );
            if ( staticRadio->isOn() )
                myProjectItem->addValue( "CONFIG", "staticlib" );
            else if ( myProjectItem->scope->variableValues( "CONFIG" ).contains( "staticlib" ) )
                myProjectItem->removeValue( "CONFIG", "staticlib" );
            if ( sharedRadio->isOn() )
            {
                myProjectItem->addValue( "CONFIG", "dll" );
                myProjectItem->scope->setEqualOp( "VERSION", m_targetLibraryVersion->text() );
            }
            else
            {
                myProjectItem->removeValue( "CONFIG", "dll" );
                if ( !myProjectItem->scope->listIsEmpty( myProjectItem->scope->variableValues( "VERSION" ) ) )
                {
                    myProjectItem->scope->removeVariable( "VERSION", "=" );
                    myProjectItem->scope->removeVariable( "VERSION", "+=" );
                    myProjectItem->scope->removeVariable( "VERSION", "-=" );
                }
            }
            if ( checkPlugin->isOn() )
                myProjectItem->addValue( "CONFIG", "plugin" );
            else
                myProjectItem->removeValue( "CONFIG", "plugin" );
            if ( checkDesigner->isChecked() )
                myProjectItem->addValue( "CONFIG", "designer" );
            else
                myProjectItem->removeValue( "CONFIG", "designer" );

            myProjectItem->setPixmap( 0, SmallIcon( "qmake_lib" ) );
        }
        else if ( radioSubdirs->isChecked() )
        {
            myProjectItem->scope->setEqualOp( "TEMPLATE", "subdirs" );
            myProjectItem->setPixmap( 0, SmallIcon( "qmake_sub" ) );
        }
    }

    // Buildmode
    if ( radioDebugMode->isChecked() )
        myProjectItem->addValue( "CONFIG", "debug" );
    else
        myProjectItem->removeValue( "CONFIG", "debug" );
    if ( radioReleaseMode->isChecked() )
        myProjectItem->addValue( "CONFIG", "release" );
    else
        myProjectItem->removeValue( "CONFIG", "release" );

    // requirements
    if ( exceptionCheck->isChecked() )
        myProjectItem->addValue( "CONFIG", "exceptions" );
    else
        myProjectItem->removeValue( "CONFIG", "exceptions" );
    if ( stlCheck->isChecked() )
        myProjectItem->addValue( "CONFIG", "stl" );
    else
        myProjectItem->removeValue( "CONFIG", "stl" );
    if ( rttiCheck->isChecked() )
        myProjectItem->addValue( "CONFIG", "rtti" );
    else
        myProjectItem->removeValue( "CONFIG", "rtti" );
    if ( checkQt->isChecked() )
        myProjectItem->addValue( "CONFIG", "qt" );
    else
        myProjectItem->removeValue( "CONFIG", "qt" );
    if ( checkOpenGL->isChecked() )
        myProjectItem->addValue( "CONFIG", "opengl" );
    else
        myProjectItem->removeValue( "CONFIG", "opengl" );
    if ( checkThread->isChecked() )
        myProjectItem->addValue( "CONFIG", "thread" );
    else
        myProjectItem->removeValue( "CONFIG", "thread" );
    if ( checkX11->isChecked() )
        myProjectItem->addValue( "CONFIG", "x11" );
    else
        myProjectItem->removeValue( "CONFIG", "x11" );
    if ( checkOrdered->isChecked() )
        myProjectItem->addValue( "CONFIG", "ordered" );
    else
        myProjectItem->removeValue( "CONFIG", "ordered" );
    if ( checkLibtool->isChecked() )
        myProjectItem->addValue( "CONFIG", "create_libtool" );
    else
        myProjectItem->removeValue( "CONFIG", "create_libtool" );
    if ( checkPkgconf->isChecked() )
        myProjectItem->addValue( "CONFIG", "create_pkgconf" );
    else
        myProjectItem->removeValue( "CONFIG", "create_pkgconf" );
    if ( checkConsole->isChecked() )
        myProjectItem->addValue( "CONFIG", "console" );
    else
        myProjectItem->removeValue( "CONFIG", "console" );
    if ( checkPCH->isChecked() )
        myProjectItem->addValue( "CONFIG", "precompile_header" );
    else
        myProjectItem->removeValue( "CONFIG", "precompile_header" );
    // Warnings
    if ( checkWarning->isChecked() )
    {
        myProjectItem->addValue( "CONFIG", "warn_on" );
        myProjectItem->removeValue( "CONFIG", "warn_off" );
    }
    else
    {
        myProjectItem->addValue( "CONFIG", "warn_on" );
        myProjectItem->removeValue( "CONFIG", "warn_off" );
    }
    if ( checkWindows->isChecked() )
        myProjectItem->addValue( "CONFIG", "windows" );
    else
        myProjectItem->removeValue( "CONFIG", "windows" );

    //Qt4 libs
    if ( prjWidget->m_part->isQt4Project() )
    {
        if ( radioDebugReleaseMode->isChecked() )
            myProjectItem->addValue( "CONFIG", "debug_and_release" );
        else
            myProjectItem->removeValue( "CONFIG", "debug_and_release" );

        if ( checkTestlib->isChecked() )
            myProjectItem->addValue( "CONFIG", "qtestlib" );
        else
            myProjectItem->removeValue( "CONFIG", "qtestlib" );
        if ( checkAssistant->isChecked() )
            myProjectItem->addValue( "CONFIG", "assistant" );
        else
            myProjectItem->removeValue( "CONFIG", "assistant" );
        if ( checkUiTools->isChecked() )
            myProjectItem->addValue( "CONFIG", "uitools" );
        else
            myProjectItem->removeValue( "CONFIG", "uitools" );
        if ( checkQDBus->isChecked() )
            myProjectItem->addValue( "CONFIG", "dbus" );
        else
            myProjectItem->removeValue( "CONFIG", "dbus" );
        if ( checkBuildAll->isChecked() )
            myProjectItem->addValue( "CONFIG", "build_all" );
        else
            myProjectItem->removeValue( "CONFIG", "build_all" );

        if ( checkQt4Core->isChecked() )
            myProjectItem->addValue( "QT", "core" );
        else
            myProjectItem->removeValue( "QT", "core" );
        if ( checkQt4Gui->isChecked() )
            myProjectItem->addValue( "QT", "gui" );
        else
            myProjectItem->removeValue( "QT", "gui" );
        if ( checkQt4SQL->isChecked() )
            myProjectItem->addValue( "QT", "sql" );
        else
            myProjectItem->removeValue( "QT", "sql" );
        if ( checkQt4SVG->isChecked() )
            myProjectItem->addValue( "QT", "svg" );
        else
            myProjectItem->removeValue( "QT", "svg" );
        if ( checkQt4XML->isChecked() )
            myProjectItem->addValue( "QT", "xml" );
        else
            myProjectItem->removeValue( "QT", "xml" );
        if ( checkQt4Network->isChecked() )
            myProjectItem->addValue( "QT", "network" );
        else
            myProjectItem->removeValue( "QT", "network" );
        if ( checkQt3Support->isChecked() )
            myProjectItem->addValue( "QT", "qt3support" );
        else
            myProjectItem->removeValue( "QT", "qt3support" );
        if ( checkQt4OpenGL->isChecked() )
            myProjectItem->addValue( "QT", "opengl" );
        else
            myProjectItem->removeValue( "QT", "opengl" );
    }

    QStringList confValues = myProjectItem->scope->variableValues( "CONFIG" );
    QStringList extraValues = QStringList::split( " ", editConfigExtra->text() );
    for ( QStringList::iterator it = confValues.begin() ; it != confValues.end() ; ++it )
    {
        if ( !Scope::KnownConfigValues.contains( *it ) && !extraValues.contains( *it ) )
        {
            myProjectItem->scope->addToMinusOp( "CONFIG", *it );
        }
    }

    for ( QStringList::iterator it = extraValues.begin() ; it != extraValues.end() ; ++it )
    {
        if ( !confValues.contains( *it ) )
            myProjectItem->scope->addToPlusOp( "CONFIG", *it );
    }

    myProjectItem->scope->setEqualOp( "TARGET", QStringList( m_targetOutputFile->text() ) );
    myProjectItem->scope->setEqualOp( "DESTDIR", QStringList( m_targetPath->text() ) );

    myProjectItem->updateValues( "DEFINES", QStringList::split( " ", m_defines->text() ) );
    myProjectItem->updateValues( "QMAKE_CXXFLAGS_DEBUG", QStringList::split( " ", m_debugFlags->text() ) );
    myProjectItem->updateValues( "QMAKE_CXXFLAGS_RELEASE", QStringList::split( " ", m_releaseFlags->text() ) );

    //add selected includes
    QStringList values;
    InsideCheckListItem *insideItem = ( InsideCheckListItem * ) insideinc_listview->firstChild();
    while ( insideItem )
    {
        if ( insideItem->isOn() )
        {
            QString tmpInc = insideItem->prjItem->getIncAddPath(
                myProjectItem->scope->projectDir() );
            tmpInc = QDir::cleanDirPath( tmpInc );
            values << tmpInc;

        }
        insideItem = ( InsideCheckListItem* ) insideItem->itemBelow();
    }

    QCheckListItem *outsideItem = ( QCheckListItem * ) outsideinc_listview->firstChild();
    while ( outsideItem )
    {
        values << outsideItem->text( 0 );
        outsideItem = ( QCheckListItem* ) outsideItem->itemBelow();
    }
    myProjectItem->updateValues( "INCLUDEPATH", values );

    //target.install
    if ( checkInstallTarget->isChecked() == true )
    {
        myProjectItem->addValue( "INSTALLS", "target" );
        myProjectItem->scope->setEqualOp( "target.path", QStringList( m_InstallTargetPath->text() ) );
    }
    else
    {
        myProjectItem->removeValue( "INSTALLS", "target" );
        myProjectItem->scope->removeVariable( "target.path", "=" );
    }

    //makefile
    myProjectItem->scope->setEqualOp( "MAKEFILE", QStringList( makefile_url->url() ) );

    //add libs to link

    values.clear();
    QStringList libPaths;

    //inside libs to link
    insideItem = ( InsideCheckListItem * ) insidelib_listview->firstChild();
    while ( insideItem )
    {
        if ( insideItem->isOn() )
        {

            QString tmpLib = insideItem->prjItem->getLibAddObject( myProjectItem->scope->projectDir() );
            if ( insideItem->prjItem->scope->variableValues( "CONFIG" ).contains( "dll" ) )
            {
                //add path if shared lib is linked
                QString tmpPath = insideItem->prjItem->getLibAddPath(
                    myProjectItem->scope->projectDir() );
                if ( tmpPath != "" )
                {
                    values << ("-L"+tmpPath) ;
                }
            }
            values << tmpLib ;

        }
        insideItem = ( InsideCheckListItem* ) insideItem->itemBelow();
    }

    //extra lib paths
    QListViewItem *lvItem = outsidelibdir_listview->firstChild();
    while ( lvItem )
    {
        values << ( "-L"+lvItem->text( 0 ) );
        lvItem = lvItem->itemBelow();
    }


    //outside libs to link
    outsideItem = ( QCheckListItem * ) outsidelib_listview->firstChild();
    while ( outsideItem )
    {
        values << outsideItem->text( 0 );
        outsideItem = ( QCheckListItem* ) outsideItem->itemBelow();
    }


    myProjectItem->updateValues( "LIBS", values );

    values.clear();

    //external project dependencies
    QListViewItem *depItem = extDeps_view->firstChild();
    while ( depItem )
    {
        values << depItem->text( 0 );
        depItem = depItem->itemBelow();
    }

    //internal project dependencies
    insideItem = dynamic_cast<InsideCheckListItem *>( intDeps_view->firstChild() );
    while ( insideItem )
    {
        if ( insideItem->isOn() )
        {
            if ( insideItem->prjItem->scope->variableValues( "CONFIG" ).contains( "staticlib" ) )
            {
                values << insideItem->prjItem->getLibAddObject(
                    myProjectItem->scope->projectDir() );
            }
            else if ( insideItem->prjItem->scope->variableValues( "CONFIG" ).contains( "dll" ) )
            {
                values << insideItem->prjItem->getSharedLibAddObject(
                    myProjectItem->scope->projectDir() );
            }
            else
            {
                values << insideItem->prjItem->getApplicationObject(
                    myProjectItem->scope->projectDir() );
            }
        }
        insideItem = dynamic_cast<InsideCheckListItem *>( insideItem->itemBelow() );
    }
    myProjectItem->updateValues( "TARGETDEPS", values );

    values.clear();
    //change build order
    lvItem = buildorder_listview->firstChild();
    if ( lvItem && lvItem->itemBelow() && myProjectItem->scope->variableValues("TEMPLATE").contains("subdirs") )
    {

        while ( lvItem )
        {
            values << lvItem->text( 0 );
            lvItem = lvItem->itemBelow();
        }

        if ( myProjectItem->scope->isVariableReset( "SUBDIRS" ) )
        {
            myProjectItem->scope->removeVariable( "SUBDIRS", "=" );
            myProjectItem->scope->setEqualOp( "SUBDIRS", values );
        }
        else
        {
            myProjectItem->scope->removeVariable( "SUBDIRS", "+=" );
            myProjectItem->scope->setPlusOp( "SUBDIRS", values );
        }
    }

    // intermediate locations
    myProjectItem->scope->setEqualOp( "OBJECTS_DIR", QStringList( objdir_url->url() ) );
    myProjectItem->scope->setEqualOp( "UI_DIR", QStringList( uidir_url->url() ) );
    myProjectItem->scope->setEqualOp( "MOC_DIR", QStringList( mocdir_url->url() ) );

    //CORBA
    myProjectItem->scope->setEqualOp( "IDL_COMPILER", QStringList( idlCmdEdit->url() ) );
    myProjectItem->updateValues( "IDL_OPTIONS", QStringList::split( " ", idlCmdOptionsEdit->text() ) );

    // custom vars
    QListViewItem *item = customVariables->firstChild();
    for ( ; item; item = item->nextSibling() )
    {
        QString var = item->text( 0 );
        QString op = item->text( 1 );
        if ( !myProjectItem->scope->customVariables().keys().contains( qMakePair( var, op ) ) )
            myProjectItem->scope->removeVariable( var, op );
        else
            myProjectItem->scope->updateCustomVariable( var, op, QStringList::split( " ", item->text( 2 ) ) );
    }
}

void ProjectConfigurationDlg::accept()
{
    apply();
    myProjectItem = 0;
    QDialog::accept();
}

void ProjectConfigurationDlg::reject()
{
    myProjectItem = 0;
    QDialog::reject();
}



void ProjectConfigurationDlg::updateControls()
{
    // Project template
    groupLibraries->setEnabled( false );
    if( myProjectItem->scope->scopeType() != Scope::ProjectScope )
        groupTemplate->setEnabled(false);

    //cache the value of the some variables
    QStringList configValues = myProjectItem->scope->variableValues( "CONFIG" );
    QStringList templateValues = myProjectItem->scope->variableValues( "TEMPLATE" );
    //if( !myProjectItem->isScope )
    //{
    if ( templateValues.contains( "lib" ) )
    {
        groupLibraries->setEnabled( true );

        radioLibrary->setChecked( true );
        staticRadio->setChecked( true ); //default

        if ( configValues.contains( "staticlib" ) )
        {
            staticRadio->setChecked( true );
        }
        else
            staticRadio->setChecked( false );
        if ( configValues.contains( "dll" ) )
        {
            sharedRadio->setChecked( true );
            m_targetLibraryVersion->setText( myProjectItem->scope->variableValues( "VERSION" ).front() );
        }
        else
        {
            sharedRadio->setChecked( false );
            m_targetLibraryVersion->setText( "" );
        }

        if ( configValues.contains( "plugin" ) )
            checkPlugin->setChecked( true );
        else
            checkPlugin->setChecked( false );
        if ( configValues.contains( "designer" ) )
            checkDesigner->setChecked( true );
        else
            checkDesigner->setChecked( false );
        if ( configValues.contains( "create_libtool" ) )
            checkLibtool->setChecked( true );
        else
            checkLibtool->setChecked( false );
        if ( configValues.contains( "create_pkgconf" ) )
            checkPkgconf->setChecked( true );
        else
            checkPkgconf->setChecked( false );
        groupTemplateChanged(1);
    }
    else if ( templateValues.contains( "subdirs" ) )
    {
        radioSubdirs->setChecked( true );
        groupTemplateChanged(2);
    }else
    {
        //Default is app mode
        radioApplication->setChecked( true );
        if ( configValues.contains( "console" ) )
        {
            checkConsole->setChecked( true );
        }
        groupTemplateChanged(0);
    }

    // Buildmode
    if ( configValues.contains( "debug" ) )
    {
        radioDebugMode->setChecked( true );
    }
    else if ( configValues.contains( "release" ) )
    {
        radioReleaseMode->setChecked( true );
    }
    else if ( configValues.contains( "debug_and_release" ) )
    {
        radioDebugReleaseMode->setChecked( true );
    }

    // Requirements
    if ( configValues.contains( "qt" ) )
        checkQt->setChecked( true );
    else
        checkQt->setChecked( false );
    if ( configValues.contains( "opengl" ) )
        checkOpenGL->setChecked( true );
    else
        checkOpenGL->setChecked( false );
    if ( configValues.contains( "thread" ) )
        checkThread->setChecked( true );
    else
        checkThread->setChecked( false );
    if ( configValues.contains( "x11" ) )
        checkX11->setChecked( true );
    else
        checkX11->setChecked( false );
    if ( configValues.contains( "ordered" ) )
        checkOrdered->setChecked( true );
    else
        checkOrdered->setChecked( false );
    if ( configValues.contains( "exceptions" ) )
        exceptionCheck->setChecked( true );
    else
        exceptionCheck->setChecked( false );
    if ( configValues.contains( "stl" ) )
        stlCheck->setChecked( true );
    else
        stlCheck->setChecked( false );
    if ( configValues.contains( "rtti" ) )
        rttiCheck->setChecked( true );
    else
        rttiCheck->setChecked( false );
    if ( configValues.contains( "precompile_header" ) )
        checkPCH->setChecked( true );
    else
        checkPCH->setChecked( false );
    // Warnings
    if ( configValues.contains( "warn_on" ) )
    {
        checkWarning->setChecked( true );
    }
    if ( configValues.contains( "warn_off" ) )
    {
        checkWarning->setChecked( false );
    }

    if ( configValues.contains( "windows" ) )
        checkWindows->setChecked( true );
    else
        checkWindows->setChecked( false );

    //Qt4 libs
    if ( prjWidget->m_part->isQt4Project() )
    {

        if ( configValues.contains( "assistant" ) )
            checkAssistant->setChecked( true );
        else
            checkAssistant->setChecked( false );
        if ( configValues.contains( "qtestlib" ) )
            checkTestlib->setChecked( true );
        else
            checkTestlib->setChecked( false );
        if ( configValues.contains( "uitools" ) )
            checkUiTools->setChecked( true );
        else
            checkUiTools->setChecked( false );
        if ( configValues.contains( "dbus" ) )
            checkQDBus->setChecked( true );
        else
            checkQDBus->setChecked( false );
        if ( configValues.contains( "build_all" ) )
            checkBuildAll->setChecked( true );
        else
            checkBuildAll->setChecked( false );

        QStringList qtLibs = myProjectItem->scope->variableValues( "QT" );
        if ( qtLibs.contains( "core" ) )
            checkQt4Core->setChecked( true );
        else
            checkQt4Core->setChecked( false );
        if ( qtLibs.contains( "gui" ) )
            checkQt4Gui->setChecked( true );
        else
            checkQt4Gui->setChecked( false );
        if ( qtLibs.contains( "sql" ) )
            checkQt4SQL->setChecked( true );
        else
            checkQt4SQL->setChecked( false );
        if ( qtLibs.contains( "xml" ) )
            checkQt4XML->setChecked( true );
        else
            checkQt4XML->setChecked( false );
        if ( qtLibs.contains( "network" ) )
            checkQt4Network->setChecked( true );
        else
            checkQt4Network->setChecked( false );
        if ( qtLibs.contains( "svg" ) )
            checkQt4SVG->setChecked( true );
        else
            checkQt4SVG->setChecked( false );
        if ( qtLibs.contains( "opengl" ) )
            checkQt4OpenGL->setChecked( true );
        else
            checkQt4OpenGL->setChecked( false );
        if ( qtLibs.contains( "qt3support" ) )
            checkQt3Support->setChecked( true );
        else
            checkQt3Support->setChecked( false );

        radioDebugReleaseMode->setEnabled( true );
        checkBuildAll->setEnabled( true );
        groupQt4Libs->setEnabled( checkQt->isChecked() );
        rccdir_url->setEnabled( true );
        rccdir_label->setEnabled( true );
    }

    //fill the custom config edit
    QStringList extraValues;
    for ( QStringList::const_iterator it = configValues.begin() ; it != configValues.end() ; ++it )
    {
        if ( !Scope::KnownConfigValues.contains( *it ) )
        {
            extraValues << *it;
        }
    }
    editConfigExtra->setText( extraValues.join( " " ) );

    //makefile
    makefile_url->setURL( myProjectItem->scope->variableValues( "MAKEFILE" ).front() );

    if ( myProjectItem->scope->variableValues( "INSTALL" ).contains( "target" ) )
    {
        checkInstallTarget->setChecked( true );
        m_InstallTargetPath->setEnabled( true );
    }
    else
    {
        checkInstallTarget->setChecked( false );
        m_InstallTargetPath->setEnabled( false );
    }
    m_InstallTargetPath->setText( myProjectItem->scope->variableValues( "target.path" ).front() );

    m_targetOutputFile->setText( myProjectItem->scope->variableValues( "TARGET" ).front() );
    m_targetPath->setText( myProjectItem->scope->variableValues( "DESTDIR" ).front() );

    m_defines->setText( myProjectItem->scope->variableValues( "DEFINES" ).join( " " ) );
    m_debugFlags->setText( myProjectItem->scope->variableValues( "QMAKE_CXXFLAGS_DEBUG" ).join( " " ) );
    m_releaseFlags->setText( myProjectItem->scope->variableValues( "QMAKE_CXXFLAGS_RELEASE" ).join( " " ) );

    updateIncludeControl();
    updateLibControls();
    updateBuildOrderControl();
    updateDependenciesControl();

    objdir_url->setURL( myProjectItem->scope->variableValues( "OBJECTS_DIR" ).front() );
    uidir_url->setURL( myProjectItem->scope->variableValues( "UI_DIR" ).front() );
    mocdir_url->setURL( myProjectItem->scope->variableValues( "MOC_DIR" ).front() );

    idlCmdEdit->setURL( myProjectItem->scope->variableValues( "IDL_COMPILER" ).front() );
    idlCmdOptionsEdit->setText( myProjectItem->scope->variableValues( "IDL_OPTIONS" ).join( " " ) );

    customVariables->clear();
    QMap<QPair<QString, QString>, QStringList > customvars = myProjectItem->scope->customVariables();
    QMap<QPair<QString, QString>, QStringList >::iterator idx = customvars.begin();
    for ( ; idx != customvars.end(); ++idx )
        new KListViewItem( customVariables, idx.key().first, idx.key().second, idx.data().join( " " ) );

}

QPtrList<QMakeScopeItem> ProjectConfigurationDlg::getAllProjects()
{
    QPtrList <QMakeScopeItem> tmpPrjList;
    QMakeScopeItem *item = static_cast<QMakeScopeItem*>( prjList->firstChild() );
    while ( item )
    {
        if ( item->scope->scopeType() == Scope::ProjectScope )
        {
            if( item != myProjectItem )
                tmpPrjList.append( item );
            getAllSubProjects( item, &tmpPrjList );
        }
        item = static_cast<QMakeScopeItem*>( item->nextSibling() );
    }
    return ( tmpPrjList );
}

void ProjectConfigurationDlg::getAllSubProjects( QMakeScopeItem *item, QPtrList <QMakeScopeItem> *itemList )
{

    QMakeScopeItem * subItem = static_cast<QMakeScopeItem*>( item->firstChild() );
    while ( subItem )
    {
        if ( subItem->scope->scopeType() == Scope::ProjectScope )
        {
            if ( subItem != myProjectItem )
                itemList->append( subItem );
            getAllSubProjects( subItem, itemList );
        }
        subItem = static_cast<QMakeScopeItem*>( subItem->nextSibling() );
    }
}

void ProjectConfigurationDlg::updateIncludeControl()
{
    insideinc_listview->setSorting( -1, false );
    outsideinc_listview->setSorting( -1, false );
    insideinc_listview->clear();
    outsideinc_listview->clear();

    QStringList incList = myProjectItem->scope->variableValues( "INCLUDEPATH" );
    QPtrList <QMakeScopeItem> itemList = getAllProjects();
    QMakeScopeItem *item = itemList.first();

    while ( item )
    {
        if ( item->scope->variableValues( "TEMPLATE" ).contains( "lib" ) ||
                item->scope->variableValues( "TEMPLATE" ).contains( "app" ) )
        {
            QString tmpInc = item->getIncAddPath( myProjectItem->scope->projectDir() );
            tmpInc = QDir::cleanDirPath( tmpInc );
            InsideCheckListItem *newItem = new InsideCheckListItem( insideinc_listview,
                                           insideinc_listview->lastItem(), item, this );
            QStringList::Iterator it = incList.begin();

            if ( incList.contains( tmpInc ) )
            {
                incList.remove( tmpInc );
                newItem->setOn( true );
            }
        }
        //    item=(ProjectItem*)item->itemBelow();
        item = itemList.next();
    }


    //all other in incList are outside incs
    outsideinc_listview->clear();
    QStringList::Iterator it1 = incList.begin();
    for ( ;it1 != incList.end();++it1 )
    {
        new QListViewItem( outsideinc_listview, outsideinc_listview->lastItem(), ( *it1 ) );
    }
}

void ProjectConfigurationDlg::updateLibControls()
{

    QPtrList <QMakeScopeItem> itemList = getAllProjects();

    insidelib_listview->setSorting( -1, false );
    outsidelib_listview->setSorting( -1, false );
    insidelib_listview->clear();
    outsidelib_listview->clear();
    //update librarys
    //temp strlist
    QStringList libList = myProjectItem->scope->variableValues( "LIBS" );
    QMakeScopeItem* item = itemList.first();
    while ( item )
    {
        if ( item->scope->variableValues( "TEMPLATE" ).contains( "lib" ) )
        {
            if ( item != myProjectItem )
            {
                // create lib string
                QString tmpLib = item->getLibAddObject( myProjectItem->scope->projectDir() );
                InsideCheckListItem * newItem = new InsideCheckListItem( insidelib_listview,
                                                insidelib_listview->lastItem(), item, this );
                QString tmpLibDir = item->getLibAddPath( myProjectItem->scope->projectDir() );

                if ( libList.contains( "-L" + tmpLibDir ) )
                {
                    libList.remove( "-L" + tmpLibDir );
                }
                if ( libList.contains( tmpLib ) )
                {
                    libList.remove( tmpLib );
                    newItem->setOn( true );
                }
            }
        }
        item = itemList.next();
    }

    //all other in libList are outside libs
    outsidelib_listview->clear();
    outsidelibdir_listview->clear();
    QStringList::Iterator it1 = libList.begin();
    for ( ;it1 != libList.end();++it1 )
    {
        if ( ( *it1 ).startsWith( "-L" ) )
            new QListViewItem( outsidelibdir_listview, outsidelibdir_listview->lastItem(), ( *it1 ).mid( 2 ) );
        else
        {
            new QListViewItem( outsidelib_listview, outsidelib_listview->lastItem(), ( *it1 ) );
        }
    }
}

void ProjectConfigurationDlg::updateDependenciesControl( )
{
    QPtrList <QMakeScopeItem> itemList = getAllProjects();

    intDeps_view->setSorting( -1, false );
    extDeps_view->setSorting( -1, false );
    intDeps_view->clear();
    extDeps_view->clear();

    QStringList depsList = myProjectItem->scope->variableValues( "TARGETDEPS" );
    QMakeScopeItem *item = itemList.first();
    while ( item )
    {
        QStringList templateval = item->scope->variableValues( "TEMPLATE" );
        if ( templateval.contains( "lib" )
                || templateval.contains( "app" ) )
        {
            QString tmpLib;
            QStringList values = item->scope->variableValues( "CONFIG" );
            if ( templateval.contains( "lib" ) && values.contains( "dll" ) )
                tmpLib = item->getSharedLibAddObject( myProjectItem->scope->projectDir() );
            else if ( templateval.contains( "lib" ) )
                tmpLib = item->getLibAddObject( myProjectItem->scope->projectDir() );
            else
                tmpLib = item->getApplicationObject( myProjectItem->scope->projectDir() );
            InsideCheckListItem * newItem = new InsideCheckListItem( intDeps_view, intDeps_view->lastItem(), item, this );
            if ( depsList.contains( tmpLib ) )
            {
                depsList.remove( tmpLib );
                newItem->setOn( true );
            }

        }
        item = itemList.next();
    }

    //add all other prj in itemList unchecked

    extDeps_view->clear();
    QStringList::Iterator it1 = depsList.begin();
    for ( ;it1 != depsList.end();++it1 )
    {
        new QListViewItem( extDeps_view, extDeps_view->lastItem(), ( *it1 ) );
    }
}


void ProjectConfigurationDlg::updateBuildOrderControl()
{
    //sort build order only if subdirs
    if ( myProjectItem->scope->variableValues( "TEMPLATE" ).contains( "subdirs" ) )
    {

        QPtrList <QMakeScopeItem> itemList;

        QMakeScopeItem *item = static_cast<QMakeScopeItem*>( myProjectItem->firstChild() );
        while ( item )
        {
            itemList.append( item );
            item = static_cast<QMakeScopeItem*>( item->nextSibling() );
        }

        incaddTab->setEnabled( false );
        buildorder_listview->setSorting( -1, false );
        buildorder_listview->clear();
        QStringList buildList = myProjectItem->scope->variableValues( "SUBDIRS" );
        QStringList::Iterator it1 = buildList.begin();
        for ( ;it1 != buildList.end();++it1 )
        {
            item = itemList.first();
            while ( item )
            {
                if ( item->scope->scopeType() == Scope::ProjectScope )
                {
                    if ( item->text( 0 ) == ( *it1 ) )
                    {
                        new QListViewItem( buildorder_listview, buildorder_listview->lastItem(), item->text( 0 ) );
                        itemList.take();
                        break;
                    }
                }
                item = itemList.next();;
            }
        }
    }else
        buildorder_listview->clear();
}

//build order buttons
void ProjectConfigurationDlg::buildorderMoveUpClicked()
{
    if ( buildorder_listview->currentItem() == buildorder_listview->firstChild() )
    {
        KNotifyClient::beep();
        return ;
    }

    QListViewItem *item = buildorder_listview->firstChild();
    while ( item->nextSibling() != buildorder_listview->currentItem() )
        item = item->nextSibling();
    item->moveItem( buildorder_listview->currentItem() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::buildorderMoveDownClicked()
{
    if ( buildorder_listview->currentItem() == 0 || buildorder_listview->currentItem() ->nextSibling() == 0 )
    {
        KNotifyClient::beep();
        return ;
    }

    buildorder_listview->currentItem() ->moveItem( buildorder_listview->currentItem() ->nextSibling() );
    activateApply( 0 );
}


//Include dir buttons
void ProjectConfigurationDlg::insideIncMoveUpClicked()
{
    if ( insideinc_listview->currentItem() == insideinc_listview->firstChild() )
    {
        KNotifyClient::beep();
        return ;
    }

    QListViewItem *item = insideinc_listview->firstChild();
    while ( item->nextSibling() != insideinc_listview->currentItem() )
        item = item->nextSibling();
    item->moveItem( insideinc_listview->currentItem() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::insideIncMoveDownClicked()
{
    if ( insideinc_listview->currentItem() == 0 || insideinc_listview->currentItem() ->nextSibling() == 0 )
    {
        KNotifyClient::beep();
        return ;
    }

    insideinc_listview->currentItem() ->moveItem( insideinc_listview->currentItem() ->nextSibling() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideIncMoveUpClicked()
{
    if ( outsideinc_listview->currentItem() == outsideinc_listview->firstChild() )
    {
        KNotifyClient::beep();
        return ;
    }

    QListViewItem *item = outsideinc_listview->firstChild();
    while ( item->nextSibling() != outsideinc_listview->currentItem() )
        item = item->nextSibling();
    item->moveItem( outsideinc_listview->currentItem() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideIncMoveDownClicked()
{
    if ( outsideinc_listview->currentItem() == 0 || outsideinc_listview->currentItem() ->nextSibling() == 0 )
    {
        KNotifyClient::beep();
        return ;
    }

    outsideinc_listview->currentItem() ->moveItem( outsideinc_listview->currentItem() ->nextSibling() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideIncAddClicked()
{
    KURLRequesterDlg dialog( "", i18n( "Add include directory:" ), 0, 0 );
    dialog.urlRequester() ->setMode( KFile::Directory );
    if ( dialog.exec() != QDialog::Accepted )
        return ;
    QString dir = dialog.urlRequester() ->url();
    if ( !dir.isEmpty() )
        new QListViewItem( outsideinc_listview, dir );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideIncRemoveClicked()
{
    delete outsideinc_listview->currentItem();
    activateApply( 0 );
}

//libadd buttons
void ProjectConfigurationDlg::insideLibMoveUpClicked()
{
    if ( insidelib_listview->currentItem() == insidelib_listview->firstChild() )
    {
        KNotifyClient::beep();
        return ;
    }

    QListViewItem *item = insidelib_listview->firstChild();
    while ( item->nextSibling() != insidelib_listview->currentItem() )
        item = item->nextSibling();
    item->moveItem( insidelib_listview->currentItem() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::insideLibMoveDownClicked()
{
    if ( insidelib_listview->currentItem() == 0 || insidelib_listview->currentItem() ->nextSibling() == 0 )
    {
        KNotifyClient::beep();
        return ;
    }

    insidelib_listview->currentItem() ->moveItem( insidelib_listview->currentItem() ->nextSibling() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideLibMoveUpClicked()
{
    if ( outsidelib_listview->currentItem() == outsidelib_listview->firstChild() )
    {
        KNotifyClient::beep();
        return ;
    }

    QListViewItem *item = outsidelib_listview->firstChild();
    while ( item->nextSibling() != outsidelib_listview->currentItem() )
        item = item->nextSibling();
    item->moveItem( outsidelib_listview->currentItem() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideLibMoveDownClicked()
{
    if ( outsidelib_listview->currentItem() == 0 || outsidelib_listview->currentItem() ->nextSibling() == 0 )
    {
        KNotifyClient::beep();
        return ;
    }

    outsidelib_listview->currentItem() ->moveItem( outsidelib_listview->currentItem() ->nextSibling() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideLibAddClicked()
{
    bool ok;
    QString dir = KInputDialog::getText( i18n( "Add Library" ), i18n( "Add library to link:" ), "-l", &ok, 0 );
    if ( ok && !dir.isEmpty() && dir != "-I" )
        new QListViewItem( outsidelib_listview, dir );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideLibRemoveClicked()
{
    delete outsidelib_listview->currentItem();
    activateApply( 0 );
}

//lib paths buttons
void ProjectConfigurationDlg::outsideLibDirMoveUpClicked()
{
    if ( outsidelibdir_listview->currentItem() == outsidelibdir_listview->firstChild() )
    {
        KNotifyClient::beep();
        return ;
    }

    QListViewItem *item = outsidelibdir_listview->firstChild();
    while ( item->nextSibling() != outsidelibdir_listview->currentItem() )
        item = item->nextSibling();
    item->moveItem( outsidelibdir_listview->currentItem() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideLibDirMoveDownClicked()
{
    if ( outsidelibdir_listview->currentItem() == 0 || outsidelibdir_listview->currentItem() ->nextSibling() == 0 )
    {
        KNotifyClient::beep();
        return ;
    }

    outsidelibdir_listview->currentItem() ->moveItem( outsidelibdir_listview->currentItem() ->nextSibling() );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideLibDirAddClicked()
{
    KURLRequesterDlg dialog( "", i18n( "Add library directory:" ), 0, 0 );
    dialog.urlRequester() ->setMode( KFile::Directory );
    if ( dialog.exec() != QDialog::Accepted )
        return ;
    QString dir = dialog.urlRequester() ->url();
    if ( !dir.isEmpty() )
        new QListViewItem( outsidelibdir_listview, dir );
    activateApply( 0 );
}


void ProjectConfigurationDlg::outsideLibDirRemoveClicked()
{
    delete outsidelibdir_listview->currentItem();
    activateApply( 0 );
}

void ProjectConfigurationDlg::outsideIncEditClicked()
{
    QListViewItem * item = outsideinc_listview->currentItem();
    if ( item == NULL ) return ;
    QString text = item->text( 0 );

    KURLRequesterDlg dialog( text, i18n( "Change include directory:" ), 0, 0 );
    dialog.urlRequester() ->setMode( KFile::Directory );
    if ( dialog.exec() != QDialog::Accepted )
        return ;
    QString dir = dialog.urlRequester() ->url();
    if ( !dir.isEmpty() )
        item->setText( 0, dir );
    activateApply( 0 );
}

void ProjectConfigurationDlg::outsideLibEditClicked()
{
    bool ok;
    QListViewItem *item = outsidelib_listview->currentItem();
    if ( item == NULL ) return ;
    QString text = item->text( 0 );

    QString dir = KInputDialog::getText( i18n( "Change Library" ), i18n( "Change library to link:" ), text, &ok, 0 );
    if ( ok && !dir.isEmpty() && dir != "-l" )
        item->setText( 0, dir );
    activateApply( 0 );
}

void ProjectConfigurationDlg::outsideLibDirEditClicked()
{
    QListViewItem * item = outsidelibdir_listview->currentItem();
    if ( item == NULL ) return ;
    QString text = item->text( 0 );

    KURLRequesterDlg dialog( text, i18n( "Change library directory:" ), 0, 0 );
    dialog.urlRequester() ->setMode( KFile::Directory );
    if ( dialog.exec() != QDialog::Accepted )
        return ;
    QString dir = dialog.urlRequester() ->url();
    if ( !dir.isEmpty() )
        item->setText( 0, dir );
    activateApply( 0 );
}


void ProjectConfigurationDlg::extAdd_button_clicked( )
{
    KURLRequesterDlg dialog( "", i18n( "Add target:" ), 0, 0 );
    dialog.urlRequester() ->setMode( KFile::File );
    if ( dialog.exec() != QDialog::Accepted )
        return ;
    QString path = dialog.urlRequester() ->url();
    if ( !path.isEmpty() )
        new QListViewItem( extDeps_view, path );
    activateApply( 0 );
}

void ProjectConfigurationDlg::extEdit_button_clicked( )
{
    QListViewItem * item = extDeps_view->currentItem();
    if ( item == NULL ) return ;
    QString text = item->text( 0 );

    KURLRequesterDlg dialog( text, i18n( "Change target:" ), 0, 0 );
    dialog.urlRequester() ->setMode( KFile::File );
    if ( dialog.exec() != QDialog::Accepted )
        return ;
    QString path = dialog.urlRequester() ->url();
    if ( !path.isEmpty() )
        item->setText( 0, path );
    activateApply( 0 );
}

void ProjectConfigurationDlg::extMoveDown_button_clicked( )
{
    if ( extDeps_view->currentItem() == 0 || extDeps_view->currentItem() ->nextSibling() == 0 )
    {
        KNotifyClient::beep();
        return ;
    }

    extDeps_view->currentItem() ->moveItem( extDeps_view->currentItem() ->nextSibling() );
    activateApply( 0 );
}

void ProjectConfigurationDlg::extMoveUp_button_clicked( )
{
    if ( extDeps_view->currentItem() == extDeps_view->firstChild() )
    {
        KNotifyClient::beep();
        return ;
    }

    QListViewItem *item = extDeps_view->firstChild();
    while ( item->nextSibling() != extDeps_view->currentItem() )
        item = item->nextSibling();
    item->moveItem( extDeps_view->currentItem() );
    activateApply( 0 );
}

void ProjectConfigurationDlg::extRemove_button_clicked( )
{
    delete extDeps_view->currentItem();
    activateApply( 0 );
}

void ProjectConfigurationDlg::intMoveDown_button_clicked( )
{
    if ( intDeps_view->currentItem() == 0 || intDeps_view->currentItem() ->nextSibling() == 0 )
    {
        KNotifyClient::beep();
        return ;
    }

    intDeps_view->currentItem() ->moveItem( intDeps_view->currentItem() ->nextSibling() );
    activateApply( 0 );
}

void ProjectConfigurationDlg::intMoveUp_button_clicked( )
{
    if ( intDeps_view->currentItem() == intDeps_view->firstChild() )
    {
        KNotifyClient::beep();
        return ;
    }

    QListViewItem *item = intDeps_view->firstChild();
    while ( item->nextSibling() != intDeps_view->currentItem() )
        item = item->nextSibling();
    item->moveItem( intDeps_view->currentItem() );
    activateApply( 0 );
}

void ProjectConfigurationDlg::addCustomValueClicked()
{
    QListViewItem * item = new QListViewItem( customVariables, i18n( "Name" ), "=", i18n( "Value" ) );
    customVariables->setSelected( item, true );
    newCustomVariableActive();
    customVariableName->setEnabled( true );
    customVariableOp->setEnabled( true );
    activateApply( 0 );
}
void ProjectConfigurationDlg::removeCustomValueClicked()
{
    QListViewItem * item = customVariables->currentItem();
    if ( item )
    {
        delete item;
    }
    activateApply( 0 );
}
void ProjectConfigurationDlg::editCustomValueClicked()
{
    QListViewItem * item = customVariables->currentItem();
    if ( item )
    {
        item->setText( 0, customVariableName->text() );
        item->setText( 1, customVariableOp->currentText() );
        item->setText( 2, customVariableData->text() );
    }
    activateApply( 0 );
}
void ProjectConfigurationDlg::upCustomValueClicked()
{
    // custom vars
    QListViewItem * item = customVariables->firstChild();
    if ( customVariables->currentItem() == item )
    {
        KNotifyClient::beep();
        return ;
    }
    while ( item->nextSibling() != customVariables->currentItem() )
        item = item->nextSibling();
    item->moveItem( customVariables->currentItem() );
    activateApply( 0 );
}

void ProjectConfigurationDlg::downCustomValueClicked()
{
    if ( customVariables->currentItem() == 0 || customVariables->currentItem() ->nextSibling() == 0 )
    {
        KNotifyClient::beep();
        return ;
    }
    customVariables->currentItem() ->moveItem( customVariables->currentItem() ->nextSibling() );
    activateApply( 0 );
}

void ProjectConfigurationDlg::newCustomVariableActive( )
{
    QListViewItem * item = customVariables->currentItem();
    if ( item )
    {
        customVariableName->setText( item->text( 0 ) );
        customVariableData->setText( item->text( 2 ) );
        customVariableOp->setCurrentText( item->text( 1 ) );
        customVariableName->setFocus();
        customVariableName->setEnabled( false );
        customVariableOp->setEnabled( false );
    }
}

void ProjectConfigurationDlg::groupLibrariesChanged( int )
{
    if ( staticRadio->isChecked() )
    {
        checkPlugin->setEnabled( false );
        checkDesigner->setEnabled( false );
    }
    else if ( sharedRadio->isChecked() )
    {
        checkPlugin->setEnabled( true );
        checkDesigner->setEnabled( checkPlugin->isChecked() );
    }
    else if ( checkPlugin->isChecked() && prjWidget->m_part->isQt4Project() )
    {
        checkDesigner->setEnabled( true );
    }
    else
    {
        checkDesigner->setEnabled( false );
    }
    activateApply( 0 );
}

void ProjectConfigurationDlg::groupTemplateChanged( int )
{

    if ( radioSubdirs->isChecked() )
    {
        TabBuild->setTabEnabled( custVarsTab, true );
        TabBuild->setTabEnabled( libAddTab, false );
        TabBuild->setTabEnabled( incaddTab, false );
        TabBuild->setTabEnabled( buildOptsTab, false );
        TabBuild->setTabEnabled( configTab, false );
        intDeps_view->setEnabled( false );
        intMoveUp_button->setEnabled( false );
        intMoveDown_button->setEnabled( false );
        extAdd_button->setEnabled( false );
        extRemove_button->setEnabled( false );
        extEdit_button->setEnabled( false );
        extMoveUp_button->setEnabled( false );
        extMoveDown_button->setEnabled( false );
        extDeps_view->setEnabled( false );
        buildorder_listview->setEnabled( true );
        buildmoveup_button->setEnabled( true );
        buildmovedown_button->setEnabled( true );
        checkOrdered->setEnabled( true );
    }
    else if ( radioLibrary->isChecked() )
    {
        //    staticRadio->setChecked(true);
        TabBuild->setTabEnabled( custVarsTab, true );
        TabBuild->setTabEnabled( depTab, true );
        TabBuild->setTabEnabled( libAddTab, true );
        TabBuild->setTabEnabled( incaddTab, true );
        TabBuild->setTabEnabled( buildOptsTab, true );
        TabBuild->setTabEnabled( configTab, true );
        intDeps_view->setEnabled( true );
        intMoveUp_button->setEnabled( true );
        intMoveDown_button->setEnabled( true );
        extAdd_button->setEnabled( true );
        extRemove_button->setEnabled( true );
        extEdit_button->setEnabled( true );
        extMoveUp_button->setEnabled( true );
        extMoveDown_button->setEnabled( true );
        extDeps_view->setEnabled( true );
        buildorder_listview->setEnabled( false );
        buildmoveup_button->setEnabled( false );
        buildmovedown_button->setEnabled( false );
        groupLibraries->setEnabled( true );
        checkOrdered->setEnabled( false );
    }
    else if ( radioApplication->isChecked() )
    {
        TabBuild->setTabEnabled( custVarsTab, true );
        TabBuild->setTabEnabled( depTab, true );
        TabBuild->setTabEnabled( libAddTab, true );
        TabBuild->setTabEnabled( incaddTab, true );
        TabBuild->setTabEnabled( buildOptsTab, true );
        TabBuild->setTabEnabled( configTab, true );
        intDeps_view->setEnabled( true );
        intMoveUp_button->setEnabled( true );
        intMoveDown_button->setEnabled( true );
        extAdd_button->setEnabled( true );
        extRemove_button->setEnabled( true );
        extEdit_button->setEnabled( true );
        extMoveUp_button->setEnabled( true );
        extMoveDown_button->setEnabled( true );
        extDeps_view->setEnabled( true );
        buildorder_listview->setEnabled( false );
        buildmoveup_button->setEnabled( false );
        buildmovedown_button->setEnabled( false );
        groupLibraries->setEnabled( false );
        checkConsole->setEnabled( true );
        checkWindows->setEnabled( true );
        checkOrdered->setEnabled( false );
    }
    activateApply( 0 );
}

void ProjectConfigurationDlg::groupRequirementsChanged( int )
{
    if ( checkQt->isChecked() && prjWidget->m_part->isQt4Project() )
    {
        groupQt4Libs->setEnabled( true );
    }
    else
    {
        groupQt4Libs->setEnabled( false );
    }
    activateApply( 0 );
}

void ProjectConfigurationDlg::targetInstallChanged( bool checked )
{
    if ( checked )
    {
        m_InstallTargetPath->setEnabled( true );
    }
    else
    {
        m_InstallTargetPath->setEnabled( false );
    }
    activateApply( 0 );
}

void ProjectConfigurationDlg::apply()
{
//     if( buttonApply->isEnabled() )
        updateProjectConfiguration();
    myProjectItem->scope->saveToFile();
    //     prjWidget->updateProjectConfiguration( myProjectItem );
    prjWidget->setupContext();
    buttonApply->setEnabled( false );
}

void ProjectConfigurationDlg::activateApply( int )
{
    buttonApply->setEnabled( true );
}
void ProjectConfigurationDlg::activateApply( const QString& )
{
    buttonApply->setEnabled( true );
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

void ProjectConfigurationDlg::activateApply( QListViewItem* )
{
    buttonApply->setEnabled( true );
}

