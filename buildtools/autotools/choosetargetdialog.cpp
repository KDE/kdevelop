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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qfileinfo.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <klistbox.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <ksqueezedtextlabel.h>
#include <kurl.h>

#include "choosetargetdialog.h"

#include "autodetailsview.h"
#include "autolistviewitems.h"
#include "autosubprojectview.h"

#include "misc.h"
#include "autoprojectwidget.h"
#include "autoprojectpart.h"

#include "kdevpartcontroller.h"


ChooseTargetDialog::ChooseTargetDialog ( AutoProjectWidget* widget, AutoProjectPart* part, QStringList fileList, QWidget* parent, const char* name )
  : ChooseTargetDlgBase ( parent, name, false, 0 ), m_choosenSubproject ( 0 ), m_choosenTarget ( 0 )
{
	m_widget = widget;
	m_part = part;
	m_fileList = fileList;
	m_subprojectList = widget->allSubprojectItems();

	subprojectComboBox->setAutoCompletion ( true );
	targetComboBox->setAutoCompletion ( true );

	setIcon ( SmallIcon ( "target_kdevelop" ) );
	/*QStringList historyList;*/

	SubprojectItem* spitem = m_subprojectList.first();

	for ( ; spitem; spitem = m_subprojectList.next() )
	{
		QPtrList <TargetItem> targetList = spitem->targets;
		TargetItem* titem = targetList.first();

		// Only insert Subproject which have a "regular" target
		for ( ; titem; titem = targetList.next() )
		{
			if ( titem->primary == "PROGRAMS" || titem->primary == "LIBRARIES" ||
					titem->primary == "LTLIBRARIES" || titem->primary == "JAVA" )
			{
				subprojectComboBox->insertItem ( SmallIcon ( "folder" ), spitem->subdir );

				//historyList.append ( spitem->subdir );
				//subprojectComboBox->addToHistory ( spitem->subdir );

				break;
			}
		}
	}

	//subprojectComboBox->setHistoryItems ( historyList, true );

	if ( widget->activeTarget() && widget->activeSubproject() )
	{
		m_choosenTarget = widget->activeTarget();
		//kdDebug ( 9000 ) << "1) Chosen target is " << m_choosenTarget->name << endl;
		m_choosenSubproject = widget->activeSubproject();
		choosenTargetLabel->setText ( ( widget->activeSubproject()->path + "/<b>" + m_widget->activeTarget()->name + "</b>" ).mid ( m_part->projectDirectory().length() + 1 ) );
		subprojectComboBox->setEnabled ( false );
		targetComboBox->setEnabled ( false );

		subprojectComboBox->setCurrentItem ( widget->activeSubproject()->subdir );
		slotSubprojectChanged ( widget->activeSubproject()->subdir );
	}
	else
	{
		activeTargetRadioButton->setChecked ( false );
		chooseTargetRadioButton->setChecked ( true );
		activeTargetRadioButton->setEnabled ( false );
		neverAskAgainCheckBox->setEnabled ( false );

		slotSubprojectChanged ( subprojectComboBox->text(0) );
	}

	QStringList::iterator it;
	QString fileName;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		int pos = ( *it ).findRev('/');
		if (pos != -1)
		{
			fileName = ( *it ).mid(pos+1);
		}
		else
		{
			fileName = ( *it );
		}

		newFilesListBox->insertItem ( SmallIcon ( "document" ), fileName );
	}

	connect ( activeTargetRadioButton, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotActiveTargetToggled ( bool ) ) );
	connect ( chooseTargetRadioButton, SIGNAL ( toggled ( bool ) ), this, SLOT ( slotChooseTargetToggled ( bool ) ) );

	connect ( subprojectComboBox, SIGNAL ( activated ( const QString& ) ), this, SLOT ( slotSubprojectChanged ( const QString& ) ) );
	connect ( targetComboBox, SIGNAL ( activated ( const QString& ) ), this, SLOT ( slotTargetChanged ( const QString& ) ) );
}


ChooseTargetDialog::~ChooseTargetDialog()
{
}

void ChooseTargetDialog::slotSubprojectChanged ( const QString& name )
{
	m_choosenTarget = 0;
	SubprojectItem* spitem = m_subprojectList.first();

	for ( ; spitem; spitem = m_subprojectList.next() )
	{
		if ( spitem->subdir == name )
		{
			QPtrList <TargetItem> targetList = spitem->targets;
			TargetItem* titem = targetList.first();

			targetComboBox->clear();

/*			choosenSubprojectLabel->setText ( ( spitem->path + "<b>" + ->name + "</b>" ).mid ( m_widget->projectDirectory().length() + 1 ) );*/

			m_choosenSubproject = spitem;

			for ( ; titem; titem = targetList.next() )
			{
				if ( titem->primary == "PROGRAMS" || titem->primary == "LIBRARIES" ||
						titem->primary == "LTLIBRARIES" || titem->primary == "JAVA" )
				{
					targetComboBox->insertItem ( SmallIcon ( "target_kdevelop" ), titem->name );

					//targetComboBox->addToHistory ( titem->name );

					// if the Active Target is in the currently selected Subproject
					if ( m_widget->activeTarget() &&
						titem->name == m_widget->activeTarget()->name )
					{
						targetComboBox->setCurrentItem ( titem->name );
						choosenTargetLabel->setText ( ( spitem->path + "/<b>" + titem->name + "</b>" ).mid ( m_part->projectDirectory().length() + 1 ) );
						//choosenSubprojectLabel->setText ( ( spitem->path + "<b>" + titem->name + "</b>" ).mid ( m_widget->projectDirectory().length() + 1 ) );
						m_choosenTarget = titem;
						//kdDebug ( 9000 ) << "2) Chosen target is " << m_choosenTarget->name << endl;
					}
					else
					{
						//targetComboBox->setCurrentItem ( 0 );
						if ( !m_choosenTarget )
						{
							choosenTargetLabel->setText ( ( spitem->path + "/<b>" + titem->name + "</b>" ).mid ( m_part->projectDirectory().length() + 1 ) );
							//choosenSubprojectLabel->setText ( ( spitem->path + "<b>" + titem->name + "</b>" ).mid ( m_widget->projectDirectory().length() + 1 ) );

							m_choosenTarget = titem;
						}
						//kdDebug ( 9000 ) << "2a) Chosen target is " << m_choosenTarget->name << endl;
					}
				}
			}

			break;
		}
	}
}

void ChooseTargetDialog::slotTargetChanged ( const QString& name )
{
	choosenTargetLabel->setText ( ( m_choosenSubproject->path + "/<b>" + name + "</b>" ).mid ( m_part->projectDirectory().length() + 1 ) );

	QPtrList <TargetItem> targetList = m_choosenSubproject->targets;
	TargetItem* titem = targetList.first();

	for ( ; titem; titem = targetList.next() )
	{
		if ( titem->name == name )
		{
			m_choosenTarget = titem;
			//kdDebug ( 9000 ) << "4) Chosen target is " << m_choosenTarget->name << endl;

			break;
		}
	}
}

void ChooseTargetDialog::slotActiveTargetToggled(bool on)
{
	if ( on )
	{
		chooseTargetRadioButton->setChecked ( false );
/*		choosenTargetGroupBox->setEnabled ( false );*/
		subprojectComboBox->setEnabled ( false );
		targetComboBox->setEnabled ( false );
	}
	else
	{
		chooseTargetRadioButton->setChecked ( true );
/*		choosenTargetGroupBox->setEnabled ( true );*/
		subprojectComboBox->setEnabled ( true );
		targetComboBox->setEnabled ( true );
	}
}

void ChooseTargetDialog::slotChooseTargetToggled(bool on)
{
	if ( on )
	{
		activeTargetRadioButton->setChecked ( false );
/*		choosenTargetGroupBox->setEnabled ( true );*/
		subprojectComboBox->setEnabled ( true );
		targetComboBox->setEnabled ( true );
	}
	else
	{
		activeTargetRadioButton->setChecked ( true );
/*		choosenTargetGroupBox->setEnabled ( false );*/
		subprojectComboBox->setEnabled ( false );
		targetComboBox->setEnabled ( false );
	}
}

void ChooseTargetDialog::accept ()
{
	if ( activeTargetRadioButton->isChecked() )
	{
		m_choosenTarget = m_widget->activeTarget();
		m_choosenSubproject = m_widget->activeSubproject();
	}
	if ( !m_choosenSubproject || !m_choosenTarget )
		return;

	//kdDebug ( 9000 ) << "3) Chosen target is " << m_choosenTarget->name << endl;

	QStringList newFileList;
	QStringList::iterator it;
	QString directory, fileName;

	for ( it = m_fileList.begin(); it != m_fileList.end(); ++it )
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

		FileItem * fitem = m_choosenTarget->sources.first();
		for ( ; fitem; fitem = m_choosenTarget->sources.next() )
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
                ( m_choosenTarget->primary == "PROGRAMS" || m_choosenTarget->primary == "LIBRARIES" ||  m_choosenTarget->primary == "LTLIBRARIES" ) )
            {
                kdDebug ( 9020 ) << "Ignoring header file and adding it to noinst_HEADERS: " << fileName << endl;
                TargetItem* noinst_HEADERS_item = m_widget->getSubprojectView()->findNoinstHeaders(m_choosenSubproject);
                FileItem *fitem = m_widget->createFileItem( fileName, m_choosenSubproject );
                noinst_HEADERS_item->sources.append( fitem );
                noinst_HEADERS_item->insertItem( fitem );

                QString varname = "noinst_HEADERS";
                m_choosenSubproject->variables[ varname ] += ( " " + fileName );

                QMap<QString, QString> replaceMap;
                replaceMap.insert( varname, m_choosenSubproject->variables[ varname ] );

                AutoProjectTool::modifyMakefileam( m_choosenSubproject->path + "/Makefile.am", replaceMap );
            }
            else
            {
                fitem = m_widget->createFileItem( fileName,m_choosenSubproject );
                m_choosenTarget->sources.append( fitem );
                m_choosenTarget->insertItem( fitem );

                QString canontargetname = AutoProjectTool::canonicalize( m_choosenTarget->name );
                QString varname = canontargetname + "_SOURCES";
                m_choosenSubproject->variables[ varname ] += ( " " + fileName );

                QMap<QString, QString> replaceMap;
                replaceMap.insert( varname, m_choosenSubproject->variables[ varname ] );

                AutoProjectTool::modifyMakefileam( m_choosenSubproject->path + "/Makefile.am", replaceMap );
            }
			newFileList.append ( m_choosenSubproject->path.mid ( m_part->projectDirectory().length() + 1 ) + "/" + fileName );
		}

		if ( directory.isEmpty() || directory != m_choosenSubproject->subdir )
		{
			KShellProcess proc("/bin/sh");

			proc << "mv";
			proc << KShellProcess::quote( m_part->projectDirectory() + "/" + directory + "/" + fileName );
			proc << KShellProcess::quote( m_choosenSubproject->path + "/" + fileName );
			proc.start(KProcess::DontCare);
		}

        // why open the files?!
//		m_part->partController()->editDocument ( KURL ( m_choosenSubproject->path + "/" + fileName ) );

		found = false;
	}

	m_widget->emitAddedFiles( newFileList );

	QDialog::accept();
}

TargetItem* ChooseTargetDialog::choosenTarget()
{
	return m_choosenTarget;
}

SubprojectItem* ChooseTargetDialog::choosenSubproject()
{
	return m_choosenSubproject;
}

void ChooseTargetDialog::slotSubprojectTextChanged ( const QString& text )
{
    Q_UNUSED( text );
	//KCompletionBox box = subprojectComboBox->
}
