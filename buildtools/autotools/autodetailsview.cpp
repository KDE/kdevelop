/*
   KDevelop Autotools Support
   Copyright (c) 2002 by Victor Roeder <victor_roeder@gmx.de>
   Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/

#include "autodetailsview.h"

/** Qt */
#include <qregexp.h>
#include <qlistview.h>
#include <qwidget.h>
#include <qheader.h>
#include <qlayout.h>

 /** KDE Libs */
#include <kxmlguiclient.h>
#include <kaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <klistview.h>
#include <kpopupmenu.h>

/** KDevelop */
#include "kdevappfrontend.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "kdevcreatefile.h"
#include "kdevlanguagesupport.h"
#include "kdevmakefrontend.h"

#include "domutil.h"

#include "targetoptionsdlg.h"
#include "addfiledlg.h"
#include "addicondlg.h"
#include "addexistingfilesdlg.h"
#include "removefiledlg.h"
#include "removetargetdlg.h"

#include "autolistviewitems.h"
#include "autotoolsaction.h"
#include "autoprojectpart.h"
#include "autoprojectwidget.h"

#include "subclassesdlg.h"

AutoDetailsView::AutoDetailsView(AutoProjectWidget* widget, AutoProjectPart* part, QWidget *parent, const char *name)
  : AutoProjectViewBase(parent, name)
{
	m_widget = widget;
	m_part = part;

	initActions();
	QDomDocument dom = *(m_part->projectDom());
	m_subclasslist = DomUtil::readPairListEntry(dom, "/kdevautoproject/subclassing",
	                                            "subclass","sourcefile", "uifile");

	m_listView->setAllColumnsShowFocus( true );
	m_listView->setRootIsDecorated( true );
	m_listView->setResizeMode( QListView::LastColumn );
	m_listView->addColumn( QString::null );
	m_listView->header()->hide();
	connect( m_listView, SIGNAL( selectionChanged( QListViewItem* ) ),
	         this, SLOT( slotSelectionChanged( QListViewItem* ) ) );
}


AutoDetailsView::~AutoDetailsView()
{
}

void AutoDetailsView::slotSelectionChanged( QListViewItem* item )
{
	if ( m_listView->selectedItems().isEmpty() )
	{
		targetOptionsAction->setEnabled( false );
		addNewFileAction->setEnabled( false );
		addExistingFileAction->setEnabled( false );
		buildTargetAction->setEnabled( false );
		executeTargetAction->setEnabled( false );
	}
	else
	{
		bool isTarget = false;
		bool isRegularTarget = false;
		bool isFile = false;
		bool isProgram = false;

		if ( item )
		{
			// We assume here that ALL items in the detail list view
			// are ProjectItem's
			ProjectItem * pvitem = static_cast<ProjectItem*>( item );
			TargetItem* titem = 0;

			if ( pvitem->type() == ProjectItem::File )
			{
				titem = static_cast <TargetItem*> ( pvitem->parent() );

				QString primary = titem->primary;
				if ( primary == "PROGRAMS" || primary == "LIBRARIES" ||
				     primary == "LTLIBRARIES" || primary == "JAVA" )
				{
					isRegularTarget = true; // not a data group
					isFile = true;
				}
			}
			else
			{
				titem = static_cast <TargetItem*> ( pvitem );
				isTarget = true;
			}

			QString primary = titem->primary;
			if ( primary == "PROGRAMS" || primary == "LIBRARIES" ||
			     primary == "LTLIBRARIES" || primary == "JAVA" )
			{
				isRegularTarget = true; // not a data group
			}

			if ( primary == "PROGRAMS" )
				isProgram = true;
		}

		targetOptionsAction->setEnabled( isRegularTarget && !isFile );
		addNewFileAction->setEnabled( isTarget );
		addExistingFileAction->setEnabled( isTarget );
		removeDetailAction->setEnabled ( true );

		if ( isRegularTarget && isFile || isRegularTarget )
		{
			buildTargetAction->setEnabled ( true );
			if( isProgram )
				executeTargetAction->setEnabled ( true );
		}
		else
		{
			buildTargetAction->setEnabled ( false );
			executeTargetAction->setEnabled ( false );
		}
	}

	emit selectionChanged( item );
}

void AutoDetailsView::initActions()
{
	KActionCollection * actions = new KActionCollection( this );

	targetOptionsAction = new AutoToolsAction( i18n( "Options..." ), "configure", 0,
	                                   this, SLOT( slotTargetOptions() ), actions,
	                                   "target options" );
	targetOptionsAction->setWhatsThis(i18n("<b>Options</b><p>Target options dialog that "
	                                       "provides settings for linker flags and lists "
	                                       "of dependencies and external libraries that "
	                                       "are used when compiling the target."));
	targetOptionsAction->plug( m_optionsButton );
	targetOptionsAction->setEnabled( false );

	QToolTip::add( m_button1, tr2i18n( "Create New File..." ) );
	addNewFileAction = new AutoToolsAction( i18n( "Create New File..." ), "filenew", 0,
	                                this, SLOT( slotAddNewFile() ), actions,
	                                "add new file" );
	addNewFileAction->setWhatsThis(i18n("<b>Create new file</b><p>Creates a new file and "
	                                    "adds it to a currently selected target."));
	addNewFileAction->plug( m_button1 );
	addNewFileAction->setEnabled( false );

	QToolTip::add( m_button2, tr2i18n( "Add Existing Files..." ) );
	addExistingFileAction = new AutoToolsAction( i18n( "Add Existing Files..." ), "fileimport", 0,
	                                     this, SLOT( slotAddExistingFile() ), actions,
	                                     "add existing file" );
	addExistingFileAction->setWhatsThis(i18n("<b>Add existing files</b><p>Adds existing "
	                                         "file to a currently selected target. Header "
	                                         "files will not be included in SOURCES list "
	                                         "of a target. They will be added to "
	                                         "noinst_HEADERS instead."));
	addExistingFileAction->plug( m_button2 );
	addExistingFileAction->setEnabled( false );

	addIconAction = new KAction( i18n( "Add Icon..." ), "iconadd_kdevelop", 0,
	                             this, SLOT( slotAddIcon() ), actions, "add icon" );
	addIconAction->setWhatsThis(i18n("<b>Add icon</b><p>Adds an icon to a KDEICON target."));

	QToolTip::add( m_button4, tr2i18n( "Build Target"));
	buildTargetAction = new AutoToolsAction( i18n( "Build Target..." ), "launch", 0,
	                                 this, SLOT( slotBuildTarget() ), actions,
	                                 "build target" );
	buildTargetAction->setWhatsThis(i18n("<b>Build target</b><p>Constructs a series of "
	                                     "make commands to build the selected target. "
	                                     "Also builds dependent targets."));
	buildTargetAction->plug( m_button4 );
	buildTargetAction->setEnabled( false );

	QToolTip::add( m_button5, tr2i18n( "Execute Target..."));
	executeTargetAction = new AutoToolsAction( i18n( "Execute Target..." ), "exec", 0,
	                                   this, SLOT( slotExecuteTarget() ), actions,
	                                   "execute target" );
	executeTargetAction->setWhatsThis(i18n("<b>Execute target</b><p>Executes the target "
	                                       "and tries to build in case it is not built."));
	executeTargetAction->plug( m_button5 );
	executeTargetAction->setEnabled( false );

	setActiveTargetAction = new KAction( i18n( "Make Target Active" ), "", 0,
	                                     this, SLOT( slotSetActiveTarget() ), actions,
	                                     "set active target" );
	setActiveTargetAction->setWhatsThis(i18n("<b>Make target active</b><p>Marks the "
	                                         "currently selected target as 'active'. New "
	                                         "files and classes by default go to an active "
	                                         "target. "
	                                         "Using the <b>Build Active Target</b> menu "
	                                         "command builds it."));

	QToolTip::add( m_button3, tr2i18n( "Remove"));
	removeDetailAction = new AutoToolsAction( i18n( "Remove" ), "editdelete", 0, this,
	                                  SLOT( slotRemoveDetail() ), actions,
	                                  "remove detail" );
	removeDetailAction->setWhatsThis(i18n("<b>Remove</b><p>Shows a list of targets "
	                                      "dependent on the selected target or file and "
	                                      "asks for removal. Also asks if the target or "
	                                      "file should be removed from disk."));
	removeDetailAction->plug( m_button3 );
	removeDetailAction->setEnabled( false );

	connect( m_listView, SIGNAL( executed( QListViewItem* ) ),
	         this, SLOT( slotDetailsExecuted( QListViewItem* ) ) );
	connect( m_listView, SIGNAL( contextMenu( KListView*, QListViewItem*, const QPoint& ) ),
	         this, SLOT( slotDetailsContextMenu( KListView*, QListViewItem*, const QPoint& ) ) );
}

QString AutoDetailsView::getUiFileLink(const QString& relpath, const QString& filename)
{
	DomUtil::PairList::iterator it;

	for (it=m_subclasslist.begin(); it != m_subclasslist.end(); ++it)
	{
		if ((*it).first == QString("/")+relpath+filename)
			return (*it).second;
	}

	return QString::null;
}

void AutoDetailsView::slotTargetOptions()
{
	kdDebug( 9020 ) << "AutoDetailsView::slotTargetOptions()" << endl;
	TargetItem *titem = dynamic_cast <TargetItem*> ( m_listView->selectedItem() );

	if ( !titem )
		return;

	TargetOptionsDialog dlg( m_widget, titem, this, "target options dialog" );

	dlg.setCaption ( i18n ( "Target Options for '%1'" ).arg ( titem->name ) );

	dlg.exec();
}


void AutoDetailsView::slotAddNewFile()
{
	TargetItem * titem = dynamic_cast <TargetItem*> ( m_listView->selectedItem() );
	if ( !titem )
		return;

	KDevCreateFile * createFileSupport = m_part->extension<KDevCreateFile>("KDevelop/CreateFile");
	if (createFileSupport)
	{
		KDevCreateFile::CreatedFile crFile =
			createFileSupport->createNewFile(QString::null,
			                                 m_widget->selectedSubproject()->path);
/*		if (crFile.status == KDevCreateFile::CreatedFile::STATUS_OK)
        {
			FileItem *fitem = m_widget->createFileItem(crFile.filename, m_widget->selectedSubproject());
			titem->sources.append(fitem);
			titem->insertItem(fitem);
			emit selectionChanged( titem ); // update list view
		}*/
    } else {
		AddFileDialog dlg( m_part, m_widget, m_widget->selectedSubproject(), titem,
		                   this, "add file dialog" );
		QString caption;
		if ( titem->name.isEmpty() )
			caption = i18n ( "%1 in %2" ).arg ( titem->primary ).arg ( titem->prefix );
		else
			caption = titem->name;

		dlg.setCaption ( i18n ( "Add New File to '%1'" ).arg ( caption ) );

		if ( dlg.exec() )
			emit selectionChanged( titem ); // update list view
	}
}


void AutoDetailsView::slotAddExistingFile()
{
	TargetItem * titem = dynamic_cast <TargetItem*> ( m_listView->selectedItem() );
	if ( !titem )
		return;

	AddExistingFilesDialog dlg( m_part, m_widget, m_widget->selectedSubproject(), titem,
	                            this, "add existing files" );
	QString caption;
	if ( titem->name.isEmpty() )
		caption = i18n ( "%1 in %2" ).arg ( titem->primary ).arg ( titem->prefix );
	else
		caption = titem->name;

	dlg.setCaption( i18n( "Add Existing Files to '%1'" ).arg ( caption ) );

	dlg.exec();
}


void AutoDetailsView::slotAddIcon()
{
	TargetItem * titem = dynamic_cast <TargetItem*> ( m_listView->selectedItem() );
	if ( !titem )
		return ;

	AddIconDialog dlg( m_part, m_widget, m_widget->selectedSubproject(), titem,
	                   this, "add icon" );
	dlg.exec();
}


void AutoDetailsView::slotBuildTarget()
{
	ProjectItem * pvitem = dynamic_cast<ProjectItem*>( m_listView->selectedItem() );
	if ( !pvitem )
		return;

	TargetItem* titem = 0;

	if ( pvitem->type() == ProjectItem::File )
		titem = static_cast <TargetItem*> ( pvitem->parent() );
	else
		titem = static_cast <TargetItem*> ( m_listView->selectedItem() );

	QString relpath = m_widget->selectedSubproject()->path.mid( m_part->projectDirectory().length() );
	m_part->buildTarget(relpath, titem);
}

void AutoDetailsView::slotExecuteTarget()
{
	ProjectItem * pvitem = dynamic_cast<ProjectItem*>( m_listView->selectedItem() );
	if ( !pvitem )
		return;

	TargetItem* titem = 0;

	if ( pvitem->type() == ProjectItem::File )
		titem = static_cast <TargetItem*> ( pvitem->parent() );
	else
		titem = static_cast <TargetItem*> ( m_listView->selectedItem() );


	m_part->executeTarget(m_widget->selectedSubproject()->path, titem);
}

void AutoDetailsView::slotRemoveDetail()
{
	ProjectItem * pvitem = dynamic_cast<ProjectItem*>( m_listView->selectedItem() );

	if ( pvitem && ( pvitem->type() == ProjectItem::File ) )
	{
		FileItem * fitem = static_cast <FileItem*> ( m_listView->selectedItem() );
		if(fitem && fitem->is_subst)
		{
			fitem->changeMakefileEntry("");
			return;
		}

		QListViewItem* sibling = fitem->nextSibling();

		if ( !fitem )
			return;

		TargetItem *titem = static_cast<TargetItem*>( fitem->parent() );

		RemoveFileDialog dlg( m_widget, m_part, m_widget->selectedSubproject(),
		                      titem, fitem->text( 0 ), this, "remove file dialog" );

		QString caption;
		if ( titem->name.isEmpty() )
			caption = i18n ( "%1 in %2" ).arg ( titem->primary ).arg ( titem->prefix );
		else
			caption = titem->name;

		dlg.setCaption ( i18n ( "Remove File From '%1'" ).arg ( caption ) );

		if ( dlg.exec() )
		{
			emit selectionChanged( titem );

			if ( sibling)
			{
				m_listView->setSelected ( sibling, true );
				m_listView->ensureItemVisible ( sibling );
			}
		}

		return;
	}

	if ( pvitem && ( pvitem->type() == ProjectItem::Target ) )
	{
		TargetItem* titem = static_cast <TargetItem*> ( m_listView->selectedItem() );
		QListViewItem* sibling = titem->nextSibling();

		if ( !titem )	return;

		RemoveTargetDialog dlg ( m_widget, m_part, m_widget->selectedSubproject(),
		                         titem, this, "remove target dialog" );

		dlg.setCaption ( i18n ( "Remove Target From '%1'" ).arg ( m_widget->selectedSubproject()->subdir ) );

		if ( dlg.exec() )
		{
			//details->takeItem ( titem );

			m_widget->slotOverviewSelectionChanged ( m_widget->selectedSubproject() );

			if ( sibling)
			{
				m_listView->setSelected ( sibling, true );
				m_listView->ensureItemVisible ( sibling );
			}
		}

		return;
	}
}


void AutoDetailsView::slotDetailsContextMenu( KListView *, QListViewItem *item, const QPoint &p )
{
	if ( !item )
		return;

	ProjectItem *pvitem = dynamic_cast<ProjectItem*>( item );
	if ( !pvitem )
	{
		kdDebug(9020) << k_funcinfo << "Cast to type of ProjectItem* failed."
			<< "Details context menu will be empty!";
		return;
	}

	if ( pvitem->type() == ProjectItem::Target )
	{

		TargetItem * titem = dynamic_cast<TargetItem*>( pvitem );
		if ( !titem )
		{
			kdDebug(9020) << k_funcinfo << "Unable to populate target item menu"
				<< " due to failed cast. " << endl;
			return;
		}

		QString caption;
		if ( titem->name.isEmpty() )
			caption = i18n ( "%1 in %2" ).arg ( titem->primary ).arg ( titem->prefix );
		else
			caption = titem->name;

		KPopupMenu popup( i18n( "Target: %1" ).arg( caption ), this );

		if ( titem->primary == "PROGRAMS" || titem->primary == "LIBRARIES"
		        || titem->primary == "LTLIBRARIES" || titem->primary == "JAVA" )
		{
			targetOptionsAction->plug( &popup );
			popup.insertSeparator();
			addNewFileAction->plug( &popup );
			addExistingFileAction->plug( &popup );
			popup.insertSeparator();
			removeDetailAction->plug( &popup );
			popup.insertSeparator();
			setActiveTargetAction->plug( &popup );
			popup.insertSeparator();
			buildTargetAction->plug( &popup );
			if( titem->primary == "PROGRAMS")
				executeTargetAction->plug( &popup );
		}
		else if ( titem->primary == "KDEDOCS" )
		{
			addNewFileAction->plug( &popup );
			addExistingFileAction->plug( &popup );
			popup.insertSeparator();
			removeDetailAction->plug( &popup );
			popup.insertSeparator();
			buildTargetAction->plug( &popup );
		}
		else if ( titem->primary == "KDEICON" )
		{
			addIconAction->plug( &popup );
			popup.insertSeparator();
			removeDetailAction->plug( &popup );
		}
		else
		{
			addNewFileAction->plug( &popup );
			addExistingFileAction->plug( &popup );
			popup.insertSeparator();
			removeDetailAction->plug( &popup );
		}

		popup.exec( p );

	}
	else if ( pvitem->type() == ProjectItem::File )
	{

		FileItem * fitem = dynamic_cast<FileItem*>( pvitem );
		if ( !fitem )
		{
			kdDebug(9020) << k_funcinfo << "Unable to populate file item menu"
				<< " due to failed cast. " << endl;
			return;
		}

		KPopupMenu popup( i18n( "File: %1" ).arg( fitem->name ), this );

		removeDetailAction->plug( &popup );
		KURL::List urls;
		urls.append(m_widget->selectedSubproject()->path + "/" + fitem->name);
		FileContext context(urls);

		int idSubclassWidget = popup.insertItem(SmallIconSet("qmake_subclass"),
		                                        i18n("Subclassing Wizard...") );
		popup.setWhatsThis(idSubclassWidget, i18n("<b>Subclass widget</b><p>Launches "
		                                          "<b>Subclassing</b> wizard. "
		                                          "It allows to create a subclass from the "
		                                          "class defined in .ui file. "
		                                          "There is also possibility to implement "
		                                          "slots and functions defined in the base "
		                                          "class."));
		int idUISubclasses = popup.insertItem(SmallIconSet("qmake_subclass"),
		                                      i18n("List of Subclasses..."));
		popup.setWhatsThis(idUISubclasses, i18n("<b>List of subclasses</b><p>Shows "
		                                        "subclasses list editor. "
		                                        "There is possibility to add or remove "
		                                        "subclasses from the list."));
		int idUpdateWidgetclass = popup.insertItem(SmallIconSet("qmake_subclass"),
		                                           i18n("Edit ui-Subclass..."));
		popup.setWhatsThis(idUpdateWidgetclass, i18n("<b>Edit ui-subclass</b><p>Launches "
		                                             "<b>Subclassing</b> wizard and prompts "
		                                             "to implement missing in childclass "
		                                             "slots and functions."));
		int idViewUIH = popup.insertItem(SmallIconSet("qmake_ui_h"),
		                                 i18n("Open ui.h File"));
		popup.setWhatsThis(idViewUIH, i18n("<b>Open ui.h file</b><p>Opens .ui.h file "
		                                   "associated with the selected .ui."));

		if (!fitem->name.contains(QRegExp("ui$")) || fitem->is_subst == true)
		{
            popup.removeItem(idUISubclasses);
			popup.removeItem(idViewUIH);
			popup.removeItem(idSubclassWidget);
		}

		if (fitem->uiFileLink.isEmpty())
			popup.removeItem(idUpdateWidgetclass);

		if(fitem->is_subst == false)
			m_part->core()->fillContextMenu( &popup, &context );

		int r = popup.exec( p );

		if(r == idViewUIH)
		{
			m_part->partController()->editDocument(KURL(m_widget->selectedSubproject()->path
			                                            + "/" + QString(fitem->name + ".h")));
		}
		else if (r == idSubclassWidget)
		{
			QStringList newFileNames;
			newFileNames = m_part->languageSupport()->subclassWidget(m_widget->selectedSubproject()->path + "/" + fitem->name);
			if (!newFileNames.empty())
			{
				QDomDocument &dom = *(m_part->projectDom());
				for (uint i=0; i<newFileNames.count(); i++)
				{
					QString srcfile_relpath = newFileNames[i];
					srcfile_relpath.remove(0,m_part->projectDirectory().length());
					QString uifile_relpath = QString(m_widget->selectedSubproject()->path + "/" + fitem->name).remove(0,m_part->projectDirectory().length());
					DomUtil::PairList list = DomUtil::readPairListEntry(dom,"/kdevautoproject/subclassing",
					                                                    "subclass","sourcefile", "uifile");

					list << DomUtil::Pair(srcfile_relpath,uifile_relpath);
					DomUtil::writePairListEntry(dom, "/kdevautoproject/subclassing",
					                            "subclass", "sourcefile", "uifile", list);
					newFileNames[i] = newFileNames[i].replace(QRegExp(m_part->projectDirectory()+"/"),"");
				}
				m_subclasslist = DomUtil::readPairListEntry(dom,"/kdevautoproject/subclassing",
				                                            "subclass","sourcefile", "uifile");
				m_part->addFiles(newFileNames);
			}
		}
		else if (r == idUpdateWidgetclass)
		{
			QString noext = m_widget->selectedSubproject()->path + "/" + fitem->name;

			if (noext.findRev('.')>-1)
				noext = noext.left(noext.findRev('.'));

			QStringList dummy;
			QString uifile = fitem->uiFileLink;

			if (uifile.findRev('/')>-1)
			{
				QStringList uisplit = QStringList::split('/',uifile);
				uifile=uisplit[uisplit.count()-1];
			}

			m_part->languageSupport()->updateWidget(m_widget->selectedSubproject()->path
			                                        + "/" + uifile, noext);
		}
		else if (r == idUISubclasses)
		{
			QDomDocument &dom = *(m_part->projectDom());
			DomUtil::PairList list = DomUtil::readPairListEntry(dom,"/kdevautoproject/subclassing",
			                                                    "subclass","sourcefile", "uifile");
			SubclassesDlg *sbdlg = new SubclassesDlg( QString(m_widget->selectedSubproject()->path + "/" +fitem->name).remove(0,m_part->projectDirectory().length()),
			                                          list, m_part->projectDirectory());

			if (sbdlg->exec())
			{
				QDomElement el = DomUtil::elementByPath( dom,  "/kdevautoproject");
				QDomElement el2 = DomUtil::elementByPath( dom,  "/kdevautoproject/subclassing");
				if ( (!el.isNull()) && (!el2.isNull()) )
				{
					el.removeChild(el2);
				}

				DomUtil::writePairListEntry(dom, "/kdevautoproject/subclassing", "subclass",
				                            "sourcefile", "uifile", list);

				m_subclasslist = DomUtil::readPairListEntry(dom,"/kdevautoproject/subclassing",
				                                            "subclass","sourcefile", "uifile");
			}
		}
	}
}


void AutoDetailsView::slotDetailsExecuted( QListViewItem *item )
{
	if ( !item )
		return ;

	ProjectItem *pvitem = static_cast<ProjectItem*>( item );
	if ( pvitem->type() != ProjectItem::File )
		return ;

	if ( !m_widget->selectedSubproject() )
		return;

	QString dirName = m_widget->selectedSubproject()->path;
	FileItem *fitem = static_cast<FileItem*>( item );
	if(fitem->is_subst)
	{
		fitem->changeSubstitution();
		return;
	}

	m_part->partController()->editDocument( KURL( dirName + "/" + fitem->name ) );
}

void AutoDetailsView::slotSetActiveTarget()
{
	TargetItem * titem = static_cast<TargetItem*>( m_listView->selectedItem() );
	if ( !titem ) return ;

       SubprojectItem * subpitem = m_widget->selectedSubproject();
       if ( !subpitem ) return;

	QString targetPath = subpitem->path + "/" + titem->name;
	targetPath = targetPath.mid( m_part->projectDirectory().length() + 1 );
	kdDebug( 9020 ) << "Setting active " << targetPath << endl;
	m_widget->setActiveTarget( targetPath );
	QDomDocument &dom = *m_part->projectDom();
	DomUtil::writeEntry( dom, "/kdevautoproject/general/activetarget", targetPath );
}

void AutoDetailsView::focusOutEvent(QFocusEvent */*e*/)
{
    m_widget->setLastFocusedView(AutoProjectWidget::DetailsView);
}

#include "autodetailsview.moc"

//kate: indent-mode csands; tab-width 4; space-indent off;
