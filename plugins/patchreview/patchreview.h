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

#ifndef PATCHREVIEW_H
#define PATCHREVIEW_H

#include <QObject>
#include <QPointer>
#include <memory>
#include <qstandarditemmodel.h>
#include <interfaces/iplugin.h>
#include "localpatchsource.h"
#include "ui_patchreview.h"
#include <language/duchain/indexedstring.h>
#include <ktexteditor/movingrangefeedback.h>
#include "libdiff2/diffmodel.h"

class PatchReviewToolViewFactory;
class PatchReviewPlugin;

namespace KParts {
class Part;
}

class QDialog;

namespace Diff2 {
class KompareModelList;
class DiffModel;
class DiffModel;
}
namespace KTextEditor {
class Document;
class Range;
class MovingRange;
class Mark;
}
namespace Kompare {
class Info;
}
namespace KDevelop {
class IDocument;
}

///Delete itself when the document(or textDocument), or Diff-Model is deleted.
class PatchHighlighter : public QObject {
    Q_OBJECT
public:
    PatchHighlighter( const Diff2::DiffModel* model, KDevelop::IDocument* doc, PatchReviewPlugin* plugin ) throw( QString );
    ~PatchHighlighter();
    KDevelop::IDocument* doc();
    QList<KTextEditor::MovingRange*> ranges() const {
      return m_differencesForRanges.keys();
    }
private slots:
    void documentDestroyed();
    void aboutToDeleteMovingInterfaceContent(KTextEditor::Document*);
private:

    void addLineMarker(KTextEditor::MovingRange* arg1, Diff2::Difference* arg2);
    void removeLineMarker(KTextEditor::MovingRange* arg1, Diff2::Difference* arg2);
  
    KTextEditor::MovingRange* rangeForMark(KTextEditor::Mark mark);

    void clear();
    QSet<KTextEditor::MovingRange*> m_ranges;
    QMap<KTextEditor::MovingRange*, Diff2::Difference*> m_differencesForRanges;
    KDevelop::IDocument* m_doc;
    PatchReviewPlugin* m_plugin;
    const Diff2::DiffModel* m_model;
public slots:
    void markToolTipRequested(KTextEditor::Document*,KTextEditor::Mark,QPoint,bool&);
    void showToolTipForMark(QPoint arg1, KTextEditor::MovingRange* arg2, QPair< int, int > highlightMark = qMakePair(-1, -1));
    bool isRemoval(Diff2::Difference*);
    bool isInsertion(Diff2::Difference*);
    void markClicked(KTextEditor::Document*,KTextEditor::Mark,bool&);
    void textInserted(KTextEditor::Document*,KTextEditor::Range);
};

class DiffSettings;
class PatchReviewPlugin;

class PatchReviewToolView : public QWidget {
    Q_OBJECT
public:
    PatchReviewToolView( QWidget* parent, PatchReviewPlugin* plugin );
    ~PatchReviewToolView();

signals:
    void dialogClosed( PatchReviewToolView* );
    void  stateChanged( PatchReviewToolView* );
private slots:

    void fileDoubleClicked( const QModelIndex& i );

    void nextHunk();
    void prevHunk();

    void patchChanged();

    void updatePatchFromEdit();

    void slotEditCommandChanged();

    void slotEditFileNameChanged();

    void finishReview();
    
private:
    void kompareModelChanged();

    void showEditDialog();
    ///Fills the editor views from m_editingPatch
    void fillEditFromPatch();

    Ui_EditPatch m_editPatch;

    QTime m_lastDataTime;
    QString m_lastTerminalData;

    QPointer<KParts::Part> m_konsolePart;

    bool m_reversed;

    KUrl urlForFileModel(const Diff2::DiffModel* model);
    
    PatchReviewPlugin* m_plugin;
    
    QPointer<QWidget> m_customWidget;
public slots:
    void documentActivated(KDevelop::IDocument*);
    void patchSelectionChanged(int);
};

class PatchReviewPlugin : public KDevelop::IPlugin, public KDevelop::IPatchReview {
    Q_OBJECT
    Q_INTERFACES( KDevelop::IPatchReview )
    
public:
    PatchReviewPlugin(QObject *parent, const QVariantList & = QVariantList() );
    ~PatchReviewPlugin();
    virtual void unload();

    QWidget* createToolView(QWidget* parent);

    KDevelop::IPatchSource::Ptr patch() const {
        return m_patch;
    }

    QList<KDevelop::IPatchSource::Ptr> knownPatches() const {
      return m_knownPatches;
    }

    Diff2::KompareModelList* modelList() const {
        return m_modelList.get();
    }

    void seekHunk( bool forwards, const KUrl& file = KUrl() );

    KUrl diffFile();
    
    void setPatch(KDevelop::IPatchSource* patch);

    void registerPatch(KDevelop::IPatchSource::Ptr patch);
    
    virtual void startReview(KDevelop::IPatchSource* patch, ReviewMode mode);
    
    void finishReview(QList<KUrl> selection);
    
Q_SIGNALS:
    void patchChanged();

public Q_SLOTS:
    //Does parts of the review-starting that are problematic to do directly in startReview, as they may open dialogs etc.
    void updateReview();
    
    void cancelReview();
    void clearPatch(QObject* patch);
    void notifyPatchChanged();
    void highlightPatch();
    void updateKompareModel();
    void showPatch();
    void forceUpdate();

private Q_SLOTS:
    void documentClosed(KDevelop::IDocument*);
    void textDocumentCreated(KDevelop::IDocument*);
    void exporterSelected(QAction* action);
    
private:
  
    QList<KDevelop::IPatchSource::Ptr> m_knownPatches;

    void addHighlighting( const KUrl& file, KDevelop::IDocument* document = 0 );
    void removeHighlighting( const KUrl& file = KUrl() );

    KDevelop::IPatchSource::Ptr m_patch;

    QTimer* m_updateKompareTimer;

    PatchReviewToolViewFactory* m_factory;

    #if 0
    void determineState();
    #endif

    QPointer<DiffSettings> m_diffSettings;
    std::auto_ptr<Kompare::Info> m_kompareInfo;
    std::auto_ptr<Diff2::KompareModelList> m_modelList;
    typedef QMap<KUrl, QPointer<PatchHighlighter> > HighlightMap;
    HighlightMap m_highlighters;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
