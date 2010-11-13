#include "grepjob.h"
#include "grepoutputview.h"
#include "grepoutputmodel.h"
#include "grepoutputdelegate.h"
#include "ui_grepoutputview.h"

#include <QtGui/QAction>
#include <QtGui/QTreeView>

#include <kpushbutton.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kxmlguiclient.h>
#include <ktoolbar.h>

#include <interfaces/icore.h>

using namespace KDevelop;

GrepOutputViewFactory::GrepOutputViewFactory()
{}

QWidget* GrepOutputViewFactory::create(QWidget* parent)
{
    return new GrepOutputView(parent);
}

Qt::DockWidgetArea GrepOutputViewFactory::defaultPosition()
{
    return Qt::BottomDockWidgetArea;
}

QString GrepOutputViewFactory::id() const
{
    return "org.kdevelop.GrepOutputView";
}

GrepOutputView::GrepOutputView(QWidget* parent)
  : QWidget(parent)
{
    Ui::GrepOutputView::setupUi(this);

    setWindowTitle(i18n("Replace output view"));
    setWindowIcon(SmallIcon("edit-find"));
    
    QAction *apply = new QAction(KIcon("dialog-ok-apply"), i18n("&Replace"), this);
    QAction *previous = new QAction(KIcon("go-previous"), i18n("&Previous"), this);
    QAction *next = new QAction(KIcon("go-next"), i18n("&Next"), this);
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    QAction *change_criteria = new QAction(KIcon("configure"), i18n("&Change criteria"), this);
    
    addAction(apply);
    addAction(previous);
    addAction(next);
    addAction(separator);
    addAction(change_criteria);
    

    m_model = new GrepOutputModel(this);
    resultsTreeView->setModel(m_model);
    resultsTreeView->setItemDelegate(GrepOutputDelegate::self());
    resultsTreeView->setHeaderHidden(true);
    connect(resultsTreeView, SIGNAL(activated(QModelIndex)), m_model, SLOT(activate(QModelIndex)));
}

GrepOutputModel* GrepOutputView::model()
{
    return m_model;
}

void GrepOutputView::setMessage(const QString& msg)
{
    messageLabel->setText(msg);
}
