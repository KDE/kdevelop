#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include "mdimainfrmcomponent.h"
#include "mdiframe.h"
#include "main.h"


MdiMainFrmComponent::MdiMainFrmComponent(QObject *parent, const char *name)
    : KDevComponent(parent, name)
{
    setInstance(MdiMainFrmFactory::instance());
    setXMLFile("kdevmdimainfrmcomponent.rc");

    m_widget = 0;
}


MdiMainFrmComponent::~MdiMainFrmComponent()
{}


void MdiMainFrmComponent::setupGUI()
{
    kdDebug(9005) << "Building QextMDI-MainFrame" << endl;

    m_widget = new MdiFrame(0L);
    emit embedWidget(m_widget, DocumentView, i18n("QextMdi-MainFrm"), i18n("source file views area"));
}


void MdiMainFrmComponent::projectOpened(CProject *prj)
{
}


void MdiMainFrmComponent::projectClosed()
{
}
