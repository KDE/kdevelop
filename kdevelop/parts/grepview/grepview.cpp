#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kiconloader.h>
#include "grepview.h"
#include "grepwidget.h"
#include "main.h"


GrepView::GrepView(QObject *parent, const char *name)
    : KDevComponent(parent, name)
{
    setInstance(GrepFactory::instance());
    setXMLFile("kdevgrepview.rc");

    m_widget = 0;
}


GrepView::~GrepView()
{}


void GrepView::setupGUI()
{
    kdDebug(9001) << "Building GrepWidget" << endl;
    
    m_widget = new GrepWidget(this);
    m_widget->setIcon(SmallIcon("find"));
    m_widget->setCaption(i18n("Grep"));

    emit embedWidget(m_widget, OutputView, i18n("Grep"));

    KAction *action;
    action = new KAction(i18n("Grep"), CTRL+ALT+Key_F, m_widget, SLOT(showDialog()),
                         actionCollection(), "grep");
    action->setShortText( i18n("Opens the search in files dialog to search "
                               "for expressions over several files") );
    action->setWhatsThis( i18n("Search in files\n\n"
                               "Opens the Search in files dialog "
                               "to enter an expression to look up "
                               "over several files. Matches will be "
                               "displayed, you can switch to a match "
                               "directly.") );
}


void GrepView::compilationAborted()
{
    kdDebug(9001) << "GrepView::compilationAborted()" << endl;
    m_widget->killJob();
}


void GrepView::projectOpened(CProject *prj)
{
    kdDebug(9001) << "GrepView::projectOpened()" << endl;
    m_widget->projectOpened(prj);
}


void GrepView::projectClosed()
{
    kdDebug(9001) << "GrepView::projectClosed()" << endl;
}
