/*
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/**
 * This is an internal header
*/

#ifndef KDEVPLATFORM_VCSDIFFPATCHSOURCES_H
#define KDEVPLATFORM_VCSDIFFPATCHSOURCES_H

#include <interfaces/ipatchsource.h>
#include "vcs/vcsstatusinfo.h"

#include <vcs/vcsexport.h>

#include <QMap>
#include <QUrl>

class KTextEdit;
class KComboBox;
class KJob;
namespace KDevelop {
class VcsCommitDialog;
class IBasicVersionControl;
class VcsDiff;
}

class QWidget;

class VCSDiffUpdater {
public:
    virtual ~VCSDiffUpdater();
    virtual KDevelop::VcsDiff update() const = 0;
    virtual KDevelop::IBasicVersionControl* vcs() const = 0;
    virtual QUrl url() const = 0;
};

class KDEVPLATFORMVCS_EXPORT VCSStandardDiffUpdater : public VCSDiffUpdater {
public:
    VCSStandardDiffUpdater(KDevelop::IBasicVersionControl* vcs, const QUrl& url);
    ~VCSStandardDiffUpdater() override;
    KDevelop::VcsDiff update() const override;
    KDevelop::IBasicVersionControl* vcs() const override { return m_vcs; }
    QUrl url() const override { return m_url; }
private:
    KDevelop::IBasicVersionControl* m_vcs;
    QUrl m_url;
};


class KDEVPLATFORMVCS_EXPORT VCSDiffPatchSource : public KDevelop::IPatchSource {
    Q_OBJECT
    public:
    /// The ownership of the updater is taken
    explicit VCSDiffPatchSource(VCSDiffUpdater* updater);
    explicit VCSDiffPatchSource(const KDevelop::VcsDiff& diff);
    ~VCSDiffPatchSource() override;

    QUrl baseDir() const override ;

    QUrl file() const override ;

    QString name() const override ;

    uint depth() const override ;

    void update() override ;

    bool isAlreadyApplied() const override { return true; }

    QMap<QUrl, KDevelop::VcsStatusInfo::State> additionalSelectableFiles() const override ;

    QUrl m_base, m_file;
    QString m_name;
    VCSDiffUpdater* m_updater;
    QList<KDevelop::VcsStatusInfo> m_infos;
    QMap<QUrl, KDevelop::VcsStatusInfo::State> m_selectable;
    private:
    void updateFromDiff(const KDevelop::VcsDiff& diff);
    uint m_depth = 0;
};

class KDEVPLATFORMVCS_EXPORT VCSCommitDiffPatchSource : public VCSDiffPatchSource {
    Q_OBJECT
    public:
    /// The ownership of the updater is taken
    explicit VCSCommitDiffPatchSource(VCSDiffUpdater* updater);
    ~VCSCommitDiffPatchSource() override ;

    QStringList oldMessages() const;

    bool canSelectFiles() const override ;

    QWidget* customWidget() const override ;

    QString finishReviewCustomText() const override ;

    bool canCancel() const override;

    void cancelReview() override;

    bool finishReview(const QList<QUrl>& selection) override;
    QList<KDevelop::VcsStatusInfo> infos() const { return m_infos; }
Q_SIGNALS:
    void reviewFinished(const QString& message, const QList<QUrl>& selection);
    void reviewCancelled(const QString& message);
public:
    QPointer<QWidget> m_commitMessageWidget;
    QPointer<KTextEdit> m_commitMessageEdit;
    KDevelop::IBasicVersionControl* m_vcs;
    KComboBox* m_oldMessages;
public Q_SLOTS:
    void addMessageToHistory(const QString& message);
    void oldMessageChanged(const QString&);
    void jobFinished(KJob*);
};

///Sends the diff to the patch-review plugin.
///Returns whether the diff was shown successfully.
bool KDEVPLATFORMVCS_EXPORT showVcsDiff(KDevelop::IPatchSource* vcsDiff);

#endif // KDEVPLATFORM_VCSDIFFPATCHSOURCES_H
