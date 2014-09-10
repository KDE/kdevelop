/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_PATCHREVIEW_H
#define KDEVPLATFORM_PLUGIN_PATCHREVIEW_H

#include <QObject>
#include <QPointer>

#include <interfaces/iplugin.h>
#include <interfaces/ipatchsource.h>

class PatchHighlighter;
class PatchReviewToolViewFactory;

class QTimer;

namespace KDevelop {
class IDocument;
}
namespace Sublime {
class Area;
}

namespace Diff2
{
class KompareModelList;
class DiffModel;
}
namespace Kompare
{
struct Info;
}

class DiffSettings;
class PatchReviewPlugin;

class PatchReviewPlugin : public KDevelop::IPlugin, public KDevelop::IPatchReview
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IPatchReview )

public :
    PatchReviewPlugin( QObject *parent, const QVariantList & = QVariantList() );
    ~PatchReviewPlugin();
    virtual void unload();

    QWidget* createToolView( QWidget* parent );

    KDevelop::IPatchSource::Ptr patch() const {
        return m_patch;
    }

    Diff2::KompareModelList* modelList() const {
        return m_modelList.data();
    }

    void seekHunk( bool forwards, const QUrl& file = QUrl() );

    void setPatch( KDevelop::IPatchSource* patch );

    virtual void startReview( KDevelop::IPatchSource* patch, ReviewMode mode );

    void finishReview( QList< QUrl > selection );

    QUrl urlForFileModel( const Diff2::DiffModel* model );
    QAction* finishReviewAction() const { return m_finishReview; }

Q_SIGNALS:
    void startingNewReview();
    void patchChanged();

public Q_SLOTS :
    //Does parts of the review-starting that are problematic to do directly in startReview, as they may open dialogs etc.
    void updateReview();

    void cancelReview();
    void clearPatch( QObject* patch );
    void notifyPatchChanged();
    void highlightPatch();
    void updateKompareModel();
    void forceUpdate();
    void areaChanged(Sublime::Area* area);

private Q_SLOTS :
    void documentClosed( KDevelop::IDocument* );
    void textDocumentCreated( KDevelop::IDocument* );
    void documentSaved( KDevelop::IDocument* );
    void exporterSelected( QAction* action );
    void closeReview();

private:
    // Switches to the review area,
    // makes sure that the working set active in the current area starts with "review" and
    // is not active in any other area. Creates new working sets if required.
    void switchAreaAndMakeWorkingSetUique();
    // Returns whether the current working set is active only in this area
    bool isWorkingSetUnique() const;
    // Makes sure that this working set is active only in this area, and that its name starts with "review".
    // Returns false on failure (if the user disagreed).
    bool setUniqueEmptyWorkingSet();

    void addHighlighting( const QUrl& file, KDevelop::IDocument* document = 0 );
    void removeHighlighting( const QUrl& file = QUrl() );

    KDevelop::IPatchSource::Ptr m_patch;

    QTimer* m_updateKompareTimer;

    PatchReviewToolViewFactory* m_factory;
    QAction* m_finishReview;

    #if 0
    void determineState();
    #endif

    QPointer< DiffSettings > m_diffSettings;
    QScopedPointer< Kompare::Info > m_kompareInfo;
    QScopedPointer< Diff2::KompareModelList > m_modelList;
    typedef QMap< QUrl, QPointer< PatchHighlighter > > HighlightMap;
    HighlightMap m_highlighters;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
