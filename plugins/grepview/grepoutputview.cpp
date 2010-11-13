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

//  setObjectName("Replace in files Tree");
//  setWindowTitle(i18n("Replace in files"));
  setWindowIcon(SmallIcon("cmake"));
  
  m_model = new GrepOutputModel(this);
  resultsTreeView->setModel(m_model);
  resultsTreeView->setItemDelegate(GrepOutputDelegate::self());
  resultsTreeView->setHeaderHidden(true);
  connect(resultsTreeView, SIGNAL(activated(QModelIndex)), m_model, SLOT(activate(QModelIndex)));
  connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(expandRootElement(QModelIndex)));
  connect(applyButton, SIGNAL(clicked(bool)), this, SLOT(onApply()));
}

GrepOutputModel* GrepOutputView::model()
{
    return m_model;
}

void GrepOutputView::setMessage(const QString& msg)
{
    messageLabel->setText(msg);
}

void GrepOutputView::enableReplace(bool enable)
{
    applyButton->setEnabled(enable);
}

void GrepOutputView::showErrorMessage( const QString& errorMessage )
{
    setStyleSheet("QLabel { color : red; }");
    setMessage(errorMessage);
}

void GrepOutputView::showMessage( KDevelop::IStatus* , const QString& message )
{
    setStyleSheet("");
    setMessage(message);
}

void GrepOutputView::onApply()
{
    setEnabled(false);
    m_model->doReplacements();
    setEnabled(true);
}

void GrepOutputView::expandRootElement(const QModelIndex& parent)
{
    if(!parent.isValid())
    {
        resultsTreeView->setExpanded(m_model->index(0,0), true);
    }
}

