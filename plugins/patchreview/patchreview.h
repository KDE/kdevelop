/*
    SPDX-FileCopyrightText: 2006 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PATCHREVIEW_H
#define KDEVPLATFORM_PLUGIN_PATCHREVIEW_H

#include <QPointer>

#include <interfaces/iplugin.h>
#include <interfaces/ipatchsource.h>
#include <interfaces/ilanguagesupport.h>

class PatchHighlighter;
class PatchReviewToolViewFactory;

class QTimer;

namespace KDevelop {
class IDocument;
}
namespace Sublime {
class Area;
}

namespace KompareDiff2 {
class DiffModel;
class DiffSettings;
class Info;
class ModelList;
}

class PatchReviewPlugin;

class PatchReviewPlugin : public KDevelop::IPlugin, public KDevelop::IPatchReview, public KDevelop::ILanguageSupport
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IPatchReview )
    Q_INTERFACES( KDevelop::ILanguageSupport )

public :
    explicit PatchReviewPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~PatchReviewPlugin() override;
    void unload() override;

    KDevelop::IPatchSource::Ptr patch() const {
        return m_patch;
    }

    KompareDiff2::ModelList* modelList() const
    {
        return m_modelList.data();
    }

    QString name() const override {
      return QStringLiteral("diff");
    }

    KDevelop::ParseJob *createParseJob(const KDevelop::IndexedString &) override {
      return nullptr;
    }

    void seekHunk( bool forwards, const QUrl& file = QUrl() );

    void setPatch( KDevelop::IPatchSource* patch );

    void startReview( KDevelop::IPatchSource* patch, ReviewMode mode ) override;

    void finishReview(const QList<QUrl>& selection);

    QUrl urlForFileModel(const KompareDiff2::DiffModel* model) const;
    QAction* finishReviewAction() const { return m_finishReview; }

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

Q_SIGNALS:
    void startingNewReview();
    void patchChanged();

public Q_SLOTS :
    //Does parts of the review-starting that are problematic to do directly in startReview, as they may open dialogs etc.
    void updateReview();

    void cancelReview();
    void notifyPatchChanged();
    void highlightPatch();
    void updateKompareModel();
    void forceUpdate();
    void areaChanged(Sublime::Area* area);
    void executeFileReviewAction();

private Q_SLOTS :
    void documentClosed( KDevelop::IDocument* );
    void textDocumentCreated( KDevelop::IDocument* );
    void documentSaved( KDevelop::IDocument* );
    void closeReview();

private:
    void switchToEmptyReviewArea();

    /// Makes sure that this working set is active only in the @p area, and that its name starts with "review".
    void setUniqueEmptyWorkingSet(Sublime::Area* area);

    void addHighlighting( const QUrl& file, KDevelop::IDocument* document = nullptr );
    void removeHighlighting( const QUrl& file = QUrl() );

    KDevelop::IPatchSource::Ptr m_patch;

    QTimer* m_updateKompareTimer;

    PatchReviewToolViewFactory* m_factory;
    QAction* m_finishReview;

    #if 0
    void determineState();
    #endif

    QScopedPointer<KompareDiff2::DiffSettings> m_diffSettings;
    QScopedPointer<KompareDiff2::Info> m_kompareInfo;
    QScopedPointer<KompareDiff2::ModelList> m_modelList;
    uint m_depth = 0; // depth of the patch represented by m_modelList
    using HighlightMap = QMap<QUrl, QPointer<PatchHighlighter>>;
    HighlightMap m_highlighters;
    QString m_lastArea;

    friend class PatchReviewToolView; // to access slot exporterSelected();
};

#endif
