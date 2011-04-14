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

#ifndef VCS_COMMITDIALOG_H
#define VCS_COMMITDIALOG_H

#include <KDE/KDialog>
#include <KDE/KUrl>
#include "../vcsexport.h"

namespace KDevelop
{
class IPatchSource;

class KDEVPLATFORMVCS_EXPORT VcsCommitDialog : public KDialog
{
    Q_OBJECT
public:
    VcsCommitDialog( IPatchSource *patchSource, QWidget *parent = 0 );
    virtual ~VcsCommitDialog();
    ///Sets the commit candidates
    void setCommitCandidates( const QVariant& );
    void setRecursive( bool );
    bool recursive() const;

private slots:
    void ok();
    void cancel();

private:
    class VcsCommitDialogPrivate* const d;
};

}

#endif
