#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include "grepview.h"
#include "grepwidget.h"
#include "main.h"


GrepView::GrepView(QWidget *parent, const char *name)
    : KDevComponent(parent, name)
{
    kdDebug(9001) << "Building GrepWidget" << endl;
    
    setInstance(GrepFactory::instance());
    setXMLFile("kdevgrepview.rc");
    setWidget(new GrepWidget(this, parent));

    KAction *action;
    action = new KAction(i18n("Grep"), CTRL+ALT+Key_F, widget(), SLOT(showDialog()),
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


GrepView::~GrepView()
{}


void GrepView::compilationAborted()
{
    kdDebug(9001) << "GrepView::compilationAborted()" << endl;
    grepWidget()->killJob();
}

void GrepView::projectOpened(CProject *prj)
{
    kdDebug(9001) << "GrepView::projectOpened()" << endl;
    grepWidget()->projectOpened(prj);
}

void GrepView::projectClosed()
{
    kdDebug(9001) << "GrepView::projectClosed()" << endl;
}
