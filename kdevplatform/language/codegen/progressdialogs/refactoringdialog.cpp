/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "refactoringdialog.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/navigation/usescollector.h>

namespace KDevelop {
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

#include "moc_refactoringdialog.cpp"
