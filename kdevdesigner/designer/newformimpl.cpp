/**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "newformimpl.h"
#include "mainwindow.h"
#include "metadatabase.h"
#include "project.h"
#include "formwindow.h"
#include "widgetfactory.h"
#include "widgetdatabase.h"
#include "actioneditorimpl.h"
#include "hierarchyview.h"
#include "resource.h"
#include "projectsettingsimpl.h"
#include "sourcefile.h"

#include <kiconloader.h>
#include "kdevdesigner_part.h"

#include <qiconview.h>
#include <qlabel.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qpushbutton.h>
#include <stdlib.h>
#include <qcombobox.h>
#include <qworkspace.h>
#include <qmessagebox.h>

static int forms = 0;

ProjectItem::ProjectItem( QIconView *view, const QString &text )
    : NewItem( view, text )
{
}

void ProjectItem::insert( Project * )
{
    MainWindow::self->createNewProject( lang );
}



FormItem::FormItem( QIconView *view, const QString &text )
    : NewItem( view, text )
{
}

void FormItem::insert( Project *pro )
{
    QString n = "Form" + QString::number( ++forms );
    FormWindow *fw = 0;
    FormFile *ff = new FormFile( FormFile::createUnnamedFileName(), TRUE, pro );
    fw = new FormWindow( ff, MainWindow::self, MainWindow::self->qWorkspace(), n );
    fw->setProject( pro );
    MetaDataBase::addEntry( fw );
    if ( fType == Widget ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QWidget" ),
					    fw, n.latin1() );
	fw->setMainContainer( w );
    } else if ( fType == Dialog ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QDialog" ), fw, n.latin1() );
	fw->setMainContainer( w );
    } else if ( fType == Wizard ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QWizard" ),
					    fw, n.latin1() );
	fw->setMainContainer( w );
    } else if ( fType == MainWindow ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QMainWindow" ),
					    fw, n.latin1() );
	fw->setMainContainer( w );
    }

    fw->setCaption( n );
    fw->resize( 600, 480 );
    MainWindow::self->insertFormWindow( fw );

    TemplateWizardInterface *iface =
	MainWindow::self->templateWizardInterface( fw->mainContainer()->className() );
    if ( iface ) {
	iface->setup( fw->mainContainer()->className(), fw->mainContainer(),
		      fw->iFace(), MainWindow::self->designerInterface() );
	iface->release();
    }

    // the wizard might have changed a lot, lets update everything
    MainWindow::self->actioneditor()->setFormWindow( fw );
    MainWindow::self->objectHierarchy()->setFormWindow( fw, fw );
    MainWindow::self->objectHierarchy()->formDefinitionView()->refresh();
    MainWindow::self->objectHierarchy()->rebuild();
    fw->killAccels( fw );
    fw->project()->setModified( TRUE );
    fw->setFocus();
    if ( !pro->isDummy() ) {
	fw->setSavePixmapInProject( TRUE );
	fw->setSavePixmapInline( FALSE );
    }
}



CustomFormItem::CustomFormItem( QIconView *view, const QString &text )
    : NewItem( view, text )
{
}

static void unifyFormName( FormWindow *fw, QWorkspace *qworkspace )
{
    QStringList lst;
    QWidgetList windows = qworkspace->windowList();
    for ( QWidget *w =windows.first(); w; w = windows.next() ) {
	if ( w == fw )
	    continue;
	lst << w->name();
    }

    if ( lst.findIndex( fw->name() ) == -1 )
	return;
    QString origName = fw->name();
    QString n = origName;
    int i = 1;
    while ( lst.findIndex( n ) != -1 ) {
	n = origName + QString::number( i++ );
    }
    fw->setName( n );
    fw->setCaption( n );
}

void CustomFormItem::insert( Project *pro )
{
    QString filename = templateFileName();
    if ( !filename.isEmpty() && QFile::exists( filename ) ) {
	Resource resource( MainWindow::self );
	FormFile *ff = new FormFile( filename, TRUE, pro );
	if ( !resource.load( ff ) ) {
	    QMessageBox::information( MainWindow::self, MainWindow::tr("Load Template"),
				      MainWindow::tr("Couldn't load form description from template '" +
						     filename + "'" ) );
	    delete ff;
	    return;
	}
	ff->setFileName( QString::null );
	if ( MainWindow::self->formWindow() ) {
	    MainWindow::self->formWindow()->setFileName( QString::null );
	    unifyFormName( MainWindow::self->formWindow(), MainWindow::self->qWorkspace() );
	    if ( !pro->isDummy() ) {
		MainWindow::self->formWindow()->setSavePixmapInProject( TRUE );
		MainWindow::self->formWindow()->setSavePixmapInline( FALSE );
	    }
	}
    }
}



SourceFileItem::SourceFileItem( QIconView *view, const QString &text )
    : NewItem( view, text ), visible( TRUE )
{
}

void SourceFileItem::insert( Project *pro )
{
    SourceFile *f = new SourceFile( SourceFile::createUnnamedFileName( ext ), TRUE, pro );
    MainWindow::self->editSource( f );
}

void SourceFileItem::setProject( Project *pro )
{
    QIconView *iv = iconView();
    bool v = lang == pro->language();
    if ( !iv || v == visible )
	return;
    visible = v;
    if ( !visible )
	iv->takeItem( this );
    else
	iv->insertItem( this );
}



SourceTemplateItem::SourceTemplateItem( QIconView *view, const QString &text )
    : NewItem( view, text ), visible( TRUE )
{
}

void SourceTemplateItem::insert( Project *pro )
{
    SourceTemplateInterface *siface = MainWindow::self->sourceTemplateInterface( text() );
    if ( !siface )
	return;
    SourceTemplateInterface::Source src = siface->create( text(), MainWindow::self->designerInterface() );
    SourceFile *f = 0;
    if ( src.type == SourceTemplateInterface::Source::Invalid )
	return;
    if ( src.type == SourceTemplateInterface::Source::FileName )
	f = new SourceFile( src.filename, FALSE, pro );
    else
	f = new SourceFile( SourceFile::createUnnamedFileName( src.extension ), TRUE, pro );
    if ( f->isAccepted()) {
	f->setText( src.code );
	MainWindow::self->editSource( f );
	f->setModified( TRUE );
    } else {
	delete f;
    }
}

void SourceTemplateItem::setProject( Project *pro )
{
    QIconView *iv = iconView();
    bool v = !pro->isDummy() && lang == pro->language();
    if ( !iv || v == visible )
	return;
    visible = v;
    if ( !visible )
	iv->takeItem( this );
    else
	iv->insertItem( this );
}

void NewForm::insertTemplates( QIconView *tView,
			       const QString &templatePath )
{
    QStringList::Iterator it;
    QStringList languages = MetaDataBase::languages();
    if ( !MainWindow::self->singleProjectMode() ) {
	for ( it = languages.begin(); it != languages.end(); ++it ) {
	    ProjectItem *pi = new ProjectItem( tView, *it + " " + tr( "Project" ) );
	    allItems.append( pi );
	    pi->setLanguage( *it );
	    pi->setPixmap( SmallIcon( "designer_project.png" , KDevDesignerPartFactory::instance()) );
	    pi->setDragEnabled( FALSE );
	}
    }
    QIconViewItem *cur = 0;
    FormItem *fi = new FormItem( tView,tr( "Dialog" ) );
    allItems.append( fi );
    fi->setFormType( FormItem::Dialog );
    fi->setPixmap( SmallIcon( "designer_newform.png" , KDevDesignerPartFactory::instance()) );
    fi->setDragEnabled( FALSE );
    cur = fi;
    if ( !MainWindow::self->singleProjectMode() ) {
	fi = new FormItem( tView,tr( "Wizard" ) );
	allItems.append( fi );
	fi->setFormType( FormItem::Wizard );
	fi->setPixmap( SmallIcon( "designer_newform.png" , KDevDesignerPartFactory::instance()) );
	fi->setDragEnabled( FALSE );
	fi = new FormItem( tView, tr( "Widget" ) );
	allItems.append( fi );
	fi->setFormType( FormItem::Widget );
	fi->setPixmap( SmallIcon( "designer_newform.png" , KDevDesignerPartFactory::instance()) );
	fi->setDragEnabled( FALSE );
	fi = new FormItem( tView, tr( "Main Window" ) );
	allItems.append( fi );
	fi->setFormType( FormItem::MainWindow );
	fi->setPixmap( SmallIcon( "designer_newform.png" , KDevDesignerPartFactory::instance()) );
	fi->setDragEnabled( FALSE );

	QString templPath = templatePath;
	QStringList templRoots;
	const char *qtdir = getenv( "QTDIR" );
	if(qtdir)
	    templRoots << qtdir;
	templRoots << qInstallPathData();
	if(qtdir) //try the tools/designer directory last!
	    templRoots << (QString(qtdir) + "/tools/designer");
	for ( QStringList::Iterator it = templRoots.begin(); it != templRoots.end(); ++it ) {
	    QString path = (*it) + "/templates";
	    if ( QFile::exists( path )) {
		templPath = path;
		break;
	    }
	}
	if ( !templPath.isEmpty() ) {
	    QDir dir( templPath  );
	    const QFileInfoList *filist = dir.entryInfoList( QDir::DefaultFilter, QDir::DirsFirst | QDir::Name );
	    if ( filist ) {
		QFileInfoListIterator it( *filist );
		QFileInfo *fi;
		while ( ( fi = it.current() ) != 0 ) {
		    ++it;
		    if ( !fi->isFile() || fi->extension() != "ui" )
			continue;
		    QString name = fi->baseName();
		    name = name.replace( '_', ' ' );
		    CustomFormItem *ci = new CustomFormItem( tView, name );
		    allItems.append( ci );
		    ci->setDragEnabled( FALSE );
		    ci->setPixmap( BarIcon( "designer_newform.png" , KDevDesignerPartFactory::instance()) );
		    ci->setTemplateFile( fi->absFilePath() );
		}
	    }
	}
    }

    for ( it = languages.begin(); it != languages.end(); ++it ) {
	LanguageInterface *iface = MetaDataBase::languageInterface( *it );
	if ( iface ) {
	    QMap<QString, QString> extensionMap;
	    iface->preferedExtensions( extensionMap );
	    for ( QMap<QString, QString>::Iterator eit = extensionMap.begin();
		  eit != extensionMap.end(); ++eit ) {
		SourceFileItem * si = new SourceFileItem( tView, *eit );
		allItems.append( si );
		si->setExtension( eit.key() );
		si->setLanguage( *it );
		si->setPixmap( BarIcon( "designer_filenew.png", KDevDesignerPartFactory::instance() ) );
		si->setDragEnabled( FALSE );
	    }
	    iface->release();
	}
    }

    if ( !MainWindow::self->singleProjectMode() ) {
	QStringList sourceTemplates = MainWindow::self->sourceTemplates();
	for ( QStringList::Iterator sit = sourceTemplates.begin(); sit != sourceTemplates.end(); ++sit ) {
	    SourceTemplateInterface *siface = MainWindow::self->sourceTemplateInterface( *sit );
	    if ( !siface )
		continue;
	    SourceTemplateItem * si = new SourceTemplateItem( tView, *sit );
	    allItems.append( si );
	    si->setTemplate( *sit );
	    si->setLanguage( siface->language( *sit ) );
	    si->setPixmap( BarIcon( "designer_filenew.png", KDevDesignerPartFactory::instance() ) );
	    si->setDragEnabled( FALSE );
	    siface->release();
	}
    }

    tView->viewport()->setFocus();
    tView->setCurrentItem( cur );

    if ( MainWindow::self->singleProjectMode() )
	adjustSize();
}

NewForm::NewForm( QIconView *templateView, const QString &templatePath )
{
    insertTemplates( templateView, templatePath );
    projectChanged( tr( "<No Project>" ) );
}

NewForm::NewForm( QWidget *parent, const QStringList& projects,
		  const QString& currentProject, const QString &templatePath )
    : NewFormBase( parent, 0, TRUE )
{
    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );

    projectCombo->insertStringList( projects );
    projectCombo->setCurrentText( currentProject );

    insertTemplates( templateView, templatePath );

    projectChanged( projectCombo->currentText() );
}

void NewForm::accept()
{
    if ( !templateView->currentItem() )
	return;
    Project *pro = MainWindow::self->findProject( projectCombo->currentText() );
    if ( !pro )
	return;
    MainWindow::self->setCurrentProject( pro );
    NewFormBase::accept();
    ( (NewItem*)templateView->currentItem() )->insert( pro );
}

void NewForm::projectChanged( const QString &project )
{
    Project *pro = MainWindow::self->findProject( project );
    if ( !pro )
	return;
    QIconViewItem *i;
    for ( i = allItems.first(); i; i = allItems.next() )
	( (NewItem*)i )->setProject( pro );
    templateView->setCurrentItem( templateView->firstItem() );
    templateView->arrangeItemsInGrid( TRUE );
}

void NewForm::itemChanged( QIconViewItem *item )
{
    labelProject->setEnabled( item->rtti() != NewItem::ProjectType );
    projectCombo->setEnabled( item->rtti() != NewItem::ProjectType );
}

QPtrList<QIconViewItem> NewForm::allViewItems()
{
    return allItems;
}
