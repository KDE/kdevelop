/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filterpart.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/viewcursorinterface.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "shellfilterdlg.h"
#include "shellinsertdlg.h"
#include "kdevfilteriface.h"

K_EXPORT_COMPONENT_FACTORY( libkdevfilter, FilterFactory( "kdevfilter" ) );

FilterPart::FilterPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin("Filter", "filter", parent, name ? name : "FilterPart")
{    
    setInstance(FilterFactory::instance());
    setXMLFile("kdevfilter.rc");

    KAction *action;
    
    action = new KAction( i18n("Execute Command..."), 0,
                          this, SLOT(slotShellInsert()),
                          actionCollection(), "tools_insertshell" );
    core()->insertNewAction( action );

    action = new KAction( i18n("Filter Selection Through Command..."), 0,
                          this, SLOT(slotShellFilter()),
                          actionCollection(), "tools_filtershell" );
    core()->insertNewAction( action );

    m_insertDialog = 0;
    m_filterDialog = 0;    
    
    new KDevFilterIface( this );
    (void) dcopClient();
}


FilterPart::~FilterPart()
{
    delete m_insertDialog;
    delete m_filterDialog;
}


void FilterPart::slotShellInsert()
{
    // TODO: Disable menu item if no active part

    KParts::ReadWritePart *part
        = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    QWidget *view = partController()->activeWidget();
    if (!part || !view) {
        kdDebug(9029) << "no rw part" << endl;
        return;
    }

    KTextEditor::EditInterface *editiface
        = dynamic_cast<KTextEditor::EditInterface*>(part);
    if (!editiface) {
        kdDebug(9029) << "no edit" << endl;
        return;
    }

    KTextEditor::ViewCursorInterface *cursoriface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(view);
    if (!cursoriface) {
        kdDebug(9029) << "no viewcursor" << endl;
        return;
    }

    if (!m_insertDialog)
        m_insertDialog = new ShellInsertDialog();

    if (m_insertDialog->exec()) {
        uint line, col;
        cursoriface->cursorPositionReal(&line, &col);
        editiface->insertText(line, col, m_insertDialog->text());
    }
}


void FilterPart::slotShellFilter()
{
    // TODO: Disable menu item if no active part

    KParts::ReadWritePart *part
        = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    QWidget *view = partController()->activeWidget();
    if (!part || !view) {
        kdDebug(9029) << "no rw part" << endl;
        return;
    }

    KTextEditor::EditInterface *editiface
        = dynamic_cast<KTextEditor::EditInterface*>(part);
    if (!editiface) {
        kdDebug(9029) << "no edit" << endl;
        return;
    }

    KTextEditor::ViewCursorInterface *cursoriface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(view);
    if (!cursoriface) {
        kdDebug(9029) << "no viewcursor" << endl;
        return;
    }

    KTextEditor::SelectionInterface *selectioniface
        = dynamic_cast<KTextEditor::SelectionInterface*>(part);
    if (!selectioniface) {
        kdDebug(9029) << "no selection" << endl;
        return;
    }

    if (!m_filterDialog)
        m_filterDialog = new ShellFilterDialog();

    kdDebug(9029) << "Old text: " << selectioniface->selection()<< endl;
    
    m_filterDialog->setText(selectioniface->selection());
    
    if (m_filterDialog->exec()) {
        uint line, col;
        // OUCH: KTextEditor doesn't allow to find out
        // where the selection is
        selectioniface->removeSelectedText();
        cursoriface->cursorPositionReal(&line, &col);
        kdDebug(9029) << "New text: " << m_filterDialog->text() << endl;
        editiface->insertText(line, col, m_filterDialog->text());
    }
}

#include "filterpart.moc"
