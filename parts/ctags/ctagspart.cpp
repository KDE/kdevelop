/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>

#include "kdevcore.h"
#include "ctagsdlg.h"
#include "ctagspart.h"

typedef KGenericFactory<CTagsPart> CTagsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevctags, CTagsFactory( "kdevctags" ) );

CTagsPart::CTagsPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin(parent, name)
{
    setInstance(CTagsFactory::instance());
    setXMLFile("kdevctags.rc");

    KAction *action;
    
    action = new KAction( i18n("CTags..."), 0,
                          this, SLOT(slotTags()),
                          actionCollection(), "tools_ctags" );

    m_dialog = 0;
}


CTagsPart::~CTagsPart()
{
    delete m_dialog;
}


void CTagsPart::slotTags()
{
    if (!m_dialog) {
        m_dialog = new CTagsDialog(this);
    }

    m_dialog->show();
}


#include "ctagspart.moc"
