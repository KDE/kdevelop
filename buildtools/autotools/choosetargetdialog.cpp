/***************************************************************************
                             -------------------
    begin                : 29.11.2002
    copyright            : (C) 2002 by Victor Rder
    email                : victor_roeder@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "choosetargetdialog.h"

#include <qcheckbox.h>
#include <qwidget.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qfileinfo.h>

#include <kcombobox.h>
#include <kdialog.h>
#include <kdebug.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <ksqueezedtextlabel.h>
#include <kurl.h>

#include "choosetargetdlgbase.h"
#include "autodetailsview.h"
#include "autolistviewitems.h"
#include "autosubprojectview.h"
#include "misc.h"
#include "autoprojectwidget.h"
#include "autoprojectpart.h"

#include "kdevpartcontroller.h"

class ChooseTargetDialog::Private
{
public:
	AutoProjectWidget* widget;
	AutoProjectPart* part;
	QStringList fileList;
	QPtrList<SubprojectItem> subprojectList;
	SubprojectItem* chosenSubproject;
	TargetItem* chosenTarget;
	ChooseTargetDlgBase* baseUI;
};

ChooseTargetDialog::ChooseTargetDialog ( AutoProjectWidget* widget, AutoProjectPart* part,
                                         QStringList fileList, QWidget* parent, const char* name )
: KDialogBase( parent, name, false, i18n("Automake Manager - Choose Target"),
               Ok | Cancel, KDialogBase::Ok, true /* seperator */ )

{
	Q_UNUSED( parent );
	Q_UNUSED( name );
	d = new ChooseTargetDialog::Private;
	d->widget = widget;
	d->part = part;
	d->fileList = fileList;
	d->subprojectList = widget->allSubprojectItems();
	d->baseUI = new ChooseTargetDlgBase( this, "base ui" );
	setMainWidget( d->baseUI );
	
	d->baseUI->subprojectComboBox->setAutoCompletion( true );
	d->baseUI->targetComboBox->setAutoCompletion( true );
	d->baseUI->newFileList->header()->hide();
	d->baseUI->newFileList->addColumn( QString::null );
	d->baseUI->newFileList->setSorting(-1);

	setIcon ( SmallIcon ( "target_kdevelop" ) );
	
	
	QPtrListIterator<SubprojectItem> sit(d->subprojectList);
	for ( ; (*sit); ++sit )
	{
		QPtrList<TargetItem> targetList = (*sit)->targets;
		QPtrListIterator<TargetItem> targetIt(targetList);

		// Only insert Subproject which have a "regular" target
		for ( ; (*targetIt); ++targetIt )
		{
			QString titemPrimary = (*targetIt)->primary;
			if ( titemPrimary == "PROGRAMS" || titemPrimary == "LIBRARIES" ||
			     titemPrimary == "LTLIBRARIES" || titemPrimary == "JAVA" )
			{
				d->baseUI->subprojectComboBox->insertItem ( SmallIcon ( "folder" ), (*sit)->subdir );
			}
		}
	}

	if ( d->widget->activeTarget() && d->widget->activeSubproject() )
	{
		d->chosenTarget = d->widget->activeTarget();
		//kdDebug ( 9000 ) << "1) Chosen target is " << d->chosenTarget->name << endl;
		d->chosenSubproject = widget->activeSubproject();
		d->baseUI->chosenTargetLabel->setText( ( widget->activeSubproject()->path + "/<b>" +
		                                         d->widget->activeTarget()->name + "</b>" )
		                                       .mid( d->part->projectDirectory().length() + 1 ) );
		d->baseUI->subprojectComboBox->setEnabled( false );
		d->baseUI->targetComboBox->setEnabled( false );

		d->baseUI->subprojectComboBox->setCurrentItem( widget->activeSubproject()->subdir );
		slotSubprojectChanged( widget->activeSubproject()->subdir );
	}
	else
	{
		d->baseUI->activeTargetRadioButton->toggle();
		d->baseUI->activeTargetRadioButton->setEnabled ( false );
		d->baseUI->neverAskAgainCheckbox->setEnabled ( false );

		slotSubprojectChanged ( d->baseUI->subprojectComboBox->text(0) );
	}

	QStringList::iterator it;
	QString fileName;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		int pos = ( *it ).findRev('/');
		if (pos != -1)
			fileName = ( *it ).mid(pos+1);
		else
			fileName = ( *it );


		d->baseUI->newFileList->insertItem( new QListViewItem( d->baseUI->newFileList, fileName ) );
	}
	
	connect ( d->baseUI->subprojectComboBox, SIGNAL ( activated ( const QString& ) ),
	          this, SLOT( slotSubprojectChanged ( const QString& ) ) );
	connect ( d->baseUI->targetComboBox, SIGNAL ( activated ( const QString& ) ),
	          this, SLOT( slotTargetChanged ( const QString& ) ) );
}


ChooseTargetDialog::~ChooseTargetDialog()
{
}

void ChooseTargetDialog::slotSubprojectChanged ( const QString& name )
{
	d->chosenTarget = 0;
	SubprojectItem* spitem = d->subprojectList.first();

	for ( ; spitem; spitem = d->subprojectList.next() )
	{
		if ( spitem->subdir == name )
		{
			QPtrList <TargetItem> targetList = spitem->targets;
			TargetItem* titem = targetList.first();

			d->baseUI->targetComboBox->clear();

/*			choosenSubprojectLabel->setText ( ( spitem->path + "<b>" + ->name + "</b>" ).mid ( d->widget->projectDirectory().length() + 1 ) );*/

			d->chosenSubproject = spitem;

			for ( ; titem; titem = targetList.next() )
			{
				if ( titem->primary == "PROGRAMS" || titem->primary == "LIBRARIES" ||
						titem->primary == "LTLIBRARIES" || titem->primary == "JAVA" )
				{
					d->baseUI->targetComboBox->insertItem ( SmallIcon ( "target_kdevelop" ), titem->name );

					//d->baseUI->targetComboBox->addToHistory ( titem->name );

					// if the Active Target is in the currently selected Subproject
					if ( d->widget->activeTarget() &&
						titem->name == d->widget->activeTarget()->name )
					{
						d->baseUI->targetComboBox->setCurrentItem( titem->name );
						d->baseUI->chosenTargetLabel->setText( ( spitem->path + "/<b>" + titem->name + "</b>" ).mid( d->part->projectDirectory().length() + 1 ) );
						d->chosenTarget = titem;
						//kdDebug ( 9000 ) << "2) Chosen target is " << d->chosenTarget->name << endl;
					}
					else
					{
						//d->baseUI->targetComboBox->setCurrentItem ( 0 );
						if ( !d->chosenTarget )
						{
							d->baseUI->chosenTargetLabel->setText( ( spitem->path + "/<b>" + titem->name + "</b>")
							                                       .mid( d->part->projectDirectory().length() + 1 ) );
							//choosenSubprojectLabel->setText ( ( spitem->path + "<b>" + titem->name + "</b>" ).mid ( d->widget->projectDirectory().length() + 1 ) );

							d->chosenTarget = titem;
						}
						//kdDebug ( 9000 ) << "2a) Chosen target is " << d->chosenTarget->name << endl;
					}
				}
			}

			break;
		}
	}
}

void ChooseTargetDialog::slotTargetChanged( const QString& name )
{
	d->baseUI->chosenTargetLabel->setText( ( d->chosenSubproject->path + "/<b>" + name + "</b>" )
	                                        .mid( d->part->projectDirectory().length() + 1 ) );

	QPtrList <TargetItem> targetList = d->chosenSubproject->targets;
	TargetItem* titem = targetList.first();

	for ( ; titem; titem = targetList.next() )
	{
		if ( titem->name == name )
		{
			d->chosenTarget = titem;
			//kdDebug ( 9000 ) << "4) Chosen target is " << d->chosenTarget->name << endl;

			break;
		}
	}
}

void ChooseTargetDialog::slotOk()
{
	if ( d->baseUI->activeTargetRadioButton->isChecked() )
	{
		d->chosenTarget = d->widget->activeTarget();
		d->chosenSubproject = d->widget->activeSubproject();
	}
	if ( !d->chosenSubproject || !d->chosenTarget )
		return;

	//kdDebug ( 9000 ) << "3) Chosen target is " << d->chosenTarget->name << endl;

	QStringList newFileList;
	QStringList::iterator it;
	QString directory, fileName;

	for ( it = d->fileList.begin(); it != d->fileList.end(); ++it )
	{
		bool found = false;

		int pos = ( *it ).findRev('/');
		if (pos != -1)
		{
			directory = ( *it ).left(pos);
			fileName = ( *it ).mid(pos+1);
		}
		else
		{
			fileName = ( *it );
		}

		FileItem * fitem = d->chosenTarget->sources.first();
		for ( ; fitem; fitem = d->chosenTarget->sources.next() )
		{
			if ( fitem->name == fileName )
			{
				KMessageBox::error ( this, i18n ( "The file %1 already exists in the chosen target.\nThe file will be created but will not be added to the target.\n"
				                                  "Rename the file and select 'Add Existing Files' from the Automake Manager." ).arg ( fitem->name ),
				                     i18n ( "Error While Adding Files" ) );
				found = true;
			}
		}

		if ( !found )
		{
            /// \FIXME a quick hack to prevent adding header files to _SOURCES
            /// and display them in noinst_HEADERS
			if (AutoProjectPrivate::isHeader(fileName) &&
			    ( d->chosenTarget->primary == "PROGRAMS" || d->chosenTarget->primary == "LIBRARIES" ||  d->chosenTarget->primary == "LTLIBRARIES" ) )
			{
				kdDebug ( 9020 ) << "Ignoring header file and adding it to noinst_HEADERS: " << fileName << endl;
				TargetItem* noinst_HEADERS_item = d->widget->getSubprojectView()->findNoinstHeaders(d->chosenSubproject);
				FileItem *fitem = d->widget->createFileItem( fileName, d->chosenSubproject );
				noinst_HEADERS_item->sources.append( fitem );
				noinst_HEADERS_item->insertItem( fitem );
				
				QString varname = "noinst_HEADERS";
				d->chosenSubproject->variables[ varname ] += ( " " + fileName );
				
				QMap<QString, QString> replaceMap;
				replaceMap.insert( varname, d->chosenSubproject->variables[ varname ] );
				
				AutoProjectTool::modifyMakefileam( d->chosenSubproject->path + "/Makefile.am", replaceMap );
			}
			else
			{
				fitem = d->widget->createFileItem( fileName,d->chosenSubproject );
				d->chosenTarget->sources.append( fitem );
				d->chosenTarget->insertItem( fitem );
				
				QString canontargetname = AutoProjectTool::canonicalize( d->chosenTarget->name );
				QString varname = canontargetname + "_SOURCES";
				d->chosenSubproject->variables[ varname ] += ( " " + fileName );
				
				QMap<QString, QString> replaceMap;
				replaceMap.insert( varname, d->chosenSubproject->variables[ varname ] );
				
				AutoProjectTool::modifyMakefileam( d->chosenSubproject->path + "/Makefile.am", replaceMap );
			}
			newFileList.append ( d->chosenSubproject->path.mid ( d->part->projectDirectory().length() + 1 ) + "/" + fileName );
		}

		if ( directory.isEmpty() || directory != d->chosenSubproject->subdir )
		{
			KShellProcess proc("/bin/sh");

			proc << "mv";
			proc << KShellProcess::quote( d->part->projectDirectory() + "/" + directory + "/" + fileName );
			proc << KShellProcess::quote( d->chosenSubproject->path + "/" + fileName );
			proc.start(KProcess::DontCare);
		}

        // why open the files?!
//		d->part->partController()->editDocument ( KURL ( d->chosenSubproject->path + "/" + fileName ) );

		found = false;
	}

	d->widget->emitAddedFiles( newFileList );
}

TargetItem* ChooseTargetDialog::chosenTarget()
{
	return d->chosenTarget;
}

SubprojectItem* ChooseTargetDialog::chosenSubproject()
{
	return d->chosenSubproject;
}

bool ChooseTargetDialog::alwaysUseActiveTarget() const
{
	return d->baseUI->neverAskAgainCheckbox->isChecked();
}


#include "choosetargetdialog.moc"
//kate: indent-mode csands; tab-width 4;
