/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2006, 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "breakpointmodel.h"

#include <QIcon>
#include <QPixmap>
#include <QPointer>
#include <QTimer>

#include <KLocalizedString>
#include <KTextEditor/Document>

#include "../interfaces/icore.h"
#include "../interfaces/idebugcontroller.h"
#include "../interfaces/idocumentcontroller.h"
#include "../interfaces/idocument.h"
#include <interfaces/idebugsession.h>
#include <interfaces/ibreakpointcontroller.h>
#include <interfaces/isession.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>

#include <debug.h>
#include "breakpoint.h"
#include <KConfigGroup>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <algorithm>
#include <utility>
#include <vector>

#define IF_DEBUG(x)

using namespace KDevelop;
using namespace KTextEditor;

namespace {

IBreakpointController* breakpointController()
{
    KDevelop::ICore* core = KDevelop::ICore::self();
    if (!core) {
        return nullptr;
    }
    IDebugController* controller = core->debugController();
    if (!controller) {
        return nullptr;
    }
    IDebugSession* session = controller->currentSession();
    return session ? session->breakpointController() : nullptr;
}

enum class ReloadState : unsigned char {
    Idle,
    StartedReloading,
    ZeroBreakpoints,
    CleanUpMarksAndReinitialize,
    Reinitialize
};

} // anonymous namespace

class KDevelop::BreakpointModelPrivate
{
public:
    bool dirty = false;
    ReloadState reloadState = ReloadState::Idle;
    /// Non-zero while KDevelop code is adding or removing a document mark.
    /// This allows to react to user-driven mark changes without getting confused by our own code changes.
    NonNegative<> inhibitMarkChange;
    QList<Breakpoint*> breakpoints;
    /// FIXME: this is just an ugly workaround to not leak deleted breakpoints
    ///        a real fix would make sure that we actually delete breakpoints
    ///        right when we delete them... aka remove Breakpoint::{set}deleted
    QList<Breakpoint*> deletedBreakpoints;

    QPointer<Sublime::Message> noBreakpointsInUntitledDocumentMessage;
};

BreakpointModel::BreakpointModel(QObject* parent)
    : QAbstractTableModel(parent),
      d_ptr(new BreakpointModelPrivate)
{
    auto* const documentController = ICore::self()->documentController();
    Q_ASSERT(documentController); // BreakpointModel is created after DocumentController.

    // This constructor is invoked before controllers are initialized, and thus before any documents can be opened.
    // So our textDocumentCreated() slot will be invoked for all documents.
    Q_ASSERT(documentController->openDocuments().empty());
    connect(documentController, &IDocumentController::textDocumentCreated, this, &BreakpointModel::textDocumentCreated);

    connect(documentController, &IDocumentController::documentUrlChanged, this, &BreakpointModel::documentUrlChanged);
    connect(documentController, &IDocumentController::documentSaved, this, &BreakpointModel::documentSaved);
}

BreakpointModel::~BreakpointModel()
{
    Q_D(BreakpointModel);

    qDeleteAll(d->breakpoints);
    qDeleteAll(d->deletedBreakpoints);
}

void BreakpointModel::textDocumentCreated(KDevelop::IDocument* doc)
{
    KTextEditor::Document* const textDocument = doc->textDocument();
    Q_ASSERT(textDocument);

    textDocument->setMarkDescription(BreakpointMark, i18n("Breakpoint"));
    textDocument->setMarkIcon(BreakpointMark, *breakpointPixmap());
    textDocument->setMarkIcon(PendingBreakpointMark, *pendingBreakpointPixmap());
    textDocument->setMarkIcon(ReachedBreakpointMark, *reachedBreakpointPixmap());
    textDocument->setMarkIcon(DisabledBreakpointMark, *disabledBreakpointPixmap());

    // We forbid adding breakpoints to an untitled/unsaved document. Such a document's URL is empty,
    // in which case we don't enable breakpoint actions yet.
    if (textDocument->url().isEmpty()) {
        textDocument->setEditableMarks(MarkType::Bookmark);
    } else {
        textDocument->setEditableMarks(MarkType::Bookmark | BreakpointMark);

        // Set up breakpoints *before* connecting to the document's signals.
        setupDocumentBreakpoints(*textDocument);
    }

    connect(textDocument, &KTextEditor::Document::markChanged, this, &BreakpointModel::markChanged);
    connect(textDocument, &KTextEditor::Document::markContextMenuRequested, this,
            &BreakpointModel::markContextMenuRequested);

    connect(textDocument, &KTextEditor::Document::aboutToReload, this, &BreakpointModel::aboutToReload);
    connect(textDocument, &KTextEditor::Document::aboutToInvalidateMovingInterfaceContent, this,
            &BreakpointModel::aboutToInvalidateMovingInterfaceContent);
    connect(textDocument, &KTextEditor::Document::reloaded, this, &BreakpointModel::reloaded);
}

void BreakpointModel::setupDocumentBreakpoints(KTextEditor::Document& document) const
{
    Q_D(const BreakpointModel);

    // Initial setup of moving cursors and marks.
    const QUrl docUrl = document.url();
    // We forbid breakpoints in documents with empty URLs. A breakpoint's empty URL value means "none".
    Q_ASSERT(!docUrl.isEmpty());

    const auto docLineCount = document.lines();
    for (Breakpoint* breakpoint : std::as_const(d->breakpoints)) {
        if (breakpoint->kind() == Breakpoint::CodeBreakpoint && docUrl == breakpoint->url()) {
            const auto savedLine = breakpoint->savedLine();
            if (savedLine >= 0 && savedLine < docLineCount) {
                setupMovingCursor(breakpoint, &document, savedLine);
            }
        }
    }
}

void BreakpointModel::detachDocumentBreakpoints(KTextEditor::Document& document) const
{
    Q_D(const BreakpointModel);

    for (auto* const breakpoint : std::as_const(d->breakpoints)) {
        if (breakpoint->movingCursor() && breakpoint->movingCursor()->document() == &document) {
            breakpoint->stopDocumentLineTracking();
        }
    }
}

void BreakpointModel::documentUrlChanged(KDevelop::IDocument* document, const QUrl& previousUrl)
{
    Q_D(BreakpointModel);

    // Ignore non-text documents.
    if (!document->textDocument()) {
        return;
    }

    // We rely on previousUrl being an IDocument::url(), which is unique and never empty.
    // An empty previousUrl is unacceptable, because a breakpoint's empty URL
    // value means "none" and must not be replaced in the loop below.
    Q_ASSERT(!previousUrl.isEmpty());

    std::vector<Breakpoint*> updatedBreakpoints;
    const auto destinationUrl = document->url();
    for (auto* const breakpoint : std::as_const(d->breakpoints)) {
        // Ignore unsuitable breakpoints.
        if (breakpoint->kind() != Breakpoint::CodeBreakpoint || breakpoint->url() != previousUrl) {
            continue;
        }
        // Update the breakpoint to use the (renamed to) destinationUrl.
        breakpoint->assignUrl(destinationUrl);
        updatedBreakpoints.push_back(breakpoint);
    }

    // The loop above silently reassigns URLs to restore consistency between the Breakpoint URLs and moving cursors.
    // Notify the world about the URL changes in this separate loop to avoid possible inconsistency issues.
    for (auto* const breakpoint : updatedBreakpoints) {
        reportChange(breakpoint, Breakpoint::LocationColumn);
    }
}

void BreakpointModel::removeBreakpointMarks(KTextEditor::Document& document)
{
    const auto& marks = document.marks();
    if (marks.empty()) {
        return;
    }

    const auto guard = markChangeGuard();

    // A call to KTextEditor::Document::removeMark() usually erases an element from the document's marks() container.
    // Erasing an element can invalidate iterators and thus requires iterating over a copy.
    // Iterating over a copy of the marks() container would almost certainly detach it and thus end implicit sharing.
    // Iterate over a local container of keys both for code simplicity (no need for QHash::key_value_iterator)
    // and performance (the QList is smaller than the QHash and is ideal for the loop below).
    const auto markLines = marks.keys();
    for (const int line : markLines) {
        document.removeMark(line, AllBreakpointMarks);
    }
}

void BreakpointModel::aboutToReload()
{
    Q_D(BreakpointModel);

    Q_ASSERT(d->reloadState == ReloadState::Idle);
    d->reloadState = ReloadState::StartedReloading;

    // KTextEditor::DocumentPrivate::documentReload() stores in a local variable all document marks right after
    // emitting aboutToReload(). Then the reloading process removes all document marks, unless the document is
    // modified and the user chooses to cancel reloading when prompted.
    // KTextEditor::DocumentPrivate::documentReload() then re-adds all document marks and emits reloaded().
    // This reloading process emits markChanged() at least once per mark-ed document line.
    // Inhibit mark change for the duration of the reload to prevent spurious mark changes.
    ++d->inhibitMarkChange;
}

void BreakpointModel::aboutToInvalidateMovingInterfaceContent(KTextEditor::Document* document)
{
    Q_D(BreakpointModel);

    if (d->reloadState == ReloadState::Idle) {
        // The document's text is about to be discarded. Remove all breakpoint marks and moving cursors from the
        // document to preserve currently line-tracking breakpoints and their saved line numbers.
        qCWarning(DEBUGGER) << "deactivating all breakpoints in" << document->url().toString(QUrl::PreferLocalFile)
                            << "due to moving interface content invalidation";
        detachDocumentBreakpoints(*document);

        if (document->url().isEmpty()) {
            // This happens when a document's file is deleted externally and the user presses the Close File button in
            // the prompt that appears above the editor. In this case the KTextEditor::Document's URL is temporarily
            // empty, because KTextEditor::DocumentPrivate::closeUrl() is called before its TextDocument is closed.
            // A breakpoint mark's line can get out of sync with its associated moving cursor's tracked line as
            // described in a comment about document line tracking in breakpoint.h. An out-of-sync breakpoint mark
            // remains even after the detachDocumentBreakpoints() call above. Then
            // KTextEditor::DocumentPrivate::clearMarks() removes the breakpoint mark and the assertion
            // !document->url().isEmpty() in BreakpointModel::markChanged() fails. Work the assertion failure around
            // by removing all remaining breakpoint marks from the document now.
            // TODO: remove this workaround once we require a KTextEditor version where
            // the temporarily empty KTextEditor::Document's URL issue is prevented
            // as the commit message of f47916f4f655a20afade579bda61e5d5754d11dd envisions.
            removeBreakpointMarks(*document);
        }

        return;
    }
    if (d->reloadState != ReloadState::StartedReloading) {
        // This slot has already been invoked before during the current reload, nothing more to be done.
        return;
    }

    // All moving cursors are invalidated in the document after this slot, so they must be dropped
    // now to avoid using invalid line numbers.
    bool reinitializeBreakpoints = false;
    bool hasBreakpointMarks = false;
    const QUrl docUrl = document->url();
    for (auto* const breakpoint : std::as_const(d->breakpoints)) {
        const auto* const cursor = breakpoint->movingCursor();
        if (cursor && cursor->document() == document) {
            reinitializeBreakpoints = true;
            hasBreakpointMarks = true;
            // KTextEditor::Document clears marks soon after the first emission of
            // aboutToInvalidateMovingInterfaceContent() during a reload.
            // Call removeMovingCursor() instead of stopDocumentLineTracking() to avoid removing marks one by one
            // in this loop, because the impending clearing of all marks at once should be more efficient.
            breakpoint->removeMovingCursor();
        } else {
            // Reloading may increase the document's line count. Therefore, we may be able to enable document
            // line tracking for breakpoints that are no longer out of bounds after the reloading.
            // So reinitialize if the document contains any breakpoints.
            reinitializeBreakpoints = reinitializeBreakpoints || docUrl == breakpoint->url();
        }
    }

    if (!reinitializeBreakpoints) {
        d->reloadState = ReloadState::ZeroBreakpoints;
        return;
    }

    // At the end of the reloading process, KTextEditor restores each mark on the line number it had at the time when
    // reloading started, i.e. on its tracked line number, if the line number is still in the document range and the
    // text on this line is unchanged by the reloading. In contrast, KDevelop::reloaded() reinitializes each breakpoint
    // mark at its saved line number, if it is still in the document range. If the reloaded document is unmodified in
    // the editor or the user opts to save the document during reload, the tracked and saved line numbers stay/become
    // equal. If the user opts to discard document changes during reload, the tracked and saved line numbers can differ.
    //
    // Let us consider a breakpoint mark's tracked line number L, which is not equal to the saved line number of
    // the associated breakpoint. If the text on the line L at the time when reloading started happens to match the
    // text on the line L at the time when reloading ends (e.g. if the file was modified on disk in the same way as
    // in the editor), a breakpoint mark unassociated with any breakpoint appears on the line L of the document border.
    //
    // In order to prevent this bug, reloaded() must remove all breakpoint marks in the document (if any exist)
    // before reinitializing breakpoints. This needs to be done only if the user opts to discard document changes
    // during reload. The discard is the only user choice that keeps the document modified at this point.
    d->reloadState = hasBreakpointMarks && document->isModified() ? ReloadState::CleanUpMarksAndReinitialize
                                                                  : ReloadState::Reinitialize;
}

void BreakpointModel::reloaded(KTextEditor::Document* document)
{
    Q_D(BreakpointModel);

    // KTextEditor::DocumentPrivate::documentReload() just re-added all document marks,
    // which were temporarily removed during the reload. So end the mark change inhibition now.
    --d->inhibitMarkChange;

    switch (d->reloadState) {
    case ReloadState::Idle:
        Q_ASSERT_X(false, Q_FUNC_INFO, "KTextEditor::Document did not emit aboutToReload() before reloaded().");
        break;
    case ReloadState::StartedReloading:
        // Moving cursors have not been invalidated, because the user opted to cancel reloading.
        [[fallthrough]];
    case ReloadState::ZeroBreakpoints:
        // There are no breakpoints at the reloaded document's URL.
        break; // nothing to do
    case ReloadState::CleanUpMarksAndReinitialize:
        removeBreakpointMarks(*document);
        [[fallthrough]];
    case ReloadState::Reinitialize:
        // reinitialize
        setupDocumentBreakpoints(*document);
    }

    d->reloadState = ReloadState::Idle;
}

void BreakpointModel::markContextMenuRequested(KTextEditor::Document* document, Mark mark, QPoint pos, bool& handled)
{
    int type = mark.type;
    qCDebug(DEBUGGER) << type;

    if (!(document->editableMarks() & BreakpointMark)) {
        return; // breakpoints are forbidden in this document, so let KTextEditor show its default context menu
    }

    Breakpoint *b = nullptr;
    if ((type & AllBreakpointMarks)) {
        b = breakpoint(document->url(), mark.line);
        if (!b) {
            QMessageBox::critical(nullptr, i18n("Breakpoint not found"), i18n("Couldn't find breakpoint at %1:%2", document->url().toString(), mark.line));
        }
    } else if (!(type & MarkType::Bookmark)) // neither breakpoint nor bookmark
        return;

    QMenu menu; // TODO: needs qwidget
    QAction* breakpointAction = menu.addAction(QIcon::fromTheme(QStringLiteral("breakpoint")), i18n("&Breakpoint"));
    breakpointAction->setCheckable(true);
    breakpointAction->setChecked(b);
    QAction* enableAction = nullptr;
    if (b) {
        enableAction = b->enabled() ?
            menu.addAction(QIcon::fromTheme(QStringLiteral("dialog-cancel")), i18n("&Disable Breakpoint")) :
            menu.addAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), i18n("&Enable Breakpoint"));
    }
    menu.addSeparator();
    QAction* bookmarkAction = menu.addAction(QIcon::fromTheme(QStringLiteral("bookmark-new")), i18n("&Bookmark"));
    bookmarkAction->setCheckable(true);
    bookmarkAction->setChecked((type & MarkType::Bookmark));

    QAction* triggeredAction = menu.exec(pos);
    if (triggeredAction) {
        if (triggeredAction == bookmarkAction) {
            if ((type & MarkType::Bookmark))
                document->removeMark(mark.line, MarkType::Bookmark);
            else
                document->addMark(mark.line, MarkType::Bookmark);
        } else if (triggeredAction == breakpointAction) {
            if (b) {
                removeBreakpoint(b);
            } else {
                addCodeBreakpoint(document->url(), mark.line);
            }
        } else if (triggeredAction == enableAction) {
            b->setData(Breakpoint::EnableColumn, b->enabled() ? Qt::Unchecked : Qt::Checked);
        }
    }

    handled = true;
}


QVariant
BreakpointModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role == Qt::DecorationRole ) {
        if (section == 0)
            return QIcon::fromTheme(QStringLiteral("dialog-ok-apply"));
        else if (section == 1)
            return QIcon::fromTheme(QStringLiteral("system-switch-user"));
    }

    if (role == Qt::DisplayRole) {
        if (section == 0 || section == 1) return QString();
        if (section == 2) return i18n("Type");
        if (section == 3) return i18n("Location");
        if (section == 4) return i18n("Condition");
    }

    if (role == Qt::ToolTipRole) {
        if (section == 0) return i18n("Active status");
        if (section == 1) return i18n("State");
        return headerData(section, orientation, Qt::DisplayRole);

    }
    return QVariant();
}

Qt::ItemFlags BreakpointModel::flags(const QModelIndex &index) const
{
    /* FIXME: all this logic must be in item */
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == 0)
        return static_cast<Qt::ItemFlags>(
            Qt::ItemIsEnabled | Qt::ItemIsSelectable
            | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);

    if (index.column() == Breakpoint::ConditionColumn)
        return static_cast<Qt::ItemFlags>(
            Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

    return static_cast<Qt::ItemFlags>(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

QModelIndex BreakpointModel::breakpointIndex(KDevelop::Breakpoint* b, int column)
{
    Q_D(BreakpointModel);

    int row = d->breakpoints.indexOf(b);
    if (row == -1) return QModelIndex();
    return index(row, column);
}

bool KDevelop::BreakpointModel::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_D(BreakpointModel);

    if (count < 1 || (row < 0) || (row + count) > rowCount(parent))
        return false;

    IBreakpointController* controller = breakpointController();

    beginRemoveRows(parent, row, row+count-1);
    for (int i=0; i < count; ++i) {
        Breakpoint* b = d->breakpoints.at(row);
        b->m_deleted = true;
        if (controller)
            controller->breakpointAboutToBeDeleted(row);

        b->stopDocumentLineTracking();
        d->breakpoints.removeAt(row);
        b->m_model = nullptr;
        // To be changed: the controller is currently still responsible for deleting the breakpoint
        // object
        // FIXME: this whole notion of m_deleted is utterly broken and needs to be fixed properly
        // for now just prevent a leak...
        d->deletedBreakpoints.append(b);
    }
    endRemoveRows();
    scheduleSave();
    return true;
}

int KDevelop::BreakpointModel::rowCount(const QModelIndex& parent) const
{
    Q_D(const BreakpointModel);

    if (!parent.isValid()) {
        return d->breakpoints.count();
    }
    return 0;
}

int KDevelop::BreakpointModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant BreakpointModel::data(const QModelIndex& index, int role) const
{
    Q_D(const BreakpointModel);

    if (!index.parent().isValid() && index.row() < d->breakpoints.count()) {
        return d->breakpoints.at(index.row())->data(index.column(), role);
    }
    return QVariant();
}

bool KDevelop::BreakpointModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_D(const BreakpointModel);

    if (!index.parent().isValid() && index.row() < d->breakpoints.count() && (role == Qt::EditRole || role == Qt::CheckStateRole)) {
        return d->breakpoints.at(index.row())->setData(index.column(), value);
    }
    return false;
}

void BreakpointModel::updateErrorText(int row, const QString& errorText)
{
    Q_D(BreakpointModel);

    Breakpoint* breakpoint = d->breakpoints.at(row);
    if (breakpoint->m_errorText != errorText) {
        breakpoint->m_errorText = errorText;
        reportChange(breakpoint, Breakpoint::StateColumn);
    }
}

void BreakpointModel::notifyHit(int row)
{
    emit hit(row);
}

void BreakpointModel::markChanged(KTextEditor::Document* document, KTextEditor::Mark mark,
    KTextEditor::Document::MarkChangeAction action)
{
    Q_D(const BreakpointModel);

    int type = mark.type;
    /* Is this a breakpoint mark, to begin with? */
    if (!(type & AllBreakpointMarks)) return;

    // This slot's only purpose is to add or remove a breakpoint in response to explicit user action.
    // Therefore, the slot is inhibited and returns early when invoked during programmatic mark changes.
    // TODO: in certain rare and unlikely scenarios, such as a call to KTextEditor::Document::setText(),
    // this slot is not inhibited when it should be. As a result, some breakpoint data (e.g. enabled
    // state, line number or breakpoint condition) can be lost. Changes in KTextEditor implementation
    // and even API additions are necessary to ensure proper inhibition.

    if (d->inhibitMarkChange)
        return;

    Q_ASSERT_X(!document->url().isEmpty(), Q_FUNC_INFO,
               "Somehow a breakpoint mark appeared in an untitled/unsaved document. This is not supported.");

    if (action == KTextEditor::Document::MarkAdded) {
        Breakpoint *b = breakpoint(document->url(), mark.line);
        if (b) {
            // This happens when the user Ctrl+clicks a mark of a breakpoint type
            // other than BreakpointActive (e.g. BreakpointDisabled) on a text editor border.
            // Delete the found breakpoint instance.
            removeBreakpoint(b);
            return;
        }
        // This happens when the user Ctrl+clicks a mark-less place of
        // a text editor border or adds a new breakpoint via the mark context menu.
        addCodeBreakpoint(document->url(), mark.line);
    } else {
        // This happens when the user Ctrl+clicks a BreakpointActive mark on a text editor border.
        // Find this breakpoint instance and delete it.
        Breakpoint *b = breakpoint(document->url(), mark.line);
        if (b) {
            removeBreakpoint(b);
        }
    }

#if 0
    if ( KDevelop::ICore::self()->documentController()->activeDocument() && KDevelop::ICore::self()->documentController()->activeDocument()->textDocument() == document )
    {
        //bring focus back to the editor
        // TODO probably want a different command here
        KDevelop::ICore::self()->documentController()->activateDocument(KDevelop::ICore::self()->documentController()->activeDocument());
    }
#endif
}

static constexpr int breakpointMarkPixmapSize = 32;

const QPixmap* BreakpointModel::breakpointPixmap()
{
    static QPixmap pixmap=QIcon::fromTheme(QStringLiteral("breakpoint")).pixmap(QSize(breakpointMarkPixmapSize, breakpointMarkPixmapSize), QIcon::Active, QIcon::Off);
    return &pixmap;
}

const QPixmap* BreakpointModel::pendingBreakpointPixmap()
{
    static QPixmap pixmap=QIcon::fromTheme(QStringLiteral("breakpoint")).pixmap(QSize(breakpointMarkPixmapSize, breakpointMarkPixmapSize), QIcon::Normal, QIcon::Off);
    return &pixmap;
}

const QPixmap* BreakpointModel::reachedBreakpointPixmap()
{
    static QPixmap pixmap=QIcon::fromTheme(QStringLiteral("breakpoint")).pixmap(QSize(breakpointMarkPixmapSize, breakpointMarkPixmapSize), QIcon::Selected, QIcon::Off);
    return &pixmap;
}

const QPixmap* BreakpointModel::disabledBreakpointPixmap()
{
    static QPixmap pixmap=QIcon::fromTheme(QStringLiteral("breakpoint")).pixmap(QSize(breakpointMarkPixmapSize, breakpointMarkPixmapSize), QIcon::Disabled, QIcon::Off);
    return &pixmap;
}

void BreakpointModel::removeBreakpoint(Breakpoint* breakpoint)
{
    Q_D(BreakpointModel);

    Q_ASSERT(breakpoint);
    const auto row = d->breakpoints.indexOf(breakpoint);
    Q_ASSERT(row != -1);
    removeRow(row);
}

void BreakpointModel::toggleBreakpoint(const QUrl& url, const KTextEditor::Cursor& cursor)
{
    Q_D(BreakpointModel);

    Breakpoint *b = breakpoint(url, cursor.line());
    if (b) {
        removeBreakpoint(b);
    } else {
        // We forbid adding breakpoints to an untitled/unsaved document by not enabling its breakpoint actions.
        // The Toggle Breakpoint action is always enabled though, hence the check and early-return below.
        const auto* const documentController = ICore::self()->documentController();
        const auto* const doc = documentController ? documentController->documentForUrl(url) : nullptr;
        const auto* const textDocument = doc ? doc->textDocument() : nullptr;
        if (textDocument && !(textDocument->editableMarks() & BreakpointMark)) {
            // Discard the previous message (if any) to prevent stacking of identical messages,
            // which the user must dismiss one by one.
            delete d->noBreakpointsInUntitledDocumentMessage.data();

            auto* const message = new Sublime::Message(
                i18n("A breakpoint cannot be added to an untitled document. Please save the document first."),
                Sublime::Message::Error);
            ICore::self()->uiController()->postMessage(message);
            d->noBreakpointsInUntitledDocumentMessage = message;
            return;
        }

        addCodeBreakpoint(url, cursor.line());
    }
}

void BreakpointModel::reportChange(Breakpoint* breakpoint, Breakpoint::Column column)
{
    Q_D(BreakpointModel);

    // note: just a portion of Breakpoint::Column is displayed in this model!
    if (column >= 0 && column < columnCount()) {
        QModelIndex idx = breakpointIndex(breakpoint, column);
        Q_ASSERT(idx.isValid()); // make sure we don't pass invalid indices to dataChanged()
        emit dataChanged(idx, idx);
    }

    if (IBreakpointController* controller = breakpointController()) {
        int row = d->breakpoints.indexOf(breakpoint);
        Q_ASSERT(row != -1);
        controller->breakpointModelChanged(row, ColumnFlags(1 << column));
    }

    scheduleSave();
}

ScopedIncrementor<> BreakpointModel::markChangeGuard()
{
    Q_D(BreakpointModel);

    return ScopedIncrementor(d->inhibitMarkChange);
}

void BreakpointModel::documentSaved(KDevelop::IDocument* doc)
{
    Q_D(BreakpointModel);

    IF_DEBUG( qCDebug(DEBUGGER); )

    auto* const textDocument = doc->textDocument();
    if (!textDocument) {
        return;
    }

    // We forbid adding breakpoints to an untitled/unsaved document by not enabling its breakpoint actions.
    // This document might have been untitled before the saving, so enable its breakpoint actions now.
    Q_ASSERT(!textDocument->url().isEmpty());
    textDocument->setEditableMarks(MarkType::Bookmark | BreakpointMark);

    // save breakpoints in the given document.
    for (Breakpoint* breakpoint : std::as_const(d->breakpoints)) {
        if (breakpoint->movingCursor()) {
            if (breakpoint->movingCursor()->document() != textDocument)
                continue;
            breakpoint->saveMovingCursorLine();

            // FIXME: temporary code to update the breakpoint widget UI.
            //        marksChanged() slot should update the UI so following is not needed:
            reportChange(breakpoint, Breakpoint::LocationColumn);
        }
    }

    scheduleSave();
}
void BreakpointModel::aboutToDeleteMovingInterfaceContent(KTextEditor::Document* document)
{
    detachDocumentBreakpoints(*document);
}

void BreakpointModel::load()
{
    KConfigGroup breakpoints = ICore::self()->activeSession()->config()->group(QStringLiteral("Breakpoints"));
    int count = breakpoints.readEntry("number", 0);
    if (count == 0)
        return;

    beginInsertRows(QModelIndex(), 0, count - 1);
    for (int i = 0; i < count; ++i) {
        if (!breakpoints.group(QString::number(i)).readEntry("kind", "").isEmpty()) {
            new Breakpoint(this, breakpoints.group(QString::number(i)));
        }
    }
    endInsertRows();
}

void BreakpointModel::save()
{
    Q_D(BreakpointModel);

    d->dirty = false;

    auto* const activeSession = ICore::self()->activeSession();
    if (!activeSession) {
        qCDebug(DEBUGGER) << "Cannot save breakpoints because there is no active session. "
                             "KDevelop must be exiting and already past SessionController::cleanup().";
        return;
    }

    KConfigGroup breakpoints = activeSession->config()->group(QStringLiteral("Breakpoints"));
    breakpoints.writeEntry("number", d->breakpoints.count());
    int i = 0;
    for (Breakpoint* b : std::as_const(d->breakpoints)) {
        KConfigGroup g = breakpoints.group(QString::number(i));
        b->save(g);
        ++i;
    }
    breakpoints.sync();
}

void BreakpointModel::scheduleSave()
{
    Q_D(BreakpointModel);

    if (d->dirty)
        return;

    d->dirty = true;
    QTimer::singleShot(0, this, &BreakpointModel::save);
}

QList<Breakpoint*> KDevelop::BreakpointModel::breakpoints() const
{
    Q_D(const BreakpointModel);

    return d->breakpoints;
}

Breakpoint* BreakpointModel::breakpoint(int row) const
{
    Q_D(const BreakpointModel);

    if (row >= d->breakpoints.count()) return nullptr;
    return d->breakpoints.at(row);
}

Breakpoint* BreakpointModel::addCodeBreakpoint()
{
    Q_D(BreakpointModel);

    beginInsertRows(QModelIndex(), d->breakpoints.count(), d->breakpoints.count());
    auto* n = new Breakpoint(this, Breakpoint::CodeBreakpoint);
    endInsertRows();
    return n;
}

Breakpoint* BreakpointModel::addCodeBreakpoint(const QUrl& url, int line)
{
    Breakpoint* n = addCodeBreakpoint();
    n->setLocation(url, line);
    return n;
}

Breakpoint* BreakpointModel::addCodeBreakpoint(const QString& expression)
{
    Breakpoint* n = addCodeBreakpoint();
    n->setExpression(expression);
    return n;
}

Breakpoint* BreakpointModel::addWatchpoint()
{
    Q_D(BreakpointModel);

    beginInsertRows(QModelIndex(), d->breakpoints.count(), d->breakpoints.count());
    auto* n = new Breakpoint(this, Breakpoint::WriteBreakpoint);
    endInsertRows();
    return n;
}

Breakpoint* BreakpointModel::addWatchpoint(const QString& expression)
{
    Breakpoint* n = addWatchpoint();
    n->setExpression(expression);
    return n;
}

Breakpoint* BreakpointModel::addReadWatchpoint()
{
    Q_D(BreakpointModel);

    beginInsertRows(QModelIndex(), d->breakpoints.count(), d->breakpoints.count());
    auto* n = new Breakpoint(this, Breakpoint::ReadBreakpoint);
    endInsertRows();
    return n;
}

Breakpoint* BreakpointModel::addReadWatchpoint(const QString& expression)
{
    Breakpoint* n = addReadWatchpoint();
    n->setExpression(expression);
    return n;
}

Breakpoint* BreakpointModel::addAccessWatchpoint()
{
    Q_D(BreakpointModel);

    beginInsertRows(QModelIndex(), d->breakpoints.count(), d->breakpoints.count());
    auto* n = new Breakpoint(this, Breakpoint::AccessBreakpoint);
    endInsertRows();
    return n;
}


Breakpoint* BreakpointModel::addAccessWatchpoint(const QString& expression)
{
    Breakpoint* n = addAccessWatchpoint();
    n->setExpression(expression);
    return n;
}

void BreakpointModel::registerBreakpoint(Breakpoint* breakpoint)
{
    Q_D(BreakpointModel);

    Q_ASSERT(!d->breakpoints.contains(breakpoint));
    int row = d->breakpoints.size();
    d->breakpoints << breakpoint;
    if (IBreakpointController* controller = breakpointController()) {
        controller->breakpointAdded(row);
    }
    scheduleSave();
}

Breakpoint* BreakpointModel::breakpoint(const QUrl& url, int line) const
{
    Q_D(const BreakpointModel);

    auto it = std::find_if(d->breakpoints.constBegin(), d->breakpoints.constEnd(), [&](Breakpoint* b) {
        return (b->url() == url && b->line() == line);
    });
    return (it != d->breakpoints.constEnd()) ? *it : nullptr;
}

// TODO: move into the Breakpoint class when/if the aboutToDeleteMovingInterfaceContent()
//       connection is moved to textDocumentCreated().
void BreakpointModel::setupMovingCursor(Breakpoint* breakpoint, KTextEditor::Document* document, int line) const
{
    Q_ASSERT(breakpoint);
    Q_ASSERT(document);
    Q_ASSERT(line >= 0);
    Q_ASSERT(line < document->lines());

    connect(document, &Document::aboutToDeleteMovingInterfaceContent, this,
            &BreakpointModel::aboutToDeleteMovingInterfaceContent, Qt::UniqueConnection);

    breakpoint->restartDocumentLineTrackingAt(document->newMovingCursor(KTextEditor::Cursor(line, 0)));
}

#include "moc_breakpointmodel.cpp"
