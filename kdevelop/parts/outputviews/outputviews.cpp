#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>

#include <cproject.h>
#include "main.h"
#include "makewidget.h"
#include "appoutputwidget.h"
#include "outputviews.h"


MakeView::MakeView(QObject *parent, const char *name)
    : KDevComponent(parent,  name)
{
    setInstance(OutputFactory::instance());
    setXMLFile("kdevmakeview.rc");

    m_prj = 0;
    m_widget = 0;
}


MakeView::~MakeView()
{}


void MakeView::setupGUI()
{
    kdDebug(9004) << "Building MakeWidget" << endl;

    m_widget = new MakeWidget(this);
    m_widget->setCaption(i18n("Compiler messages"));

    emit embedWidget(m_widget, OutputView, i18n("Messages"), i18n("messages output view"));

    KAction *action;
    action = new KAction( i18n("&Next error"), Key_F4, m_widget, SLOT(nextError()),
                          actionCollection(), "view_next_error");
    action->setShortText( i18n("Switches to the file and line the next error was reported") );
    action = new KAction( i18n("&Previous error"), SHIFT+Key_F4, m_widget, SLOT(prevError()),
                          actionCollection(), "view_previous_error");
    action->setShortText( i18n("Switches to the file and line the previous error was reported") );
}


void MakeView::compilationStarted(const QString &command)
{
    if (!m_prj) {
        kdDebug(9004) << "MakeView: compilation started with project?" << endl;
    }
    
    m_widget->prepareJob(m_prj->getProjectDir());
    *m_widget << command;
    m_widget->startJob();
}


void MakeView::projectOpened(CProject *prj)
{
    m_prj = prj;
}


void MakeView::projectClosed()
{
    m_prj = 0;
    m_widget->clear();
}


AppOutputView::AppOutputView(QObject *parent, const char *name)
    : KDevComponent(parent,  name)
{
    setInstance(OutputFactory::instance());
}


AppOutputView::~AppOutputView()
{}


void AppOutputView::setupGUI()
{
    kdDebug(9004) << "Building AppOutputWidget" << endl;

    m_widget = new AppOutputWidget();
    m_widget->setCaption(i18n("Application output"));

    emit embedWidget(m_widget, OutputView, i18n("Application"), i18n("application output view"));
}


void AppOutputView::compilationAborted()
{
    m_widget->compilationAborted();
}
