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

#include "kdevcore.h"
#include "regexptestdlg.h"
#include "regexptestfactory.h"
#include "regexptestpart.h"


RegexpTestPart::RegexpTestPart(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
    setInstance(RegexpTestFactory::instance());
    setXMLFile("kdevregexptest.rc");

    KAction *action;
    
    action = new KAction( i18n("Debug regular expression..."), 0,
                          this, SLOT(slotRegexpTest()),
                          actionCollection(), "edit_regexptest" );

    m_dialog = 0;
}


RegexpTestPart::~RegexpTestPart()
{
    delete m_dialog;
}


void RegexpTestPart::slotRegexpTest()
{
    if (!m_dialog) {
        m_dialog = new RegexpTestDialog(/*this*/);
    }

    m_dialog->show();
}


#include "regexptestpart.moc"
