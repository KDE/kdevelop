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

    new KAction(i18n("Grep"), CTRL+ALT+Key_F, widget(), SLOT(showDialog()),
                actionCollection(), "grep");
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
