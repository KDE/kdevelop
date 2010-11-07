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

GrepOutputViewFactory::GrepOutputViewFactory(GrepJob* job): m_job(job)
{}

QWidget* GrepOutputViewFactory::create(QWidget* parent)
{
    return new GrepOutputView(parent, m_job);
}

Qt::DockWidgetArea GrepOutputViewFactory::defaultPosition()
{
    return Qt::BottomDockWidgetArea;
}

QString GrepOutputViewFactory::id() const
{
    return "org.kdevelop.GrepOutputView";
}


GrepOutputView::GrepOutputView(QWidget* parent, GrepJob* job)
  : QWidget(parent), m_job(job)
{
  Ui::GrepOutputView::setupUi(this);

//  setObjectName("Replace in files Tree");
//  setWindowTitle(i18n("Replace in files"));
  setWindowIcon(SmallIcon("cmake"));
  
  m_model = new GrepOutputModel(this);
  resultsTreeView->setModel(m_model);
  resultsTreeView->setItemDelegate(GrepOutputDelegate::self());
}

GrepOutputModel* GrepOutputView::model()
{
  return m_model;
}

