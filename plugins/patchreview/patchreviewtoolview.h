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

#ifndef PATCHREVIEWTOOLVIEW_H
#define PATCHREVIEWTOOLVIEW_H

#include <QWidget>
#include <QPointer>
#include <QTime>
#include <ui_patchreview.h>

namespace KDevelop {
class IDocument;
class ITestSuite;
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

    void fileDoubleClicked( const QModelIndex& i );

    void nextHunk();
    void prevHunk();

    void patchChanged();

    void updatePatchFromEdit();

    void slotAppliedChanged( int newState );

    void finishReview();

    void runTests();

private:
    void kompareModelChanged();

    void showEditDialog();
    ///Fills the editor views from m_editingPatch
    void fillEditFromPatch();
    /// Retrieve the patch from plugin and perform all necessary casts
    LocalPatchSource* GetLocalPatchSource();

    QList<KDevelop::ITestSuite*> testSuites();

    Ui_EditPatch m_editPatch;

    QTime m_lastDataTime;
    QString m_lastTerminalData;

    QPointer< KParts::Part > m_konsolePart;

    bool m_reversed;

    PatchReviewPlugin* m_plugin;

    QPointer< QWidget > m_customWidget;

    class PatchFilesModel* m_fileModel;
public slots:
    void documentActivated( KDevelop::IDocument* );
    void patchSelectionChanged( int );
    void customContextMenuRequested(const QPoint& p);
};

#endif // PATCHREVIEWTOOLVIEW_H
