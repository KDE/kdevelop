/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2011 Andrey Batyiev <batyiev@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCS_COMMITDIALOG_H
#define KDEVPLATFORM_VCS_COMMITDIALOG_H

#include <QDialog>

#include <vcs/vcsexport.h>

namespace KDevelop
{

class VcsStatusInfo;
class IPatchSource;
class VcsCommitDialogPrivate;

class KDEVPLATFORMVCS_EXPORT VcsCommitDialog : public QDialog
{
    Q_OBJECT
public:
    explicit VcsCommitDialog( IPatchSource *patchSource, QWidget *parent = nullptr );
    ~VcsCommitDialog() override;
    ///Sets the commit candidates
    void setCommitCandidates( const QList<KDevelop::VcsStatusInfo>& statuses );
    void setRecursive( bool );
    bool recursive() const;

public Q_SLOTS:
    void accept() override;
    void reject() override;

private:
    const QScopedPointer<class VcsCommitDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE(VcsCommitDialog)
};

}

#endif
