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

#include "regexptestpart.h"

#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>

#include "kdevcore.h"
#include "regexptestdlg.h"


typedef KGenericFactory<RegexpTestPart> RegexpTestFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevregexptest, RegexpTestFactory( "kdevregexptest" ) )

RegexpTestPart::RegexpTestPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin("RegExpTest", "regexptest", parent, name ? name : "RegexpTestPart")
{
    setInstance(RegexpTestFactory::instance());
    setXMLFile("kdevregexptest.rc");

    KAction *action;

    action = new KAction( i18n("Debug Regular Expression..."), 0,
                          this, SLOT(slotRegexpTest()),
                          actionCollection(), "tools_regexptest" );

    m_dialog = 0;
}


RegexpTestPart::~RegexpTestPart()
{
    delete m_dialog;
}


void RegexpTestPart::slotRegexpTest()
{
    if (!m_dialog) {
        m_dialog = new RegexpTestDialog(this);
    }

    m_dialog->show();
}


#include "regexptestpart.moc"
