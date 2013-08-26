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
#include <memory>
#include <qstandarditemmodel.h>
#include <interfaces/iplugin.h>
#include "localpatchsource.h"
#include "ui_patchreview.h"
#include <language/duchain/indexedstring.h>
#include "libdiff2/diffmodel.h"


class PatchHighlighter;
namespace KDevelop {
class IDocument;
}
namespace Sublime {
class Area;
}

class PatchReviewToolViewFactory;

namespace Diff2
{
class KompareModelList;
class DiffModel;
}
namespace Kompare
{
class Info;
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

    QList< KDevelop::IPatchSource::Ptr > knownPatches() const {
        return m_knownPatches;
    }

    Diff2::KompareModelList* modelList() const {
        return m_modelList.get();
    }

    void seekHunk( bool forwards, const KUrl& file = KUrl() );

    void setPatch( KDevelop::IPatchSource* patch );

    void registerPatch( KDevelop::IPatchSource::Ptr patch );

    virtual void startReview( KDevelop::IPatchSource* patch, ReviewMode mode );

    void finishReview( QList< KUrl > selection );

    KUrl urlForFileModel( const Diff2::DiffModel* model );

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

    QList< KDevelop::IPatchSource::Ptr > m_knownPatches;

    void addHighlighting( const KUrl& file, KDevelop::IDocument* document = 0 );
    void removeHighlighting( const KUrl& file = KUrl() );

    KDevelop::IPatchSource::Ptr m_patch;

    QTimer* m_updateKompareTimer;

    PatchReviewToolViewFactory* m_factory;

    #if 0
    void determineState();
    #endif

    QPointer< DiffSettings > m_diffSettings;
    std::auto_ptr< Kompare::Info > m_kompareInfo;
    std::auto_ptr< Diff2::KompareModelList > m_modelList;
    typedef QMap< KUrl, QPointer< PatchHighlighter > > HighlightMap;
    HighlightMap m_highlighters;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
