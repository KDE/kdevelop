/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "debuggerconfigwidget.h"

#include "debuggerpart.h"
#include "kdevproject.h"

#include "domutil.h"
#include <kurlrequester.h>
#include <klineedit.h>

#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qradiobutton.h>

namespace GDBDebugger
{

DebuggerConfigWidget::DebuggerConfigWidget(DebuggerPart* part, QWidget *parent, const char *name)
    : DebuggerConfigWidgetBase(parent, name), dom(*part->projectDom())
{
    QFontMetrics fm(programArgs_edit->fontMetrics());
    programArgs_edit->setMinimumWidth(fm.width('X')*30);

    gdbPath_edit->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);

    programArgs_edit->setText(  DomUtil::readEntry(dom, "/kdevdebugger/general/programargs"));
    gdbPath_edit->setURL(      DomUtil::readEntry(dom, "/kdevdebugger/general/gdbpath"));

    QString shell =             DomUtil::readEntry(dom, "/kdevdebugger/general/dbgshell","no_value");
    if( shell == QString("no_value") )
    {
        QFileInfo info( part->project()->buildDirectory() + "/libtool" );
        if( info.exists() ) {
            shell = "libtool";
        } else {
            // Try one directory up.
            info.setFile( part->project()->buildDirectory() + "/../libtool" );
            if( info.exists() ) {
                shell = "../libtool";
            } else {
                // Give up.
                shell = QString::null;
            }
        }
    }
    debuggingShell_edit->setURL( shell );

    // Use setFile instead?
    configGdbScript_edit->setURL( DomUtil::readEntry(dom, "/kdevdebugger/general/configGdbScript"));
    runShellScript_edit ->setURL( DomUtil::readEntry(dom, "/kdevdebugger/general/runShellScript"));
    runGdbScript_edit   ->setURL( DomUtil::readEntry(dom, "/kdevdebugger/general/runGdbScript"));

    displayStaticMembers_box->setChecked(  DomUtil::readBoolEntry(dom, "/kdevdebugger/display/staticmembers", false));
    asmDemangle_box->setChecked(           DomUtil::readBoolEntry(dom, "/kdevdebugger/display/demanglenames", true));
    breakOnLoadingLibrary_box->setChecked( DomUtil::readBoolEntry(dom, "/kdevdebugger/general/breakonloadinglibs", true));
    dbgTerminal_box->setChecked(           DomUtil::readBoolEntry(dom, "/kdevdebugger/general/separatetty", false));
    enableFloatingToolBar_box->setChecked( DomUtil::readBoolEntry(dom, "/kdevdebugger/general/floatingtoolbar", false));
    int outputRadix = DomUtil::readIntEntry(dom, "/kdevdebugger/display/outputradix", 10);
    
    switch (outputRadix)
    {
    case 8:
      outputRadixOctal->setChecked(true);
      break;
    case 16:
      outputRadixHexadecimal->setChecked(true);
      break;
    case 10:
    default:
      outputRadixDecimal->setChecked(true);
      break;
    }
    
    
    // ??? DomUtil::readEntry(dom, "/kdevdebugger/general/allowforcedbpset");

    resize(sizeHint());
}


DebuggerConfigWidget::~DebuggerConfigWidget()
{}


void DebuggerConfigWidget::accept()
{
    DomUtil::writeEntry(dom, "/kdevdebugger/general/programargs", programArgs_edit->text());
    DomUtil::writeEntry(dom, "/kdevdebugger/general/gdbpath", gdbPath_edit->url());
    DomUtil::writeEntry(dom, "/kdevdebugger/general/dbgshell", debuggingShell_edit->url());

    DomUtil::writeEntry(dom, "/kdevdebugger/general/configGdbScript", configGdbScript_edit->url());
    DomUtil::writeEntry(dom, "/kdevdebugger/general/runShellScript", runShellScript_edit ->url());
    DomUtil::writeEntry(dom, "/kdevdebugger/general/runGdbScript", runGdbScript_edit   ->url());

    DomUtil::writeBoolEntry(dom, "/kdevdebugger/display/staticmembers", displayStaticMembers_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdebugger/display/demanglenames", asmDemangle_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdebugger/general/breakonloadinglibs", breakOnLoadingLibrary_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdebugger/general/separatetty", dbgTerminal_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdebugger/general/floatingtoolbar", enableFloatingToolBar_box->isChecked());
    
    int outputRadix;
    if (outputRadixOctal->isChecked())
      outputRadix = 8;
    else if (outputRadixHexadecimal->isChecked())
      outputRadix = 16;
    else
      outputRadix = 10;
      
   DomUtil::writeIntEntry(dom, "/kdevdebugger/display/outputradix", outputRadix);
}

}

#include "debuggerconfigwidget.moc"
