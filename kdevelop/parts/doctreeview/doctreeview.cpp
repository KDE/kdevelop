#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include "doctreeview.h"
#include "doctreewidget.h"
#include "main.h"


DocTreeView::DocTreeView(QObject *parent, const char *name)
    : KDevComponent(parent, name)
{
    setInstance(DocTreeFactory::instance());

    m_widget = 0;
}


DocTreeView::~DocTreeView()
{}


void DocTreeView::setupGUI()
{
    kdDebug(9002) << "Building DocTreeWidget" << endl;

    m_widget = new DocTreeWidget(this);
    m_widget->setIcon(SmallIcon("mini-book1"));
    m_widget->setCaption(i18n("Documentation"));
    
    emit embedWidget(m_widget, SelectView, i18n("DOC"));
}


void DocTreeView::docPathChanged()
{
    m_widget->docPathChanged();
}


void DocTreeView::projectOpened(CProject *prj)
{
    m_widget->projectOpened(prj);
}


void DocTreeView::projectClosed()
{
    m_widget->projectClosed();
}
