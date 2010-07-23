/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
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

class KJob;
class QStringList;

namespace KDevelop
{
class IBasicVersionControl;
class IPlugin;

class KDEVPLATFORMVCS_EXPORT VcsCommitDialog : public KDialog
{
    Q_OBJECT
public:
    VcsCommitDialog( IPlugin *plugin, QWidget *parent = 0 );
    virtual ~VcsCommitDialog();
    ///Sets the commit candidates, and eventually shows the dialog
    ///Should be called as last action
    void setCommitCandidatesAndShow( const KUrl &list );
    void setMessage( const QString& );
    void setRecursive( bool );
    void setOldMessages( const QStringList& );
    bool recursive() const;
    QString message() const;
    IPlugin* versionControlPlugin();
    
    ///Returns the items that are changed and should be checked in.
    ///All other changes like adding, deleting, etc. are done in place
    KUrl::List determineUrlsForCheckin();
    
private Q_SLOTS:
    void commitDiffJobFinished(KJob* job);
    //Connection to the patch-review plugin
    void reviewFinished(QString message, QList<KUrl> selection);

Q_SIGNALS:
    void doCommit( KDevelop::VcsCommitDialog* dlg );
    void cancelCommit( KDevelop::VcsCommitDialog* dlg );
private:
    
    void executeCommit();
    
    Q_PRIVATE_SLOT( d, void ok() )
    Q_PRIVATE_SLOT( d, void cancel() )
    void getDVCSfileLists(KUrl::List &resetFiles, KUrl::List &addFiles, KUrl::List &rmFiles) const;
    friend class VcsCommitDialogPrivate;
    class VcsCommitDialogPrivate* const d;
public slots:
    void reviewCancalled(QString);
};

}

#endif
