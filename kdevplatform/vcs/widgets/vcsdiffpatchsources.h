/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
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
