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
#include <ktextedit.h>


#include "customprojectpart.h"
#include "domutil.h"

CustomManagerWidget::CustomManagerWidget( CustomProjectPart* part, QWidget* parent )
        : CustomManagerWidgetBase( parent ), m_dom( *part->projectDom() )
{
    m_filetypes->setText( DomUtil::readListEntry( m_dom, "kdevcustomproject/filetypes", "filetype" ).join( "\n" ) );
}

CustomManagerWidget::~CustomManagerWidget()
{}

void CustomManagerWidget::accept()
{
    DomUtil::writeListEntry( m_dom, "kdevcustomproject/filetypes", "filetype", QStringList::split( "\n", m_filetypes->text() ) );
}


#include "custommanagerwidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
