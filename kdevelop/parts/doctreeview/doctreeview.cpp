/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qvbox.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>

#include "doctreeview.h"
#include "doctreewidget.h"
#include "doctreeconfigwidget.h"
#include "main.h"


DocTreeView::DocTreeView(QObject *parent, const char *name)
    : KDevComponent(parent, name)
{
    setInstance(DocTreeFactory::instance());

    m_widget = 0;
}


DocTreeView::~DocTreeView()
{
    delete m_widget;
}


void DocTreeView::setupGUI()
{
    kdDebug(9002) << "Building DocTreeWidget" << endl;

    m_widget = new DocTreeWidget(this);
    m_widget->setIcon(SmallIcon("mini-book1"));
    m_widget->setCaption(i18n("Documentation Tree"));
    QWhatsThis::add(m_widget, i18n("Documentation Tree\n\n"
                                   "The documentation tree gives access to library "
                                   "documentation and the KDevelop manuals. It can "
                                   "be configured individually."));
    
    emit embedWidget(m_widget, SelectView, i18n("DOC"), i18n("documentation tree view"));
}


void DocTreeView::configWidgetRequested(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Documentation tree"));
    DocTreeConfigWidget *w = new DocTreeConfigWidget(this, vbox, "documentation tree config widget");
    connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}


void DocTreeView::projectSpaceOpened()
{
    m_widget->projectSpaceOpened(projectSpace());
}

#include "doctreeview.moc"
