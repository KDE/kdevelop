#include <kdebug.h>

#include "doctreeview.h"
#include "doctreewidget.h"
#include "main.h"


DocTreeView::DocTreeView(QWidget *parent, const char *name)
{
    kdDebug(9002) << "Building DocTreeWidget" << endl;

    setInstance(DocTreeFactory::instance());
    setWidget(new DocTreeWidget(this, parent));
}


DocTreeView::~DocTreeView()
{}


void DocTreeView::docPathChanged()
{
    doctreeWidget()->docPathChanged();
}


void DocTreeView::projectOpened(CProject *prj)
{
    doctreeWidget()->projectOpened(prj);
}


void DocTreeView::projectClosed()
{
    doctreeWidget()->projectClosed();
}
