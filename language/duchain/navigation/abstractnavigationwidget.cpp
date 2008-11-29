/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "abstractnavigationwidget.h"

#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QMetaObject>
#include <QtGui/QScrollBar>
#include <QtGui/QBoxLayout>

#include <klocale.h>

#include "../declaration.h"
#include "../ducontext.h"
#include "../duchainlock.h"
#include "../functiondeclaration.h"
#include "../functiondefinition.h"
#include "../forwarddeclaration.h"
#include "../namespacealiasdeclaration.h"
#include "../classfunctiondeclaration.h"
#include "../classmemberdeclaration.h"
#include "../topducontext.h"
#include "abstractnavigationcontext.h"
#include "abstractdeclarationnavigationcontext.h"
#include "navigationaction.h"
#include "useswidget.h"
#include "../../../interfaces/icore.h"
#include "../../../interfaces/idocumentcontroller.h"

namespace KDevelop {

AbstractNavigationWidget::AbstractNavigationWidget()
  : m_currentWidget(0)
{
}

void AbstractNavigationWidget::initBrowser(int height) {
  m_browser = new KTextBrowser;

  m_browser->setOpenLinks(false);
  m_browser->setOpenExternalLinks(false);
  resize(height, 100);
  //m_browser->setNotifyClick(true);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(m_browser);
  setLayout(layout);

  connect( m_browser, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(anchorClicked(const QUrl&)) );
}

AbstractNavigationWidget::~AbstractNavigationWidget() {
  if(m_currentWidget)
    layout()->removeWidget(m_currentWidget);
    
}

void AbstractNavigationWidget::setContext(NavigationContextPointer context)
{
  if(!context) {
    kDebug() << "no new context created";
    return;
  }
  if(context == m_context && (!context || context->alreadyComputed()))
    return;
  m_context = context;
  update();
}

void AbstractNavigationWidget::update() {
  setUpdatesEnabled(false);
  Q_ASSERT( m_context );
  
  QString html = m_context->html();
  if(!html.isEmpty()) {
    int scrollPos = m_browser->verticalScrollBar()->value();
    m_browser->setHtml( m_context->html() );

    m_browser->verticalScrollBar()->setValue(scrollPos);
    m_browser->scrollToAnchor("selectedItem");
    m_browser->show();
  }else{
    m_browser->hide();
  }
  
  if(m_currentWidget) {
    layout()->removeWidget(m_currentWidget);
    m_currentWidget->setParent(0);
  }

  m_currentWidget = m_context->widget();
  
  if(m_currentWidget) {
    //This connection is a bit hacky..
    connect(m_currentWidget, SIGNAL(navigateDeclaration(KDevelop::IndexedDeclaration)),  this, SLOT(navigateDeclaration(KDevelop::IndexedDeclaration)));
    layout()->addWidget(m_currentWidget);
    //Leave unused room to the widget
    m_browser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_browser->setMaximumHeight(25);
  }else{
    m_browser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_browser->setMaximumHeight(10000);
  }

  setUpdatesEnabled(true);
}

NavigationContextPointer AbstractNavigationWidget::context() {
  return m_context;
}

void AbstractNavigationWidget::navigateDeclaration(KDevelop::IndexedDeclaration decl) {
  DUChainReadLocker lock( DUChain::lock() );
  setContext(m_context->accept(decl));
}

void AbstractNavigationWidget::anchorClicked(const QUrl& url) {
  DUChainReadLocker lock( DUChain::lock() );
  setContext( m_context->acceptLink(url.toString()) );
}

void AbstractNavigationWidget::keyPressEvent(QKeyEvent* event) {
  QWidget::keyPressEvent(event);
}

void AbstractNavigationWidget::next() {
  DUChainReadLocker lock( DUChain::lock() );
  Q_ASSERT( m_context );
  m_context->nextLink();
  update();
}

void AbstractNavigationWidget::previous() {
  DUChainReadLocker lock( DUChain::lock() );
  Q_ASSERT( m_context );
  m_context->previousLink();
  update();
}

void AbstractNavigationWidget::accept() {
  DUChainReadLocker lock( DUChain::lock() );
  Q_ASSERT( m_context );
  setContext( m_context->accept() );
}

void AbstractNavigationWidget::back() {
  setContext( m_context->back() );
}

void AbstractNavigationWidget::up() {
  m_browser->verticalScrollBar()->triggerAction( QAbstractSlider::SliderSingleStepSub );
}

void AbstractNavigationWidget::down() {
  m_browser->verticalScrollBar()->triggerAction( QAbstractSlider::SliderSingleStepAdd );
}

void AbstractNavigationWidget::embeddedWidgetAccept() {
  accept();
}
void AbstractNavigationWidget::embeddedWidgetDown() {
  down();
}

void AbstractNavigationWidget::embeddedWidgetRight() {
  next();
}

void AbstractNavigationWidget::embeddedWidgetLeft() {
  previous();
}

void AbstractNavigationWidget::embeddedWidgetUp() {
  up();
}



}

#include "abstractnavigationwidget.moc"
