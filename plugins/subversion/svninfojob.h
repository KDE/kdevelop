/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNINFOJOB_H
#define KDEVPLATFORM_PLUGIN_SVNINFOJOB_H

#include "svnjobbase.h"

#include <QDateTime>
#include <QUrl>

#include <vcs/vcsrevision.h>

class SvnInternalInfoJob;


/// A structure which describes various system-generated metadata about
/// a working-copy path or URL.
class SvnInfoHolder
{
public:
    QString name;
    QUrl url;
    qlonglong rev;
    int kind;
    QUrl repoUrl;
    QString repouuid;
    qlonglong lastChangedRev;
    QDateTime lastChangedDate;
    QString lastChangedAuthor;
    int scheduled;
    QUrl copyFromUrl;
    qlonglong copyFromRevision;
    QDateTime textTime;
    QDateTime propertyTime;
    QString oldFileConflict;
    QString newFileConflict;
    QString workingCopyFileConflict;
    QString propertyRejectFile;

};


/// This is an svn internal class, it shouldn't be used outside of the svn
/// plugin
class SvnInfoJob : public SvnJobBaseImpl<SvnInternalInfoJob>
{
    Q_OBJECT
public:
    enum ProvideInformationType
    {
        AllInfo,
        RevisionOnly,
        RepoUrlOnly
    };
    explicit SvnInfoJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setLocation( const QUrl &location );
    void setProvideInformation( ProvideInformationType );
    void setProvideRevisionType( KDevelop::VcsRevision::RevisionType );
public Q_SLOTS:
    void setInfo( const SvnInfoHolder& );
private:
    SvnInfoHolder m_info;
    ProvideInformationType m_provideInfo;
    KDevelop::VcsRevision::RevisionType m_provideRevisionType;

};

Q_DECLARE_METATYPE( SvnInfoHolder )

#endif

