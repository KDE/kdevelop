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

#include <ktextedit.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <kfiledialog.h>
#include <keditlistbox.h>
#include <klocale.h>

#include "customprojectpart.h"
#include "domutil.h"

CustomManagerWidget::CustomManagerWidget( CustomProjectPart* part, QWidget* parent )
        : CustomManagerWidgetBase( parent ), m_dom( *part->projectDom() )
{
    m_filetypes->insertStringList( DomUtil::readListEntry( m_dom, "kdevcustomproject/filetypes", "filetype" ) );
    KURLRequester* urlselector = new KURLRequester( );
    urlselector->setMode( KFile::Files | KFile::ExistingOnly | KFile::LocalOnly );
    urlselector->setURL( QString::null );
    urlselector->completionObject() ->setDir( part->projectDirectory() );
    urlselector->fileDialog() ->setURL( KURL( part->projectDirectory() ) );
    m_blacklistBox = new KEditListBox( i18n("Blacklisted files/dirs"), urlselector->customEditor(), this);
    m_blacklistBox->setButtons( KEditListBox::Add | KEditListBox::Remove );
    m_blacklistBox->insertStringList( DomUtil::readListEntry( m_dom, "kdevcustomproject/blacklist","path") );
    grid->addWidget( m_blacklistBox, 0, 1 );
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
