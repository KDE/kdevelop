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

#ifndef EDITPATCH_H
#define EDITPATCH_H

#include <QObject>
#include <QPointer>
#include <memory>
#include <qstandarditemmodel.h>
#include <interfaces/iplugin.h>
#include "localpatchsource.h"
#include "ui_patchreview.h"

class PatchReviewToolViewFactory;

namespace KParts {
class Part;
}

class QDialog;

namespace Diff2 {
class KompareModelList;
class DiffModel;
}
namespace KTextEditor {
class SmartRange;
}
namespace Kompare {
class Info;
}
namespace KDevelop {
class IDocument;
}

///Delete itself when the document(or textDocument), or Diff-Model is deleted.
class DocumentHighlighter : public QObject {
    Q_OBJECT
public:
    DocumentHighlighter( const Diff2::DiffModel* model, KDevelop::IDocument* doc, bool isSource ) throw( QString );
    ~DocumentHighlighter();
    KDevelop::IDocument* doc();
private slots:
    void documentDestroyed();
private:
    QList<KTextEditor::SmartRange*> m_ranges;
    KDevelop::IDocument* m_doc;
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
    void slotStateChanged();

    void fileDoubleClicked( const QModelIndex& i );
    void fileSelectionChanged();

    void nextHunk();
    void prevHunk();

    void patchChanged();

    void updatePatchFromEdit();

    void slotEditCommandChanged();

    void slotEditFileNameChanged();

private:
    void kompareModelChanged();

    void showEditDialog();
    void editPatchReadOnly();
    ///Returns the edit-state visible in the UI
    LocalPatchSource::State editState();
    ///Fills the editor views from m_editingPatch
    void fillEditFromPatch();

    LocalPatchSource::State m_actionState;

    Ui_EditPatch m_editPatch;

    QTime m_lastDataTime;
    QString m_lastTerminalData;

    QPointer<KParts::Part> m_konsolePart;

    bool m_reversed;
    QStandardItemModel* m_filesModel;

    PatchReviewPlugin* m_plugin;
};

class PatchReviewPlugin : public KDevelop::IPlugin {
    Q_OBJECT
public:
    PatchReviewPlugin(QObject *parent, const QVariantList & = QVariantList() );
    ~PatchReviewPlugin();
    virtual void unload();

    QWidget* createToolView(QWidget* parent);

    LocalPatchSourcePointer patch() const {
        return m_patch;
    }

    bool isSource() const {
        return m_isSource;
    }

    Diff2::KompareModelList* modelList() const {
        return m_modelList.get();
    }

    void notifyPatchChanged();

    void seekHunk( bool forwards, const QString& file = QString() );

Q_SIGNALS:
    void patchChanged();

public Q_SLOTS:
    void  highlightPatch();

private Q_SLOTS:
    void updateKompareModel();

private:

    void removeHighlighting( const QString& file = QString() );

    LocalPatchSourcePointer m_patch;
    bool m_isSource;

    QTimer* m_updateKompareTimer;

    PatchReviewToolViewFactory* m_factory;

    void determineState();

    QPointer<DiffSettings> m_diffSettings;
    std::auto_ptr<Kompare::Info> m_kompareInfo;
    std::auto_ptr<Diff2::KompareModelList> m_modelList;
    KUrl m_lastModelFile;
    typedef QMap<QString, QPointer<DocumentHighlighter> > HighlightMap;
    HighlightMap m_highlighters;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
