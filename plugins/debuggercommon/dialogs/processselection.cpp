/* KDevelop GDB Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "processselection.h"

#include <processui/ksysguardprocesslist.h>
#include <processcore/process.h>

#include <KLocalizedString>
#include <KSharedConfig>

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>


using namespace KDevMI;

ProcessSelectionDialog::ProcessSelectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Attach to a Process"));
    m_processList = new KSysGuardProcessList(this);
    auto* mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(m_processList);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    connect(m_processList->treeView()->selectionModel(), &QItemSelectionModel::selectionChanged,
             this, &ProcessSelectionDialog::selectionChanged);
    m_processList->treeView()->setSelectionMode(QAbstractItemView::SingleSelection);
    m_processList->setState(ProcessFilter::UserProcesses);
    m_processList->setKillButtonVisible(false);
    m_processList->filterLineEdit()->setFocus();
    //m_processList->setPidFilter(qApp->pid());

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_attachButton = buttonBox->button(QDialogButtonBox::Ok);
    m_attachButton->setDefault(true);
    m_attachButton->setText(i18n("Attach"));
    m_attachButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    m_attachButton->setEnabled(false);

    KConfigGroup config = KSharedConfig::openConfig()->group("GdbProcessSelectionDialog");
    m_processList->filterLineEdit()->setText(config.readEntry("filterText", QString()));
    m_processList->loadSettings(config);
    restoreGeometry(config.readEntry("dialogGeometry", QByteArray()));
}

ProcessSelectionDialog::~ProcessSelectionDialog()
{
    KConfigGroup config = KSharedConfig::openConfig()->group("GdbProcessSelectionDialog");
    config.writeEntry("filterText", m_processList->filterLineEdit()->text());
    m_processList->saveSettings(config);
    config.writeEntry("dialogGeometry", saveGeometry());
}

long int ProcessSelectionDialog::pidSelected()
{
    QList<KSysGuard::Process*> ps=m_processList->selectedProcesses();
    Q_ASSERT(ps.count()==1);

    KSysGuard::Process* process=ps.first();

    return process->pid();
}

QSize ProcessSelectionDialog::sizeHint() const
{
    return QSize(740, 720);
}

void ProcessSelectionDialog::selectionChanged(const QItemSelection &selected)
{
    m_attachButton->setEnabled(selected.count());
}
