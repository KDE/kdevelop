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

#include "refactoringdialog.h"

#include <language/duchain/duchainlock.h>
#include <serialization/indexedstring.h>
#include <language/duchain/navigation/usescollector.h>

namespace KDevelop
{

RefactoringProgressDialog::RefactoringProgressDialog(const QString& action, KDevelop::UsesCollector* collector)
    : m_collector(collector)
{
    if (!collector) {
        return;
    }

    m_rd.setupUi(this);

    m_rd.progressBar->setMinimum(0);
    m_rd.progressBar->setMaximum(0);
    m_rd.renameLabel->setText(action);
    connect(m_collector, &UsesCollector::processUsesSignal, this, &RefactoringProgressDialog::processUses);
    connect(m_collector, &UsesCollector::progressSignal, this, &RefactoringProgressDialog::progress);
    connect(m_collector, &UsesCollector::maximumProgressSignal, this, &RefactoringProgressDialog::maximumProgress);
}

void RefactoringProgressDialog::progress(uint done, uint max)
{
    if (done == max)
        accept();
}

void RefactoringProgressDialog::maximumProgress(uint max)
{
    if (max == 0)
        accept();
}

void RefactoringProgressDialog::processUses(const KDevelop::ReferencedTopDUContext& context)
{
    DUChainReadLocker lock;
    if (context.data()) {
        m_rd.fileLabel->setText(context->url().str());
    }
}
}
