#include <qvbox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>

#include "doctreeview.h"
#include "doctreewidget.h"
#include "doctreeconfigwidget.h"
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
    
    emit embedWidget(m_widget, SelectView, i18n("DOC"), i18n("documentation tree view"));
}


void DocTreeView::configWidgetRequested(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Documentation tree"));
    DocTreeConfigWidget *w = new DocTreeConfigWidget(this, vbox, "documentation tree config widget");
    connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}


void DocTreeView::projectOpened(CProject *prj)
{
    m_widget->projectOpened(prj);
}


void DocTreeView::projectClosed()
{
    m_widget->projectClosed();
}
