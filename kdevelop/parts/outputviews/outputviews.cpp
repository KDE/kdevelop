#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include <cproject.h>
#include "main.h"
#include "makewidget.h"
#include "appoutputwidget.h"
#include "outputviews.h"


MakeView::MakeView(QWidget *parent, const char *name)
{
    kdDebug(9004) << "Building MakeWidget" << endl;

    setInstance(OutputFactory::instance());
    setXMLFile("kdevmakeview.rc");
    setWidget(new MakeWidget(this, parent));

    KAction *action;
    action = new KAction( i18n("&Next error"), Key_F4, widget(), SLOT(nextError()),
                          actionCollection(), "view_next_error");
    action->setShortText( i18n("Switches to the file and line the next error was reported") );
    action = new KAction( i18n("&Previous error"), SHIFT+Key_F4, widget(), SLOT(prevError()),
                          actionCollection(), "view_previous_error");
    action->setShortText( i18n("Switches to the file and line the previous error was reported") );

    m_prj = 0;
}


MakeView::~MakeView()
{}


void MakeView::compilationStarted(const QString &command)
{
    if (!m_prj) {
        kdDebug(9004) << "MakeView: compilation started with project?" << endl;
    }
    
    makeWidget()->prepareJob(m_prj->getProjectDir());
    *makeWidget() << command;
    makeWidget()->startJob();
}


void MakeView::projectOpened(CProject *prj)
{
    m_prj = prj;
}


void MakeView::projectClosed()
{
    m_prj = 0;
    makeWidget()->clear();
}


AppOutputView::AppOutputView(QWidget *parent, const char *name)
{
    kdDebug(9004) << "Building AppOutputWidget" << endl;

    setInstance(OutputFactory::instance());
    setWidget(new AppOutputWidget(parent));
}


AppOutputView::~AppOutputView()
{}


void AppOutputView::compilationAborted()
{
    appOutputWidget()->compilationAborted();
}
