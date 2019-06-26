/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2011 Andrey Batyiev <batyiev@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

private Q_SLOTS:
    void ok();
    void cancel();

private:
    const QScopedPointer<class VcsCommitDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE(VcsCommitDialog)
};

}

#endif
