#include "doctreeview.h"
#include "doctreewidget.h"
#include "main.h"


DocTreeView::DocTreeView(QWidget *parent, const char *name)
{
    setInstance(DocTreeFactory::instance());
    
    qDebug("Building DocTreeWidget");
    m_widget = new DocTreeWidget(this, parent);
}


DocTreeView::~DocTreeView()
{}


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


QWidget *DocTreeView::widget()
{
    return m_widget;
}
