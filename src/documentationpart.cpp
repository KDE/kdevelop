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
#include <qfileinfo.h>
#include <qframe.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <kaction.h>
#include <kdebug.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kparts/partmanager.h>
#include <kstdaction.h>

#include "splitter.h"
#include "documentationpart.h"


class HistoryItem
{
public:
    QString caption;
    QByteArray state;
};


// FIXME: Check whether this really needs to inherit ReadOnlyPart

DocumentationPart::DocumentationPart(QWidget *parent, const char */*name*/)
    : KParts::ReadOnlyPart(parent, "documentationpart")
{
    QWidget *vbox = new QWidget(parent, "documentationvbox");
    vbox->setFocusPolicy(QWidget::StrongFocus);
    setWidget(vbox);
    
    htmlPart = new KHTMLPart(vbox, "htmlwidget", this, "htmlpart");
    vbox->setFocusProxy(htmlPart->widget());
    connect( htmlPart, SIGNAL(popupMenu(const QString &, const QPoint &)),
             this, SLOT(popupMenu(const QString &)) );
    connect( htmlPart->browserExtension(), SIGNAL(openURLRequestDelayed(const KURL &,const KParts::URLArgs &)),
             this, SLOT(openURLRequest(const KURL &)) );
    connect( htmlPart, SIGNAL(setWindowCaption(const QString &)),
             this, SLOT(setWindowCaption(const QString &)) );
    connect( htmlPart, SIGNAL(completed()),
             this, SLOT(completed()) );
    
    QFrame *statusbox = new SplitterBar(vbox);
    locationLabel = new QLabel(statusbox);

    QBoxLayout *statusLayout = new QHBoxLayout(statusbox, statusbox->frameWidth());
    statusLayout->addWidget(locationLabel);

    setXML("<!DOCTYPE kpartgui SYSTEM \"kpartgui.dtd\">\n"
           "<kpartgui version=\"1\" name=\"documentationpart\">\n"
           "<MenuBar>\n"
           "  <Menu name=\"file\">\n"
           "    <Action name=\"file_print\" />\n"
           "  </Menu>\n"
           "</MenuBar>\n"
           "<ToolBar name=\"mainToolBar\" >\n"
           "  <Action name=\"file_print\" />\n"
           "  <Action name=\"documentation_back\" />\n"
           "  <Action name=\"documentation_forward\" />\n"
           "  <Action name=\"documentation_find\" />\n"
           "</ToolBar>\n"
           "</kpartgui>"
           );

    QBoxLayout *layout = new QVBoxLayout(widget());
    layout->addWidget(htmlPart->widget(), 1);
    layout->addWidget(statusbox, 0);

    backAction = new KToolBarPopupAction(i18n("&Back"), "back", 0,
                                         this, SLOT(backActivated()),
                                         actionCollection(), "documentation_back");
    connect( backAction->popupMenu(), SIGNAL(aboutToShow()),
             this, SLOT(backShowing()) );
    connect( backAction->popupMenu(), SIGNAL(activated(int)),
             this, SLOT(backPopupActivated(int)) );
    backAction->setEnabled(false);
    
    forwAction = new KToolBarPopupAction(i18n("&Forward"), "forward", 0,
                                         this, SLOT(forwActivated()),
                                         actionCollection(), "documentation_forward");
    connect( forwAction->popupMenu(), SIGNAL(aboutToShow()),
             this, SLOT(forwShowing()) );
    connect( forwAction->popupMenu(), SIGNAL(activated(int)),
             this, SLOT(forwPopupActivated(int)) );
    forwAction->setEnabled(false);
    
    KStdAction::print(this, SLOT(slotPrint()), actionCollection(), "file_print");
    KStdAction::find(this, SLOT(slotFind()), actionCollection(), "documentation_find");
}


DocumentationPart::~DocumentationPart()
{}


void DocumentationPart::setManager(KParts::PartManager *manager)
{
    // When we are added, add the embedded part too
    KParts::ReadOnlyPart::setManager(manager);
    manager->addPart(htmlPart, false);
}


bool DocumentationPart::openFile()
{
    // ReadOnlyPart::openFile() is abstract, so we fake an implementation here
    return true;
}


void DocumentationPart::saveState()
{
    HistoryItem *current = history.current();
    if (current) {
        QDataStream stream(current->state, IO_WriteOnly);
        htmlPart->browserExtension()->saveState(stream);
        current->caption = currentCaption;
    }
}


void DocumentationPart::restoreState()
{
    HistoryItem *current = history.current();
    if (current) {
        QDataStream stream(current->state, IO_ReadOnly);
        htmlPart->browserExtension()->restoreState(stream);
        locationLabel->setText(current->caption);
    }
}


void DocumentationPart::updateHistoryAction()
{
    backAction->setEnabled(history.current() != history.getFirst());
    forwAction->setEnabled(history.current() != history.getLast());
}


void DocumentationPart::gotoURL(KURL url)
{
    QString fileName = QFile::encodeName(url.path());
    kdDebug(9000) << "Filename: " << fileName << endl;
    if (QFileInfo(fileName).isDir()) {
        url.addPath(QString::fromLatin1("index.html"));
    }

    saveState();

    HistoryItem *current = history.current();
    while (history.getLast() != current)
           history.removeLast();
    history.append(new HistoryItem);
    history.last();
    updateHistoryAction();

    currentCaption = QString();
    (void) htmlPart->openURL(url);
}


void DocumentationPart::popupMenu(const QString &url)
{
    QPopupMenu popup;
    backAction->plug(&popup);
    forwAction->plug(&popup);
    emit contextMenu(&popup, url, htmlPart->selectedText());
    if (popup.count())
        popup.exec(QCursor::pos());
}


KURL DocumentationPart::browserURL() const
{
    return htmlPart->url();
}


void DocumentationPart::openURLRequest(const KURL &url)
{
    gotoURL(url);
}


void DocumentationPart::setWindowCaption(const QString &caption)
{
    currentCaption = caption;
    locationLabel->setText(caption);
    widget()->setCaption(caption);
}


void DocumentationPart::completed()
{
    saveState();
}


void DocumentationPart::backActivated()
{
    saveState();
    history.prev();
    restoreState();
    updateHistoryAction();
}


void DocumentationPart::forwActivated()
{
    saveState();
    history.next();
    if (!history.current())
        history.append(new HistoryItem);
    restoreState();
    updateHistoryAction();
}


void DocumentationPart::backShowing()
{
    KPopupMenu *popup = backAction->popupMenu();
    popup->clear();
    int savePos = history.at();
    for (int i=0; i<10 && history.prev(); ++i) {
        popup->insertItem(history.current()->caption);
    }
    history.at(savePos);
}


void DocumentationPart::forwShowing()
{
    KPopupMenu *popup = forwAction->popupMenu();
    popup->clear();
    int savePos = history.at();
    for (int i=0; i<10 && history.next(); ++i) {
        popup->insertItem(history.current()->caption);
    }
    history.at(savePos);
}


void DocumentationPart::backPopupActivated(int id)
{
    int by = backAction->popupMenu()->indexOf(id)+1;
    kdDebug(9000) << "back by" << by << endl;
    saveState();
    for (int i=0; i < by; ++i)
        history.prev();
    restoreState();
    updateHistoryAction();
}


void DocumentationPart::forwPopupActivated(int id)
{
    int by = forwAction->popupMenu()->indexOf(id)+1;
    kdDebug(9000) << "forw by" << by << endl;
    saveState();
    for (int i=0; i < by; ++i)
        history.next();
    restoreState();
    updateHistoryAction();
}


void DocumentationPart::slotPrint()
{
    htmlPart->view()->print();
}


void DocumentationPart::slotFind()
{
    htmlPart->actionCollection()->action("find")->activate();
}

#include "documentationpart.moc"
