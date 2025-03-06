/*
 *    This file is part of KDevelop, made within the KDE community.
 *
 *    SPDX-FileCopyrightText: 2025 Morten Danielsen Volden <mvolden2@gmail.com>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "bookmarkmodel.h"
#include "debug.h"

#include "interfaces/icore.h"
#include "interfaces/idocumentcontroller.h"
#include "interfaces/idocument.h"

#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KBookmark>

#include <QFontDatabase>
#include <QIcon>
#include <QPixmap>

namespace KDevelop {

BookmarkModel::BookmarkModel(QObject* parent)
    : QAbstractTableModel(parent)
    , mFixedFont(QFontDatabase::systemFont(QFontDatabase::FixedFont))
{
    auto* const documentController = ICore::self()->documentController();
    Q_ASSERT(documentController); // BreakpointModel is created after DocumentController.

    // This constructor is invoked before controllers are initialized, and thus before any documents can be opened.
    // So our textDocumentCreated() slot will be invoked for all documents.
    Q_ASSERT(documentController->openDocuments().empty());

    /// Make our model handle bookmarks added from the IconBorder of each document
    connect(documentController, &IDocumentController::textDocumentCreated, this, &BookmarkModel::textDocumentCreated);

    //         mPrintFunction = Okteta::OffsetFormat::printFunction((Okteta::OffsetFormat::Format)tool->offsetCoding());
    //
    //         connect(mTool, &BookmarksTool::hasBookmarksChanged,
    //                 this, &BookmarkListModel::onHasBookmarksChanged);
    //         connect(mTool, &BookmarksTool::bookmarksAdded,
    //                 this, qOverload<>(&BookmarkListModel::onBookmarksChanged));
    //         connect(mTool, &BookmarksTool::bookmarksRemoved,
    //                 this, qOverload<>(&BookmarkListModel::onBookmarksChanged));
    //         connect(mTool, &BookmarksTool::bookmarksModified,
    //                 this, qOverload<const QList<int>&>(&BookmarkListModel::onBookmarksChanged));
    //         connect(mTool, &BookmarksTool::offsetCodingChanged,
    //                 this, &BookmarkListModel::onOffsetCodingChanged);
}

BookmarkModel::~BookmarkModel() = default;

void BookmarkModel::textDocumentCreated(KDevelop::IDocument* doc)
{
    KTextEditor::Document* const textDocument = doc->textDocument();
    Q_ASSERT(textDocument);

    textDocument->setMarkDescription(KTextEditor::Document::MarkTypes::Bookmark, i18n("bookmark"));
    textDocument->setMarkIcon(KTextEditor::Document::MarkTypes::Bookmark, bookmarkPixmap());
    textDocument->setEditableMarks(KTextEditor::Document::MarkTypes::Bookmark
                                   | KTextEditor::Document::MarkTypes::BreakpointActive);

    // TODO! Call setup to bookmarks before connecting the signals

    connect(textDocument, &KTextEditor::Document::markChanged, this, &BookmarkModel::markChanged);
    // connect(textDocument, &KTextEditor::Document::markContextMenuRequested, this,
    //         &BreakpointModel::markContextMenuRequested);
    //
    // connect(textDocument, &KTextEditor::Document::aboutToReload, this, &BreakpointModel::aboutToReload);
    // connect(textDocument, &KTextEditor::Document::aboutToInvalidateMovingInterfaceContent, this,
    //         &BreakpointModel::aboutToInvalidateMovingInterfaceContent);
    // connect(textDocument, &KTextEditor::Document::reloaded, this, &BreakpointModel::reloaded);

#if KTEXTEDITOR_VERSION < QT_VERSION_CHECK(6, 9, 0)
// Since https://commits.kde.org/ktexteditor/6ca19934786fb808ab2b307d558967a74f87e4f4
// first included in KTextEditor version 6.9, KTextEditor::Document emits the signal
// aboutToInvalidateMovingInterfaceContent() instead of aboutToDeleteMovingInterfaceContent()
// from the destructor.
// connect(textDocument, &KTextEditor::Document::aboutToDeleteMovingInterfaceContent, this,
//         &BreakpointModel::aboutToInvalidateMovingInterfaceContent);
#endif
}

void BookmarkModel::markChanged(KTextEditor::Document* document, KTextEditor::Mark mark,
                                KTextEditor::Document::MarkChangeAction action)
{
    int type = mark.type;
    /* Is this a breakpoint mark, to begin with? */
    if (!(type & KTextEditor::Document::MarkTypes::Bookmark)) {
        return;
    }
    if (action == KTextEditor::Document::MarkAdded) {
        qCDebug(PLUGIN_FILEMANAGER) << "Got an added Bookmark in:" << document->url() << " , line: " << mark.line;
    } else {
        qCDebug(PLUGIN_FILEMANAGER) << "Got an Bookmark removed in:" << document->url() << " , line: " << mark.line;
    }
}

static constexpr int pixmapSize = 32;

const QPixmap KDevelop::BookmarkModel::bookmarkPixmap()
{
    static QPixmap pixmap =
        QIcon::fromTheme(QStringLiteral("breakpoint")).pixmap(QSize(pixmapSize, pixmapSize), QIcon::Active, QIcon::Off);
    return pixmap;
}

int BookmarkModel::rowCount(const QModelIndex& parent) const
{
    return (!parent.isValid()) ? /*mTool->bookmarksCount()*/ 0 : 0;
}

int BookmarkModel::columnCount(const QModelIndex& parent) const
{
    return (!parent.isValid()) ? NumColumns : 0;
}

QVariant BookmarkModel::data(const QModelIndex& index, int role) const
{
    QVariant result;

    switch (role) {
    case Qt::DisplayRole: {
        [[maybe_unused]] const int bookmarkIndex = index.row();

        //const Okteta::Bookmark& bookmark = mTool->bookmarkAt(bookmarkIndex);
        //KBookmark bookmark;

        const int tableColumn = index.column();
        switch (tableColumn) {
        case LocationColumnId: {
            //mPrintFunction(mCodedOffset, bookmark.offset());

            result = QString::fromLatin1("offset" /*mCodedOffset*/); // FIXME
            break;
        }
        case TitleColumnId:
            result = QString::fromLatin1("bookmark.name()"); // FIXME
            break;
        default:;
        }
        break;
    }
    case Qt::FontRole: {
        const int column = index.column();
        if (column == LocationColumnId) {
            result = mFixedFont;
        }
        break;
    }
    case Qt::EditRole: {
        [[maybe_unused]] const int bookmarkIndex = index.row();

        const int column = index.column();
        if (column == TitleColumnId) {
            //const Okteta::Bookmark& bookmark = mTool->bookmarkAt(bookmarkIndex);
            result = QString::fromLatin1("bookmark.name()"); // FIXME
        }
        break;
    }
    default:
        break;
    }

    return result;
}

Qt::ItemFlags BookmarkModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags result = QAbstractTableModel::flags(index);
    const int column = index.column();
    if (column == TitleColumnId) {
        result |= Qt::ItemIsEditable;
    }

    return result;
}

QVariant BookmarkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;

    if (role == Qt::DisplayRole) {
        const QString titel = section == LocationColumnId ? i18nc("@title:column location of the bookmark", "Location")
            : section == TitleColumnId                    ? i18nc("@title:column title of the bookmark", "Title")
                                                          : QString();
        result = titel;
    } else {
        result = QAbstractTableModel::headerData(section, orientation, role);
    }

    return result;
}

bool BookmarkModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool result;

    if (role == Qt::EditRole) {
        [[maybe_unused]] const int bookmarkIndex = index.row();

        const int column = index.column();
        if (column == TitleColumnId) {
            // mTool->setBookmarkName(bookmarkIndex, value.toString());
            //             Q_EMIT dataChanged( index, index );
            result = true;
        } else {
            result = false;
        }
    } else {
        result = QAbstractItemModel::setData(index, value, role);
    }

    return result;
}

const KBookmark& BookmarkModel::bookmark(const QModelIndex& index) const
{
    [[maybe_unused]] const int bookmarkIndex = index.row();
    const KBookmark* hello = new KBookmark();
    return *hello; //FIXME
    //return mTool->bookmarkAt(bookmarkIndex);
}

QModelIndex BookmarkModel::index([[maybe_unused]] const KBookmark& bookmark, int column) const
{
    QModelIndex result;

    const int indexOfBookmark = -1; //mTool->indexOf(bookmark);
    if (indexOfBookmark != -1) {
        result = createIndex(indexOfBookmark, column);
    }

    return result;
}

void BookmarkModel::onHasBookmarksChanged(bool hasBookmarks)
{
    Q_UNUSED(hasBookmarks)

    beginResetModel();
    endResetModel();
}

void BookmarkModel::onBookmarksChanged()
{
    beginResetModel();
    endResetModel();
}

void BookmarkModel::onBookmarksChanged(const QList<int>& bookmarkIndizes)
{
    for (int row : bookmarkIndizes) {
        Q_EMIT dataChanged(index(row, LocationColumnId), index(row, TitleColumnId));
    }
}

void BookmarkModel::onOffsetCodingChanged(int /*offsetCoding*/)
{
    //mPrintFunction = Okteta::OffsetFormat::printFunction((Okteta::OffsetFormat::Format)offsetCoding);
    beginResetModel();
    endResetModel();
}

}

#include "moc_bookmarkmodel.cpp"
