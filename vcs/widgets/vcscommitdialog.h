/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_COMMITDIALOG_H
#define SVN_COMMITDIALOG_H

#include <KDialog>
#include <KUrl>
#include "../vcsexport.h"

class QStringList;

namespace KDevelop
{
class IBasicVersionControl;

class KDEVPLATFORMVCS_EXPORT VcsCommitDialog : public KDialog
{
    Q_OBJECT
public:
    VcsCommitDialog( IBasicVersionControl *iface, QWidget *parent = 0 );
    virtual ~VcsCommitDialog();
    void setCommitCandidates( const KUrl::List &list );
    void setMessage( const QString& );
    void setRecursive( bool );
    void setOldMessages( const QStringList& );
    KUrl::List checkedUrls() const;
    bool recursive() const;
    QString message() const;
    IBasicVersionControl* versionControlIface();
Q_SIGNALS:
    void doCommit( KDevelop::VcsCommitDialog* dlg );
    void cancelCommit( KDevelop::VcsCommitDialog* dlg );
private:
    Q_PRIVATE_SLOT( d, void ok() )
    Q_PRIVATE_SLOT( d, void cancel() )
    friend class VcsCommitDialogPrivate;
    class VcsCommitDialogPrivate* const d;
};

}

#endif
