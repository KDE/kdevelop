/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "progressdialogs.h"
#include <language/duchain/duchainlock.h>
#include <qboxlayout.h>
#include <QPushButton>
#include <QLabel>
#include <klocalizedstring.h>
#include <language/duchain/navigation/usescollector.h>
#include <language/duchain/duchain.h>

using namespace KDevelop;

QString StringProgressBar::text() const {
  return m_text.str();
}

void StringProgressBar::setText(IndexedString text) {
  if(text != m_text) {
    setUpdatesEnabled(false);
    setTextVisible(false);
    m_text = text;
    setTextVisible(true);
    setUpdatesEnabled(true);
  }
}

RefactoringProgressDialog::RefactoringProgressDialog(QString action) {
  
  QHBoxLayout* layout = new QHBoxLayout();
  QVBoxLayout* layoutV = new QVBoxLayout;
  layout->addWidget(new QLabel(action));
  layoutV->addLayout(layout);
  
  m_progressBar = new StringProgressBar;
  layoutV->addWidget(m_progressBar);
  
  m_cancelButton = new QPushButton(KIcon("dialog-cancel"), i18n("Cancel"));
  layout->addWidget(m_cancelButton, 0, Qt::AlignRight);
  
  resize(380, 90);//layoutV->sizeHint().height());
  setLayout(layoutV);
  
  connect(m_cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

void RefactoringProgressDialog::progress(uint done, uint max) {

  m_progressBar->setValue(done);
  m_progressBar->setMaximum((int)max);

  if(done == max)
    accept();
}
    
void RefactoringProgressDialog::maximumProgress(uint max) {
  m_progressBar->setMaximum(max);
  if(max == 0)
    accept();
}
    
void RefactoringProgressDialog::setProcessing(IndexedString processing) {
  m_progressBar->setText(processing);
}

CollectorProgressDialog::CollectorProgressDialog(QString action, UsesCollector& collector) : RefactoringProgressDialog(action), m_collector(collector) {
  
  connect(&m_collector, SIGNAL(processUsesSignal(KDevelop::ReferencedTopDUContext)), this, SLOT(processUses(KDevelop::ReferencedTopDUContext)));
  connect(&m_collector, SIGNAL(progressSignal(uint,uint)), this, SLOT(progress(uint,uint)));
  connect(&m_collector, SIGNAL(maximumProgressSignal(uint)), this, SLOT(maximumProgress(uint)));
}
    
void CollectorProgressDialog::processUses(ReferencedTopDUContext context) {
  DUChainReadLocker lock(DUChain::lock());
  if(context.data())
    setProcessing(context->url());
}

#include "progressdialogs.moc"
