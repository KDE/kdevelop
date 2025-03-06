/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>
    SPDX-FileCopyrightText: 2011 Andrey Batyiev <batyiev@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "vcscommitdialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>
#include <interfaces/ipatchsource.h>

#include "../vcsstatusinfo.h"
#include "../models/vcsfilechangesmodel.h"

#include "ui_vcscommitdialog.h"

namespace KDevelop
{

class VcsCommitDialogPrivate
{
public:
    Ui::VcsCommitDialog ui;
    IPatchSource* m_patchSource;
    VcsFileChangesModel* m_model;
};

VcsCommitDialog::VcsCommitDialog( IPatchSource *patchSource, QWidget *parent )
    : QDialog(parent)
    , d_ptr(new VcsCommitDialogPrivate())
{
    Q_D(VcsCommitDialog);

    auto mainWidget = new QWidget(this);
    d->ui.setupUi(mainWidget);

    QWidget *customWidget = patchSource->customWidget();
    if( customWidget )
    {
        d->ui.gridLayout->addWidget( customWidget, 0, 0, 1, 2 );
    }

    auto okButton = d->ui.buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(d->ui.buttonBox, &QDialogButtonBox::accepted, this, &VcsCommitDialog::accept);
    connect(d->ui.buttonBox, &QDialogButtonBox::rejected, this, &VcsCommitDialog::reject);

    d->m_patchSource = patchSource;
    d->m_model = new VcsFileChangesModel( this, true );
    d->ui.files->setModel( d->m_model );
}

VcsCommitDialog::~VcsCommitDialog() = default;

void VcsCommitDialog::setRecursive( bool recursive )
{
    Q_D(VcsCommitDialog);

    d->ui.recursiveChk->setChecked( recursive );
}

void VcsCommitDialog::setCommitCandidates( const QList<KDevelop::VcsStatusInfo>& statuses )
{
    Q_D(VcsCommitDialog);

    for (const VcsStatusInfo& info : statuses) {
        d->m_model->updateState( info );
    }
}

bool VcsCommitDialog::recursive() const
{
    Q_D(const VcsCommitDialog);

    return d->ui.recursiveChk->isChecked();
}

void VcsCommitDialog::accept()
{
    Q_D(VcsCommitDialog);

    if( d->m_patchSource->finishReview( d->m_model->checkedUrls() ) )
    {
        QDialog::accept();
    } // else: do nothing to allow the user to make changes in this dialog and commit again
}

void VcsCommitDialog::reject()
{
    Q_D(VcsCommitDialog);

    d->m_patchSource->cancelReview();
    QDialog::reject();
}

}

#include "moc_vcscommitdialog.cpp"
