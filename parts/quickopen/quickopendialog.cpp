/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "quickopendialog.h"
#include "quickopen_part.h"

#include <kdevproject.h>
#include <kdevpartcontroller.h>

#include "doclineedit.h"

#include <klistbox.h>
#include <qregexp.h>

QuickOpenDialog::QuickOpenDialog(QuickOpenPart* part, QWidget* parent, const char* name, bool modal, WFlags fl)
    : QuickOpenDialogBase( parent, name, modal, fl ), m_part( part )
{
    m_fileList = m_part->project()->allFiles();

    m_completion = new KCompletion();
    m_completion->insertItems( m_fileList );
    m_completion->setIgnoreCase( true );

    fileNameEdit->setFocus();

    projectFileList->insertStringList( m_fileList );
    projectFileList->setCurrentItem(0);

    connect(fileNameEdit, SIGNAL(upPressed()), this, SLOT(moveUpInList()));
    connect(fileNameEdit, SIGNAL(downPressed()), this, SLOT(moveDownInList()));
    connect(fileNameEdit, SIGNAL(pgupPressed()), this, SLOT(scrollUpInList()));
    connect(fileNameEdit, SIGNAL(pgdownPressed()), this, SLOT(scrollDownInList()));
    connect(fileNameEdit, SIGNAL(homePressed()), this, SLOT(goToBegin()));
    connect(fileNameEdit, SIGNAL(endPressed()), this, SLOT(goToEnd()));
}

QuickOpenDialog::~QuickOpenDialog()
{
    delete( m_completion );
    m_completion = 0;
}

/*$SPECIALIZATION$*/
void QuickOpenDialog::slotExecuted( QListBoxItem* item )
{
    m_part->partController()->editDocument( m_part->project()->projectDirectory() + "/" + item->text() );
    accept();
}

void QuickOpenDialog::reject()
{
    QDialog::reject();
}

void QuickOpenDialog::accept()
{
    QDialog::accept();
}

void QuickOpenDialog::slotReturnPressed( )
{
/*    if( m_fileList.contains(fileNameEdit->text()) ) {
        m_part->partController()->editDocument( m_part->project()->projectDirectory() + "/" + fileNameEdit->text() );
        accept();
    }*/
    if( projectFileList->currentItem() != -1 ) {
        m_part->partController()->editDocument( m_part->project()->projectDirectory() + "/" + projectFileList->currentText() );
        accept();
    }
}

void QuickOpenDialog::slotTextChanged( const QString & text )
{
    projectFileList->clear();
    projectFileList->insertStringList( m_completion->substringCompletion(text) );
    projectFileList->setCurrentItem(0);
}

void QuickOpenDialog::moveUpInList( )
{
    if (projectFileList->currentItem() == -1)
        projectFileList->setCurrentItem(projectFileList->count() - 1);
    else
        projectFileList->setCurrentItem(projectFileList->currentItem() - 1);
    projectFileList->ensureCurrentVisible();
}

void QuickOpenDialog::moveDownInList( )
{
    if (projectFileList->currentItem() == -1)
        projectFileList->setCurrentItem(0);
    else
        projectFileList->setCurrentItem(projectFileList->currentItem() + 1);
    projectFileList->ensureCurrentVisible();
}

void QuickOpenDialog::scrollUpInList( )
{
    if (projectFileList->currentItem() == -1)
        projectFileList->setCurrentItem(projectFileList->count() - 1);
    else
        projectFileList->setCurrentItem(projectFileList->currentItem() - (projectFileList->numItemsVisible()-1));
    projectFileList->ensureCurrentVisible();
}

void QuickOpenDialog::scrollDownInList( )
{
    if (projectFileList->currentItem() == -1)
        projectFileList->setCurrentItem(0);
    else
        projectFileList->setCurrentItem(projectFileList->currentItem() + (projectFileList->numItemsVisible()-1));
    projectFileList->ensureCurrentVisible();
}

void QuickOpenDialog::goToBegin( )
{
    projectFileList->setCurrentItem(0);
}

void QuickOpenDialog::goToEnd( )
{
    projectFileList->setCurrentItem(projectFileList->count()-1);
}


#include "quickopendialog.moc"

