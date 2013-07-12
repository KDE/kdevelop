/***************************************************************************
   Copyright 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_PATCHREVIEWTOOLVIEW_H
#define KDEVPLATFORM_PLUGIN_PATCHREVIEWTOOLVIEW_H

#include <QWidget>
#include <QPointer>
#include <QTime>
#include <ui_patchreview.h>
#include <interfaces/ipatchsource.h>

class QStandardItem;
class KJob;
namespace KDevelop {
class IDocument;
}

class LocalPatchSource;
class QModelIndex;
class PatchReviewPlugin;
namespace KParts { class Part; }

class PatchReviewToolView : public QWidget
{
    Q_OBJECT
public:
    PatchReviewToolView( QWidget* parent, PatchReviewPlugin* plugin );
    ~PatchReviewToolView();

signals:
    void dialogClosed( PatchReviewToolView* );
    void  stateChanged( PatchReviewToolView* );
private slots:

    void startingNewReview();
    
    void fileDoubleClicked( const QModelIndex& i );

    void nextHunk();
    void prevHunk();
    void prevFile();
    void nextFile();
    void seekFile(bool forwards);

    void patchChanged();

    void updatePatchFromEdit();

    void slotAppliedChanged( int newState );

    void finishReview();

    void runTests();

    void selectAll();
    void deselectAll();
    
    void fileItemChanged( QStandardItem* item );

private:
    void activate( const KUrl& url, KDevelop::IDocument* buddy = 0 ) const;
    
    void kompareModelChanged();

    void showEditDialog();
    ///Fills the editor views from m_editingPatch
    void fillEditFromPatch();
    /// Retrieve the patch from plugin and perform all necessary casts
    LocalPatchSource* GetLocalPatchSource();

    Ui_EditPatch m_editPatch;

    QTime m_lastDataTime;
    QString m_lastTerminalData;

    QPointer< KParts::Part > m_konsolePart;

    /// Whether the set of checked URLs should be reset on the next update
    bool m_resetCheckedUrls;

    PatchReviewPlugin* m_plugin;

    QPointer< QWidget > m_customWidget;
    QAction* m_selectAllAction;
    QAction* m_deselectAllAction;
    
    class PatchFilesModel* m_fileModel;
public slots:
    void documentActivated( KDevelop::IDocument* );
    void patchSelectionChanged( int );
    void customContextMenuRequested(const QPoint& p);
    void testJobResult(KJob* job);
    void testJobPercent(KJob* job, ulong percent);
};

#endif // KDEVPLATFORM_PLUGIN_PATCHREVIEWTOOLVIEW_H
