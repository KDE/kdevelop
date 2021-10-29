/*
    SPDX-FileCopyrightText: 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PATCHREVIEWTOOLVIEW_H
#define KDEVPLATFORM_PLUGIN_PATCHREVIEWTOOLVIEW_H

#include <QWidget>
#include <QPointer>
#include <QTime>

#include <ui_patchreview.h>

namespace Sublime { class Area; }
namespace KDevelop { class IDocument; }
namespace Purpose { class Menu; }

class QStandardItem;
class KJob;
class PatchReviewPlugin;
class LocalPatchSource;
class QModelIndex;
class QSortFilterProxyModel;

class PatchReviewToolView : public QWidget
{
    Q_OBJECT
public:
    PatchReviewToolView( QWidget* parent, PatchReviewPlugin* plugin );
    ~PatchReviewToolView() override;

    // opens the document which should be part of the patch,
    // ensuring that the tabs are sorted correctly
    void open( const QUrl& url, bool activate ) const;

Q_SIGNALS:
    void dialogClosed( PatchReviewToolView* );
    void  stateChanged( PatchReviewToolView* );
private Q_SLOTS:

    void startingNewReview();

    void fileDoubleClicked( const QModelIndex& i );

    void nextHunk();
    void prevHunk();
    void prevFile();
    void nextFile();
    void seekFile(bool forwards);

    void patchChanged();

    void slotAppliedChanged( int newState );

    void finishReview();

    void runTests();

    void selectAll();
    void deselectAll();

    void fileItemChanged( QStandardItem* item );

private:
    void resizeEvent(QResizeEvent*) override;

    void kompareModelChanged();

    void showEditDialog();
    ///Fills the editor views from m_editingPatch
    void fillEditFromPatch();
    /// Retrieve the patch from plugin and perform all necessary casts
    LocalPatchSource* GetLocalPatchSource();

    Ui_EditPatch m_editPatch;

    QTime m_lastDataTime;
    QString m_lastTerminalData;

    /// Whether the set of checked URLs should be reset on the next update
    bool m_resetCheckedUrls;

    PatchReviewPlugin* m_plugin;

    QPointer< QWidget > m_customWidget;
    QAction* m_selectAllAction;
    QAction* m_deselectAllAction;
    Purpose::Menu* m_exportMenu;

    class PatchFilesModel* m_fileModel;
    QSortFilterProxyModel* m_fileSortProxyModel;

public Q_SLOTS:
    void documentActivated( KDevelop::IDocument* );
    void customContextMenuRequested(const QPoint& p);
    void testJobResult(KJob* job);
    void testJobPercent(KJob* job, unsigned long percent);
};

#endif // KDEVPLATFORM_PLUGIN_PATCHREVIEWTOOLVIEW_H
