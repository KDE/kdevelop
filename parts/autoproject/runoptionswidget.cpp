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

#include <qlineedit.h>
#include <qlistview.h>

#include "domutil.h"
#include "addenvvardlg.h"
#include "autoprojectpart.h"
#include "runoptionswidget.h"


RunOptionsWidget::RunOptionsWidget(AutoProjectPart *part, QWidget *parent, const char *name)
    : RunOptionsWidgetBase(parent, name)
{
    m_part = part;

    QDomDocument dom = *m_part->document();

    mainprogram_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/run/mainprogram"));
    progargs_edit->setText(DomUtil::readEntry(dom, "/kdevautoproject/run/programargs"));

    QDomElement docEl = dom.documentElement();
    QDomElement autoprojectEl = docEl.namedItem("kdevautoproject").toElement();
    QDomElement envvarsEl = autoprojectEl.namedItem("envvars").toElement();

    QListViewItem *lastItem = 0;
    QDomElement envvarEl = envvarsEl.firstChild().toElement();
    while (!envvarEl.isNull()) {
        if (envvarEl.tagName() == "envvar") {
            QListViewItem *newItem =
                new QListViewItem(listview, envvarEl.attribute("name"), envvarEl.attribute("value"));
            if (lastItem)
                newItem->moveItem(lastItem);
            lastItem = newItem;
        }
        envvarEl = envvarEl.nextSibling().toElement();
    }
    
}


RunOptionsWidget::~RunOptionsWidget()
{}


void RunOptionsWidget::accept()
{
    QDomDocument dom = *m_part->document();

    DomUtil::writeEntry(dom, "/kdevautoproject/run/mainprogram", mainprogram_edit->text());
    DomUtil::writeEntry(dom, "/kdevautoproject/run/programargs", progargs_edit->text());

    QDomElement docEl = dom.documentElement();
    QDomElement autoprojectEl = docEl.namedItem("kdevautoproject").toElement();
    if (autoprojectEl.isNull()) {
        autoprojectEl = dom.createElement("kdevautoproject");
        docEl.appendChild(autoprojectEl);
    }
    QDomElement envvarsEl = autoprojectEl.namedItem("envvars").toElement();
    if (envvarsEl.isNull()) {
        envvarsEl = dom.createElement("envvars");
        autoprojectEl.appendChild(envvarsEl);
    }

    // Clear old entries
    while (!envvarsEl.firstChild().isNull())
        envvarsEl.removeChild(envvarsEl.firstChild());

    QListViewItem *item = listview->firstChild();
    while (item) {
        QDomElement envvarEl = dom.createElement("envvar");
        envvarEl.setAttribute("name", item->text(0));
        envvarEl.setAttribute("value", item->text(1));
        envvarsEl.appendChild(envvarEl);
        item = item->nextSibling();
    }
   
}


void RunOptionsWidget::addVarClicked()
{
    AddEnvvarDialog dlg;
    if (!dlg.exec())
        return;

    (void) new QListViewItem(listview, dlg.varname(), dlg.value());
}


void RunOptionsWidget::removeVarClicked()
{
    delete listview->currentItem();
}

#include "runoptionswidget.moc"
