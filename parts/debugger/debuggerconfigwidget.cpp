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

#include <qcheckbox.h>
#include <qlineedit.h>
#include "domutil.h"


DebuggerConfigWidget::DebuggerConfigWidget(QDomDocument &projectDom, QWidget *parent, const char *name)
    : DebuggerConfigWidgetBase(parent, name), dom(projectDom)
{
    QFontMetrics fm(programArgs_edit->fontMetrics());
    programArgs_edit->setMinimumWidth(fm.width('X')*30);
 
    programArgs_edit->setText(DomUtil::readEntry(dom, "/kdevdebugger/general/programargs"));
    gdbPath_edit->setText(DomUtil::readEntry(dom, "/kdevdebugger/general/gdbpath"));
    debuggingShell_edit->setText(DomUtil::readEntry(dom, "/kdevdebugger/general/dbgshell"));

    displayStaticMembers_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevdebugger/display/staticmembers"));
    asmDemangle_box->setChecked(!DomUtil::readBoolEntry(dom, "/kdevdebugger/display/manglednames"));
    breakOnLoadingLibrary_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevdebugger/general/breakonloadinglibs"));
    dbgTerminal_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevdebugger/general/separatetty"));
    enableFloatingToolBar_box->setChecked(DomUtil::readBoolEntry(dom, "/kdevdebugger/general/floatingtoolbar"));
    // ??? DomUtil::readEntry(dom, "/kdevdebugger/general/allowforcedbpset");

    resize(sizeHint());
}


DebuggerConfigWidget::~DebuggerConfigWidget()
{}


void DebuggerConfigWidget::accept()
{
    DomUtil::writeEntry(dom, "/kdevdebugger/general/programargs", programArgs_edit->text());
    DomUtil::writeEntry(dom, "/kdevdebugger/general/gdbpath", gdbPath_edit->text());
    DomUtil::writeEntry(dom, "/kdevdebugger/general/dbgshell", debuggingShell_edit->text());

    DomUtil::writeBoolEntry(dom, "/kdevdebugger/display/staticmembers", displayStaticMembers_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdebugger/display/manglednames", asmDemangle_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdebugger/general/breakonloadinglibs", breakOnLoadingLibrary_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdebugger/general/separatetty", dbgTerminal_box->isChecked());
    DomUtil::writeBoolEntry(dom, "/kdevdebugger/general/floatingtoolbar", enableFloatingToolBar_box->isChecked());
}

#include "debuggerconfigwidget.moc"
