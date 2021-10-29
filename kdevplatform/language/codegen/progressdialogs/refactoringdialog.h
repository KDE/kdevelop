/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_REFACTORINGDIALOG_H
#define KDEVPLATFORM_REFACTORINGDIALOG_H

#include <QDialog>
#include <language/duchain/topducontext.h>

#include "ui_refactoringdialog.h"

namespace KDevelop {
class UsesCollector;

class RefactoringProgressDialog
    : public QDialog
{
    Q_OBJECT

public:
    RefactoringProgressDialog(const QString& action, UsesCollector* collector);

private Q_SLOTS:
    void progress(uint done, uint max);
    void maximumProgress(uint max);
    void processUses(const KDevelop::ReferencedTopDUContext& context);

private:
    UsesCollector* m_collector;
    Ui::RefactoringDialog m_rd;
};
}
#endif
