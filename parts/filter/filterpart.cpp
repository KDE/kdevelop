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
#include "kdevplugininfo.h"
#include "shellfilterdlg.h"
#include "shellinsertdlg.h"
#include "kdevfilterIface.h"

static const KDevPluginInfo data("kdevfilter");
K_EXPORT_COMPONENT_FACTORY( libkdevfilter, FilterFactory( data ) )

FilterPart::FilterPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(&data, parent, name ? name : "FilterPart")
{
    setInstance(FilterFactory::instance());
    setXMLFile("kdevfilter.rc");

    KAction *action;

    action = new KAction( i18n("Execute Command..."), 0,
                          this, SLOT(slotShellInsert()),
                          actionCollection(), "tools_insertshell" );
    action->setToolTip(i18n("Execute shell command"));
    action->setWhatsThis(i18n("<b>Execute shell command</b><p>Executes a shell command and outputs its result into the current document."));

    action = new KAction( i18n("Filter Selection Through Command..."), 0,
                          this, SLOT(slotShellFilter()),
                          actionCollection(), "tools_filtershell" );
    action->setToolTip(i18n("Filter selection through a shell command"));
    action->setWhatsThis(i18n("<b>Filter selection through shell command</b><p>Filters selection through a shell command and outputs its result into the current document."));

    m_insertDialog = 0;
    m_filterDialog = 0;

    new KDevFilterIface( this );
    // (void) dcopClient();
}


FilterPart::~FilterPart()
{
    delete m_insertDialog;
    delete m_filterDialog;
}


void FilterPart::slotShellInsert()
{
    /// @todo Disable menu item if no active part

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
    {
        m_insertDialog = new ShellInsertDialog();
        m_insertDialog->setCaption(i18n("Execute Command"));
    }
    if (m_insertDialog->exec()) {
        uint line, col;
        cursoriface->cursorPositionReal(&line, &col);
        editiface->insertText(line, col, m_insertDialog->text());
    }
}


void FilterPart::slotShellFilter()
{
    /// @todo Disable menu item if no active part

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
    {
        m_filterDialog = new ShellFilterDialog();
        m_filterDialog->setCaption(i18n("Filter Selection Through Command"));
    }

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
