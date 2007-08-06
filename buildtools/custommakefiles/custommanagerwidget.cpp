/***************************************************************************
 *   Copyright (C) 2007 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "custommanagerwidget.h"

#include <qstringlist.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

#include <ktextedit.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <kfiledialog.h>
#include <keditlistbox.h>
#include <klocale.h>
#include <kdebug.h>

#include "customprojectpart.h"
#include "domutil.h"

CustomManagerWidget::CustomManagerWidget( CustomProjectPart* part, QWidget* parent )
        : CustomManagerWidgetBase( parent ), m_part( part), m_dom( *part->projectDom() )
{
    m_filetypes->insertStringList( DomUtil::readListEntry( m_dom, "kdevcustomproject/filetypes", "filetype" ) );
    KURLRequester* urlselector = new KURLRequester( );
    urlselector->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
    urlselector->setURL( QString::null );
    urlselector->completionObject() ->setDir( part->projectDirectory() );
    urlselector->fileDialog() ->setURL( KURL( part->projectDirectory() ) );
    m_blacklistBox = new KEditListBox( i18n("blacklisted files and directories are not"
                         " considered part of the project, even if they fit one of "
                         "the wildcard patterns in the project file list",
                         "Blacklisted files/dirs"), urlselector->customEditor(), this);
    m_blacklistBox->setButtons( KEditListBox::Add | KEditListBox::Remove );
    m_blacklistBox->insertStringList( DomUtil::readListEntry( m_dom, "kdevcustomproject/blacklist","path") );
    grid->addWidget( m_blacklistBox, 0, 1 );
    connect(m_blacklistBox, SIGNAL(added(const QString&)), this, SLOT(checkUrl(const QString&)));
}

void CustomManagerWidget::checkUrl(const QString& url)
{
    kdDebug(9025) << "got file:" << url << endl;
    if( !QFileInfo(url).isRelative() )
    {
        kdDebug(9025) << "seems to be non-relative" << endl;
        QString relpath = m_part->relativeToProject( url );
        QListBoxItem* item = m_blacklistBox->listBox()->findItem( url );
        m_blacklistBox->listBox()->takeItem( item );
        kdDebug(9025) << "relative path:" << relpath << endl;
        if( !relpath.isEmpty() )
            m_blacklistBox->insertItem( relpath );
    }
}

CustomManagerWidget::~CustomManagerWidget()
{
}

void CustomManagerWidget::accept()
{
    DomUtil::writeListEntry( m_dom, "kdevcustomproject/filetypes", "filetype",
                            m_filetypes->items() );
    DomUtil::writeListEntry( m_dom, "kdevcustomproject/blacklist", "path",
                            m_blacklistBox->items() );
}


#include "custommanagerwidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
