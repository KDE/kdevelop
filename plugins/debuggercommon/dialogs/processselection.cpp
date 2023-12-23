/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    setWindowTitle(i18nc("@title:window", "Attach to a Process"));
    m_processList = new KSysGuardProcessList(this);
    auto* mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(m_processList);
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
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
    m_attachButton->setText(i18nc("@action:button", "Attach"));
    m_attachButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    m_attachButton->setEnabled(false);

    KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("GdbProcessSelectionDialog"));
    m_processList->filterLineEdit()->setText(config.readEntry("filterText", QString()));
    m_processList->loadSettings(config);
    restoreGeometry(config.readEntry("dialogGeometry", QByteArray()));
}

ProcessSelectionDialog::~ProcessSelectionDialog()
{
    KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("GdbProcessSelectionDialog"));
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

#include "moc_processselection.cpp"
