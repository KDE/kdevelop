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
#include <QTimer>

#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/MovingInterface>

#include "../interfaces/icore.h"
#include "../interfaces/idebugcontroller.h"
#include "../interfaces/idocumentcontroller.h"
#include "../interfaces/idocument.h"
#include <interfaces/idebugsession.h>
#include <interfaces/ibreakpointcontroller.h>
#include <interfaces/isession.h>
#include <debug.h>
#include "breakpoint.h"
#include <KConfigGroup>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <algorithm>
#include <utility>

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

} // anonymous namespace

class KDevelop::BreakpointModelPrivate
{
public:
    bool dirty = false;
    /// Non-zero while KDevelop code is adding or removing a document mark.
    /// This allows to react to user-driven mark changes without getting confused by our own code changes.
    int inhibitMarkChange = 0;
    QList<Breakpoint*> breakpoints;
    /// FIXME: this is just an ugly workaround to not leak deleted breakpoints
    ///        a real fix would make sure that we actually delete breakpoints
    ///        right when we delete them... aka remove Breakpoint::{set}deleted
    QList<Breakpoint*> deletedBreakpoints;
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
    Q_D(const BreakpointModel);

    KTextEditor::Document* const textDocument = doc->textDocument();

    auto* const imark = qobject_cast<KTextEditor::MarkInterface*>(textDocument);
    if (imark) {
        imark->setMarkDescription(BreakpointMark, i18n("Breakpoint"));
        imark->setMarkPixmap(BreakpointMark, *breakpointPixmap());
        imark->setMarkPixmap(PendingBreakpointMark, *pendingBreakpointPixmap());
        imark->setMarkPixmap(ReachedBreakpointMark, *reachedBreakpointPixmap());
        imark->setMarkPixmap(DisabledBreakpointMark, *disabledBreakpointPixmap());
        imark->setEditableMarks(MarkInterface::Bookmark | BreakpointMark);

        // Set up breakpoints *before* connecting to the document's signals.
        setupDocumentBreakpoints(*textDocument);

        // can't use new signal slot syntax here, MarkInterface is not a QObject
        connect(textDocument, SIGNAL(markChanged(KTextEditor::Document*,KTextEditor::Mark,KTextEditor::MarkInterface::MarkChangeAction)),
                 this, SLOT(markChanged(KTextEditor::Document*,KTextEditor::Mark,KTextEditor::MarkInterface::MarkChangeAction)));
        connect(textDocument, SIGNAL(markContextMenuRequested(KTextEditor::Document*,KTextEditor::Mark,QPoint,bool&)),
                SLOT(markContextMenuRequested(KTextEditor::Document*,KTextEditor::Mark,QPoint,bool&)));
    }

    connect(textDocument, &KTextEditor::Document::aboutToReload, this, &BreakpointModel::aboutToReload);
    // can't use new signal/slot syntax here, MovingInterface is not a QObject
    connect(textDocument, SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)), this,
            SLOT(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)));
    connect(textDocument, &KTextEditor::Document::reloaded, this, &BreakpointModel::reloaded);
}

void BreakpointModel::setupDocumentBreakpoints(KTextEditor::Document& document) const
{
    Q_D(const BreakpointModel);
qCritical() << "---- start setupDocumentBreakpoints()";
    // Initial setup of moving cursors and marks.
    const QUrl docUrl = document.url();
    const auto docLineCount = document.lines();
    for (Breakpoint* breakpoint : qAsConst(d->breakpoints)) {
        if (docUrl == breakpoint->url()) {
            const auto savedLine = breakpoint->savedLine();
            if (savedLine >= 0 && savedLine < docLineCount) {
                setupMovingCursor(breakpoint, &document, savedLine);
            }
        }
    }
qCritical() << "---- end setupDocumentBreakpoints()";
}

[[maybe_unused]] bool BreakpointModel::containsBreakpointMarks(const KTextEditor::Document& document)
{
    auto* const imark = qobject_cast<KTextEditor::MarkInterface*>(&document);
    Q_ASSERT(imark);
    const auto& marks = imark->marks();
    return std::any_of(marks.cbegin(), marks.cend(), [](const KTextEditor::Mark* mark) -> bool {
        return mark->type & AllBreakpointMarks;
    });
}

void BreakpointModel::aboutToReload(KTextEditor::Document* document)
{
    Q_D(BreakpointModel);

    qCritical() << "aboutToReload()";



    ++d->inhibitMarkChange;
}

void BreakpointModel::aboutToInvalidateMovingInterfaceContent(KTextEditor::Document* document)
{
    Q_D(BreakpointModel);

    qCritical() << "aboutToInvalidateMovingInterfaceContent()";

    // disconnect ourselves
    { // helper function 1 -> bool
    // can't use new signal/slot syntax here, MovingInterface is not a QObject
    disconnect(document, SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)), this,
               SLOT(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)));
    }

    // All moving cursors are invalidated in the document after this slot, so they must be dropped
    // now to avoid using invalid line numbers. Conveniently, this also removes the associated breakpoint marks.
    bool reinitializeBreakpoints = false;
    const QUrl docUrl = document->url();
    for (auto* const breakpoint : std::as_const(d->breakpoints)) {
        const auto* const cursor = breakpoint->movingCursor();
        if (cursor && cursor->document() == document) {
            reinitializeBreakpoints = true;
            breakpoint->stopDocumentLineTracking();
        } else {
            // Reloading may increase the document's line count. Therefore, we may be able to enable document
            // line tracking for breakpoints that are no longer out of bounds after the reloading.
            // So reinitialize if the document contains any breakpoints.
            reinitializeBreakpoints = reinitializeBreakpoints || docUrl == breakpoint->url();
        }
    }

    // TODO: disable this costly assertion eventually.
    Q_ASSERT(!containsBreakpointMarks(*document));

    // NOTE: KTextEditor::DocumentPrivate::documentReload() stores in a local variable all document marks right after
    //       emitting aboutToReload(). Then the reloading process removes all document marks, unless the document is
    //       modified and the user chooses to cancel reloading when prompted.
    //       KTextEditor::DocumentPrivate::documentReload() then re-adds all document marks and emits reloaded().
    //       This reloading process emits markChanged() at least once per mark-ed document line. The loop above
    //       removed all breakpoint marks in the document. Therefore, only non-breakpoint marks remain. markChanged()
    //       early-returns if the changed mark is not a breakpoint mark. In this way, removing all breakpoint marks
    //       in the loop above prevents spurious mark changes when a document is reloaded.

    if (reinitializeBreakpoints) {
        return;
    }

    { // helper function 2 -> void
    --d->inhibitMarkChange;
    // already disconnected ourselves and un-inhibited mark change => nothing to do in reloaded()
    disconnect(document, &KTextEditor::Document::reloaded, this, &BreakpointModel::reloaded);
    }
}

void BreakpointModel::reloaded(KTextEditor::Document* document)
{
    Q_D(BreakpointModel);

    qCritical() << "reloaded()";

    --d->inhibitMarkChange;

    // disconnect ourselves
    disconnect(document, &KTextEditor::Document::reloaded, this, &BreakpointModel::reloaded);

    // can't use new signal/slot syntax here, MovingInterface is not a QObject
    const bool disconnected = disconnect(document, SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)), this,
                                         SLOT(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)));
    if (disconnected) {
        return; // the moving cursors have not been invalidated => the user selected "Cancel"
    }

    // reinitialize
    setupDocumentBreakpoints(*document);
}

void BreakpointModel::markContextMenuRequested(Document* document, Mark mark, const QPoint &pos, bool& handled)
{
    int type = mark.type;
    qCDebug(DEBUGGER) << type;

    Breakpoint *b = nullptr;
    if ((type & AllBreakpointMarks)) {
        b = breakpoint(document->url(), mark.line);
        if (!b) {
            QMessageBox::critical(nullptr, i18n("Breakpoint not found"), i18n("Couldn't find breakpoint at %1:%2", document->url().toString(), mark.line));
        }
    } else if (!(type & MarkInterface::Bookmark)) // neither breakpoint nor bookmark
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
    bookmarkAction->setChecked((type & MarkInterface::Bookmark));

    QAction* triggeredAction = menu.exec(pos);
    if (triggeredAction) {
        if (triggeredAction == bookmarkAction) {
            KTextEditor::MarkInterface *iface = qobject_cast<KTextEditor::MarkInterface*>(document);
            if ((type & MarkInterface::Bookmark))
                iface->removeMark(mark.line, MarkInterface::Bookmark);
            else
                iface->addMark(mark.line, MarkInterface::Bookmark);
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

    if (!errorText.isEmpty()) {
        emit error(row, errorText);
    }
}

void BreakpointModel::notifyHit(int row)
{
    emit hit(row);
}

void BreakpointModel::markChanged(
    KTextEditor::Document *document,
    KTextEditor::Mark mark,
    KTextEditor::MarkInterface::MarkChangeAction action)
{
    Q_D(const BreakpointModel);


    if (action == KTextEditor::MarkInterface::MarkAdded)
        qCritical() << "markChanged(MarkAdded):"<<mark.line;
    else {
        Q_ASSERT(action == KTextEditor::MarkInterface::MarkRemoved);
        qCritical() << "markChanged(MarkRemoved)" << mark.line;
    }

    int type = mark.type;
    /* Is this a breakpoint mark, to begin with? */
    if (!(type & AllBreakpointMarks)) return;

    if (d->inhibitMarkChange)
        return;

    if (action == KTextEditor::MarkInterface::MarkAdded) {
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
    Breakpoint *b = breakpoint(url, cursor.line());
    if (b) {
        removeBreakpoint(b);
    } else {
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

ScopedIncrementor BreakpointModel::markChangeGuard()
{
    Q_D(BreakpointModel);

    return ScopedIncrementor(d->inhibitMarkChange);
}

void BreakpointModel::documentSaved(KDevelop::IDocument* doc)
{
    Q_D(BreakpointModel);

    qCritical() << "documentSaved()";

    IF_DEBUG( qCDebug(DEBUGGER); )
    // save breakpoints in the given document.
    for (Breakpoint* breakpoint : qAsConst(d->breakpoints)) {
        if (breakpoint->movingCursor()) {
            if (breakpoint->movingCursor()->document() != doc->textDocument()) continue;
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
    Q_D(BreakpointModel);

    qCritical() << "aboutToDeleteMovingInterfaceContent()";

    for (Breakpoint* breakpoint : qAsConst(d->breakpoints)) {
        if (breakpoint->movingCursor() && breakpoint->movingCursor()->document() == document) {
            breakpoint->stopDocumentLineTracking();
        }
    }
}

void BreakpointModel::load()
{
    KConfigGroup breakpoints = ICore::self()->activeSession()->config()->group("Breakpoints");
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

    KConfigGroup breakpoints = activeSession->config()->group("Breakpoints");
    breakpoints.writeEntry("number", d->breakpoints.count());
    int i = 0;
    for (Breakpoint* b : qAsConst(d->breakpoints)) {
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

    auto* const movingInterface = qobject_cast<KTextEditor::MovingInterface*>(document);
    if (!movingInterface) {
        breakpoint->stopDocumentLineTracking();
        return;
    }

    // can't use new signal/slot syntax here, MovingInterface is not a QObject
    connect(document, SIGNAL(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)), this,
            SLOT(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)), Qt::UniqueConnection);

    breakpoint->restartDocumentLineTrackingAt(movingInterface->newMovingCursor(KTextEditor::Cursor(line, 0)));
}

#include "moc_breakpointmodel.cpp"
