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

#include <qapplication.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kconfig.h>
#include <kapplication.h>

#include "splitter.h"
#include "statusbar.h"
#include "toplevel.h"


TopLevel::TopLevel(QWidget *parent, const char *name)
    : KParts::MainWindow(parent, name)
{
    setXMLFile("gideonui.rc");

    KToggleAction *action;
    KConfig* config = kapp->config();

    action = new KToggleAction( i18n("&Selection views"), 0,
                                this, SLOT(slotToggleSelectViews()),
                                actionCollection(), "view_selectviews" );
    action->setChecked(true);

    action = new KToggleAction( i18n("&Output views"), 0,
                                this, SLOT(slotToggleOutputViews()),
                                actionCollection(), "view_outputviews" );
    action->setChecked(true);

    KStdAction::showToolbar( this, SLOT(slotShowToolbar()),
                             actionCollection(), "settings_show_toolbar" );
    KStdAction::configureToolbars( this, SLOT(slotOptionsEditToolbars()),
                                   actionCollection(), "settings_configure_toolbars" );

    vertSplitter = new QSplitter(Vertical, this);
    horzSplitter = new QSplitter(Horizontal, vertSplitter);

    leftTabGroup =  new QTabWidget(horzSplitter);
    mainSplitter =  new Splitter(horzSplitter, "main splitter");
    lowerTabGroup = new QTabWidget(vertSplitter);

    vertSplitter->setResizeMode(horzSplitter, QSplitter::Stretch);
    vertSplitter->setResizeMode(lowerTabGroup, QSplitter::KeepSize);
    horzSplitter->setResizeMode(leftTabGroup, QSplitter::KeepSize);
    horzSplitter->setResizeMode(mainSplitter, QSplitter::Stretch);

    setCentralWidget(vertSplitter);

    (void) new StatusBar(this);

    config->setGroup("General Options");
    setGeometry(config->readRectEntry("Geomentry"));
    vertSplitter->setSizes(config->readIntListEntry("Vertical Splitter"));
    horzSplitter->setSizes(config->readIntListEntry("Horizontal Splitter"));
    closing = false;
}


TopLevel::~TopLevel()
{}


void TopLevel::createGUI(KParts::Part *part)
{
    KParts::MainWindow::createGUI(part);
}


bool TopLevel::queryClose()
{
    if (closing)
        return true;

    emit wantsToQuit();
    return false;
}


void TopLevel::splitterCollapsed(Splitter *splitter)
{
    if (splitter!=mainSplitter && !splitter->hasMultipleChildren()) {
        // The splitter has 'collapsed' to have only one child
        // So we can destroy it and let the only child be
        // adopted by the splitter one level higher
        if (!splitter->parentWidget()->inherits("Splitter")) {
            kdDebug(9000) << "Hmm, splitter is not nested in another one?" << endl;
            return;
        }
        Splitter *umbrellaSplitter = static_cast<Splitter*>(splitter->parentWidget());
        QWidget *single = splitter->firstChild();
        single->reparent(umbrellaSplitter, QPoint(0, 0));
        umbrellaSplitter->replaceChild(splitter, single);
        single->show();
        delete splitter;
        return;
    }
}


void TopLevel::closeReal()
{
  // store the widget configuration
  KConfig* config = kapp->config();
  config->setGroup("General Options");
  config->writeEntry("Geomentry",geometry());
  config->writeEntry("Vertical Splitter",vertSplitter->sizes());
  config->writeEntry("Horizontal Splitter",horzSplitter->sizes());
  closing = true;
  close();
}


void TopLevel::splitDocumentWidget(QWidget *w, QWidget *old, Orientation orient)
{
    kdDebug(9000) << "splitting widget" << endl;

    // Note: old must be != w
    Splitter *splitter = mainSplitter;
    if (old)
        splitter = static_cast<Splitter*>(old->parentWidget());

    if (orient == splitter->orientation()) {
        w->reparent(splitter, QPoint(0, 0));
        splitter->splitChild(old, w);
    } else {
        if (splitter->hasMultipleChildren()) {
            // orthogonal to the splitter's direction
            // => create nested splitter and embed both the
            // old widget and the new one in it.
            Splitter *nestedSplitter = new Splitter(splitter, "splitter");
            nestedSplitter->setOrientation(orient);
            connect( nestedSplitter, SIGNAL(collapsed(Splitter*)),
                     this, SLOT(splitterCollapsed(Splitter*)) );
            // The order of the following instructions is important
            splitter->replaceChild(old, nestedSplitter);
            old->reparent(nestedSplitter, QPoint(0, 0));
            nestedSplitter->addChild(old);
            w->reparent(nestedSplitter, QPoint(0, 0));
            nestedSplitter->addChild(w);
            nestedSplitter->show();
        } else {
            splitter->setOrientation(orient);
            w->reparent(splitter, QPoint(0, 0));
            splitter->splitChild(old, w);
        }
    }
}


void TopLevel::embedDocumentWidget(QWidget *w, QWidget *old)
{
    // Note: old must be != w
    Splitter *splitter = mainSplitter;
    if (old)
        splitter = static_cast<Splitter*>(old->parentWidget());

    w->reparent(splitter, QPoint(0, 0));

    if (old) {
        kdDebug(9000) << "replacing widget" << endl;
        splitter->replaceChild(old, w);
    // TODO: WHAT WAS THIS?!?        delete old;
    } else {
        kdDebug(9000) << "adding widget" << endl;
        splitter->addChild(w);
    }
}


void TopLevel::embedToolWidget(QWidget *w, KDevCore::Role role, const QString &shortCaption)
{
    switch (role) {
    case KDevCore::SelectView:
        w->reparent(leftTabGroup, QPoint(0, 0));
        leftTabGroup->addTab(w, shortCaption);
        leftWidgets.append(w);
        break;
    case KDevCore::OutputView:
        w->reparent(lowerTabGroup, QPoint(0, 0));
        lowerTabGroup->addTab(w, shortCaption);
        lowerWidgets.append(w);
        break;
    default:
        return;
    }
}


void TopLevel::raiseWidget(QWidget *w)
{
    w->show();
    w->setFocus();

    if (leftWidgets.contains(w))
        leftTabGroup->showPage(w);
    else if (lowerWidgets.contains(w))
        lowerTabGroup->showPage(w);
}


void TopLevel::slotToggleSelectViews()
{
    if (leftTabGroup->isVisible())
        leftTabGroup->hide();
    else
        leftTabGroup->show();
    KToggleAction *toggleAction = static_cast<KToggleAction*>(action("view_selectviews"));
    toggleAction->setChecked(leftTabGroup->isVisible());
}


void TopLevel::slotToggleOutputViews()
{
    if (lowerTabGroup->isVisible())
        lowerTabGroup->hide();
    else
        lowerTabGroup->show();
    KToggleAction *toggleAction = static_cast<KToggleAction*>(action("view_outputviews"));
    toggleAction->setChecked(lowerTabGroup->isVisible());
}


void TopLevel::slotOptionsEditToolbars()
{
    KEditToolbar dlg(factory());
    if (dlg.exec())
        createGUI(0);
}

#include "toplevel.moc"
