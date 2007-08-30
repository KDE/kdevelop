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
#include "network/sharedptr.h"
#include "ui_kdevteamwork_editpatch.h"
#include <QPointer>
#include "safelogger.h"
#include <memory>
#include "patchmessage.h"


namespace KParts{
  class Part;
}

class QDialog;

class PatchesManager;
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

class EditPatch : public QObject, public Shared, public SafeLogger {
    Q_OBJECT
  public:
    EditPatch( PatchesManager* parent, LocalPatchSourcePointer patch, bool local );
    ~EditPatch();

    void apply( bool reverse = false, const QString& fileName = "" );

    LocalPatchSourcePointer patch() const;
  signals:
    void dialogClosed( EditPatch* );
    void  stateChanged( EditPatch* );
  private slots:
    void receivedTerminalData( const QString& s );
    void slotEditOk();
    void slotEditCancel();
    void slotEditFileNameChanged( const QString& str );
    void slotEditCommandChanged( const QString& str );
    void slotFileNameEdited();
    void slotCommandEdited();
    void slotEditMimeType( const QString& str );
    void slotEditDialogFinished( int result );
    void slotChooseType();
    void slotStateChanged();
    void slotApplyEditPatch();
    void slotUnapplyEditPatch();
    void slotShowEditPatch();
    void dialogDestroyed();
    void slotDetermineState();
    void slotToFile();
    void slotUserButton();

    void fileDoubleClicked( const QModelIndex& i );
    void fileSelectionChanged();

    void nextHunk();
    void prevHunk();
    void  highlightFile();

    void updateKompareModel();

    void updateByType();
  private:
    void seekHunk( bool forwards, bool isSource, const QString& file = QString() );

    virtual std::string logPrefix();

    void showEditDialog();
    void hideEditDialog();
    void editPatchReadOnly();
    LocalPatchSource::State editState();
    void fillEditFromPatch();
    LocalPatchSourcePointer patchFromEdit();

    void removeHighlighting( const QString& file = QString() );

    LocalPatchSource::State m_actionState;

    ///Gets local patches via a fake-session
    SafeSharedPtr<PatchMessage> getPatchMessage( PatchRequestData::RequestType type );
    ///Tries to locally get the patch-message and then store it to a temporary file. If successful returns the filename.
    KUrl getPatchFile(bool temp = false);

    PatchesManager* m_parent;
    LocalPatchSourcePointer m_editingPatch;
    bool m_editPatchLocal;

    Ui_EditPatch m_editPatch;
    QDialog* m_editDlg;

    QTime m_lastDataTime;
    QString m_lastTerminalData;

    QPointer<KParts::Part> m_konsolePart;

    QTimer* m_updateKompareTimer;

    bool m_reversed;
    bool m_started;
    QStandardItemModel* m_filesModel;
    QPointer<DiffSettings> m_diffSettings;
    auto_ptr<Kompare::Info> m_kompareInfo;
    auto_ptr<Diff2::KompareModelList> m_modelList;
    QString m_lastModelCommand, m_lastModelFile;
    typedef QMap<QString, QPointer<DocumentHighlighter> > HighlightMap;
    HighlightMap m_highlighters;
    bool m_isSource;
};

typedef SharedPtr<EditPatch> EditPatchPointer;

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
