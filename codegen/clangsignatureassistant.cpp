/*
 * Copyright 2014 David Stevens <dgedstevens@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "clangsignatureassistant.h"

#include <KTextEditor/Document>
#include <KTextEditor/Cursor>
#include <language/duchain/duchainutils.h>
#include <language/duchain/topducontext.h>
#include <serialization/indexedstring.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/codegen/documentchangeset.h>
#include <interfaces/ibuddydocumentfinder.h>

#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icore.h>

#include "../duchain/parsesession.h"
#include "../duchain/cursorkindtraits.h"

#include "../util/clangdebug.h"
#include "../util/clangutils.h"
#include "../util/clangtypes.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QUrl>

using namespace KDevelop;

namespace {

struct DeclSearchInfo
{
    CXCursor defn;
    CXCursor decl;
};

ParseSessionData::Ptr getSession(const QUrl &url)
{
    DUChainReadLocker lock;
    auto top = DUChainUtils::standardContextForUrl(url);
    if (!top) {
        qCWarning(KDEV_CLANG) << "No context found for" << url;
        return {};
    }
    auto session = ParseSessionData::Ptr(dynamic_cast<ParseSessionData*>(top->ast().data()));
    if (!session) {
        qCWarning(KDEV_CLANG) << "No parse session / AST attached to context for url" << url;
        return {};
    }
    return session;
}

CXChildVisitResult findDecl(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
    DeclSearchInfo *info = static_cast<DeclSearchInfo*>(data);
    //TODO this takes the first declaration, so it doesn't quite work if there
    //are multiple declarations of the same function.
    if (!clang_equalCursors(cursor, info->defn) &&
       clang_equalCursors(clang_getCursorDefinition(cursor), info->defn)) {
        info->decl = cursor;
        return CXChildVisit_Break;
    }
    return CXChildVisit_Continue;
}

CXCursor getFunctionCursor(const KTextEditor::Cursor &sc, const CXTranslationUnit &unit, const CXFile &file)
{
    CXCursor cursor = ClangUtils::getCXCursor(sc.line(), sc.column(), unit, file);
    if (clang_Cursor_isNull(cursor)) {
        return clang_getNullCursor();
    }
    if (clang_getCursorKind(cursor) == CXCursor_ParmDecl) {
        cursor = clang_getCursorSemanticParent(cursor);
    }
    if (CursorKindTraits::isFunction(clang_getCursorKind(cursor))) {
        return cursor;
    } else {
        return clang_getNullCursor();
    }
}

CXChildVisitResult findLastParam(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
    KTextEditor::Cursor *cur = static_cast<KTextEditor::Cursor*>(data);
    if (clang_getCursorKind(cursor) == CXCursor_ParmDecl) {
        ClangRange range(clang_getCursorExtent(cursor));
        KTextEditor::Cursor end = (KTextEditor::Cursor)range.end();
        if (*cur < end) {
            *cur = end;
        }
    }
    return CXChildVisit_Continue;
}

//Unfortunately, a function definition's cursor extent includes the function
//body. We only want the range of the signature
KTextEditor::Cursor findSignatureEnd(KTextEditor::Document *targetDoc, CXCursor cursor)
{
    KTextEditor::Range range = ClangRange(clang_getCursorExtent(cursor)).toRange();
    KTextEditor::Cursor search = range.start();
    //We need to start at the end of the last paramater, so we don't have
    //to worry about parens in default arguments
    clang_visitChildren(cursor, findLastParam, &search);

    int endLine = search.line(), endColumn = search.column();
    int rangeEnd = range.end().line();
    QChar endChar(')'), searchChar;

    do {
        searchChar = targetDoc->characterAt(KTextEditor::Cursor(endLine, endColumn));
        while (searchChar != endChar && (endLine <= rangeEnd)) {
            endColumn++;
            while ((searchChar = targetDoc->characterAt(KTextEditor::Cursor(endLine, endColumn))) == QChar() && (endLine <= rangeEnd)) {
                endLine++;
                endColumn = 0;
            }
        }
        if (searchChar != endChar) {
            clangDebug() << "Could not find ending character of declaration";
            return KTextEditor::Cursor::invalid();
        }

        if (endChar == QChar(')') && ClangUtils::isConstMethod(cursor)) {
            endChar = QChar('t');
        }
    } while (searchChar != endChar);

    return KTextEditor::Cursor(endLine, endColumn+1);
}

QUrl findCompanionFile(const QUrl &fileUrl, const KTextEditor::Cursor& sc, const CXFile& file, CXCursor& otherSide)
{
    static QStringList headerMime({"text/x-c++hdr", "text/x-chdr"});
    static QStringList srcMime({"text/x-c++src", "text/x-csrc"});

    QMimeDatabase db;
    QString me = db.mimeTypeForUrl(fileUrl).name();
    QStringList targetTypes;
    if (headerMime.contains(me)) {
        targetTypes = srcMime;
    } else if (srcMime.contains(me)) {
        targetTypes = headerMime;
    } else {
        clangDebug() << "Unrecgonized file extension";
        return QUrl();
    }

    IBuddyDocumentFinder* buddyFinder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(fileUrl).name());
    if (!buddyFinder) {
        clangDebug() << "Could not create buddy finder for " << fileUrl;
        return QUrl();
    }
    foreach (QUrl potentialUrl, buddyFinder->getPotentialBuddies(fileUrl)) {
        QString potentialMime = db.mimeTypeForUrl(potentialUrl).name();
        if (!QFile::exists(potentialUrl.toLocalFile()) || !targetTypes.contains(potentialMime)) {
            continue;
        }

        const ParseSession altSession(getSession(potentialUrl));
            if (!altSession.data()) {
            continue;
        }

        //TODO name collisions? Comparing the USR doesn't work because of including
        //Unfortunately, CXFiles of the same name aren't the same across translation units
        CXFile altFile = clang_getFile(altSession.unit(), ClangString(clang_getFileName(file)).c_str());
        CXCursor altCursor = getFunctionCursor(sc, altSession.unit(), altFile);
        if (clang_Cursor_isNull(altCursor)) {
            continue;
        }
        otherSide = clang_getCursorDefinition(altCursor);
        if (clang_Cursor_isNull(otherSide)) {
            continue;
        }

        return potentialUrl;
    }
    return QUrl();
}

bool fixDefaults(QVector<QString>& defaults, const CXCursor& cursor, const QList<ParamInfo>& oldInfo)
{
    bool hasDefaults = false;
    foreach (QString str, defaults) {
        if (!str.isEmpty()) {
            hasDefaults = true;
            break;
        }
    }

    if (hasDefaults) {
        //A vector for the new position of each of the old parameters
        QVector<int> positions(oldInfo.count(), -1);

        for (int curNewParam = clang_Cursor_getNumArguments(cursor) - 1; curNewParam >= 0; --curNewParam) {
            int foundAt = -1;

            for (int curOldParam = oldInfo.count() - 1; curOldParam >= 0; --curOldParam) {
                CXCursor curParam = clang_Cursor_getArgument(cursor, curNewParam);
                if (ClangString(clang_getTypeSpelling(clang_getCursorType(curParam))).toString() != oldInfo[curOldParam].type) {
                    continue; //Different types, different parameters
                }
                QString name = ClangString(clang_getCursorSpelling(curParam)).toString();
                if (name == oldInfo[curOldParam].name || curOldParam == curNewParam) {
                    //Same type and either same position or same name, probably same parameters
                    foundAt = curOldParam;
                    if (name == oldInfo[curOldParam].name) {
                        break; //same name and type, almost certainly same parameters
                    }
                    //else position/type match, but allow name match in the future to trump
                }
            }

            if (foundAt != -1) {
                //If we map the same old parameter to two new ones, whoops!
                if (positions[foundAt] != -1) {
                    return false;
                }
                positions[foundAt] = curNewParam;
            }
        }

        QVector<QString> newDefaults(clang_Cursor_getNumArguments(cursor));
        for (int i = 0; i < positions.count(); i++) {
            if (positions[i] != -1) {
                newDefaults[positions[i]] = defaults[i];
            }
        }
        defaults.swap(newDefaults);
    } else {
        defaults.clear();
        defaults.resize(clang_Cursor_getNumArguments(cursor));
    }

    return true;
}

}

ClangAdaptSignatureAction::ClangAdaptSignatureAction(bool targetDecl, const QUrl &url,
                                                     const KTextEditor::Range& range,
                                                     const QString& newSig, const QString& oldSig):
    m_targetDecl(targetDecl), m_url(url), m_range(range), m_newSig(newSig), m_oldSig(oldSig)
{
}

QString ClangAdaptSignatureAction::description() const
{
    return m_targetDecl ? i18n("Update declaration signature") : i18n("Update definition signature");
}

void ClangAdaptSignatureAction::execute()
{
    DUChainReadLocker lock;
    DocumentChange changeParameters(DUChainUtils::standardContextForUrl(m_url)->url(), m_range, QString(), m_newSig);
    changeParameters.m_ignoreOldText = true;
    DocumentChangeSet changes;
    changes.addChange(changeParameters);

    changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
    auto result = changes.applyAllChanges();
    if (!result) {
        KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
    }

    emit executed(this);
}

QString ClangAdaptSignatureAction::toolTip() const
{
    return i18n("Update %1\nfrom: %2\nto: %3",
            m_targetDecl ? i18n("declaration") : i18n("definition"),
            m_newSig, m_oldSig);
}

ClangSignatureAssistant::ClangSignatureAssistant(ILanguageSupport* languageSupport)
    : StaticAssistant(languageSupport)
{
    connect(ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)),
            this, SLOT(parseJobFinished(KDevelop::ParseJob*)));
}

ClangSignatureAssistant::~ClangSignatureAssistant()
{
}

void ClangSignatureAssistant::reset()
{
    m_oldSig.clear();
    m_oldName.clear();
    m_targetUnit.clear();
    m_oldParamInfo.clear();
    hide();
}


void ClangSignatureAssistant::textChanged(KTextEditor::View* view, const KTextEditor::Range& invocationRange,
                                          const QString& removedText)
{
    Q_UNUSED(removedText);

    reset();
    m_view = view;

    QUrl fileUrl = m_view.data()->document()->url();
    const ParseSession session(getSession(fileUrl));
    if (!session.data()) {
        return;
    }

    const KTextEditor::Cursor simpleCursor(invocationRange.start());
    CXCursor cursor = getFunctionCursor(simpleCursor, session.unit(), session.file());
    if (clang_Cursor_isNull(cursor)) {
        return;
    }

    CXCursor otherSide = clang_getCursorDefinition(cursor);

    if (clang_equalCursors(otherSide, cursor)) {
        //If we are our definition, we need to try to find our declaration. Do
        //this by searching through the children of our semantic parent. This won't
        //handle the case where our declaration isn't in our translation unit, but
        //that is a very unusual case, and it's not clear how to handle it.
        CXCursor par = clang_getCursorSemanticParent(cursor);

        if (clang_Cursor_isNull(par)) {
            return;
        }

        DeclSearchInfo info{cursor, clang_getNullCursor()};
        clang_visitChildren(par, findDecl, &info);

        if (clang_Cursor_isNull(info.decl)) {
            return;
        }
        otherSide = info.decl;

        if (!clang_Location_isFromMainFile(clang_getCursorLocation(otherSide))) {
            CXFile otherFile;
            clang_getFileLocation(clang_getCursorLocation(otherSide), &otherFile, nullptr, nullptr, nullptr);
            if (!otherFile) {
                clangDebug() << "Could not find file corresponding to other side of definition";
                return;
            }

            m_targetUnit = QUrl::fromLocalFile(ClangString(clang_getFileName(otherFile)).toString());
            if (m_targetUnit.isEmpty()) {
                clangDebug() << "Could not access file " << clang_getFileName(otherFile);
                return;
            }
        }

        m_targetDecl = true;
    } else if (!clang_Cursor_isNull(otherSide)) {
        //If otherside is not null or us, then it's our definition
        m_targetDecl = false;
    } else {
        //We are a declaration without any definition. This is the tricky case, because
        //the function could be defined in any file which includes us, but that
        //information is not available to use through clang's translation unit model. The
        //best we can do is guess at similar file names.
        m_targetUnit = findCompanionFile(fileUrl, simpleCursor, session.file(), otherSide);

        if (m_targetUnit.isEmpty()) {
            clangDebug() << "Could not find candidate target for " << fileUrl;
            return;
        }
        m_targetDecl = false;
    }

    m_otherLoc = ClangLocation(clang_getCursorLocation(otherSide));
    if (!m_otherLoc.isValid()) {
        return;
    }

    //When we use this, the translation unit will be different, so we have to
    //turn them into strings.
    m_oldParamInfo.clear();
    for (int i = 0; i < clang_Cursor_getNumArguments(cursor); i++) {
        CXCursor param = clang_Cursor_getArgument(cursor, i);
        m_oldParamInfo.append(ParamInfo{ClangString(clang_getCursorSpelling(param)).toString(),
                                        ClangString(clang_getTypeSpelling(clang_getCursorType(param))).toString()});
    }

    m_oldName = ClangString(clang_getCursorSpelling(cursor)).toString();
    QString scope = ClangUtils::getScope(otherSide);
    m_oldSig = ClangUtils::getCursorSignature(otherSide, scope);
}

QString ClangSignatureAssistant::title() const
{
    return tr("Adapt Signature");
}

bool ClangSignatureAssistant::isUseful() const
{
    return !m_oldSig.isEmpty() && !m_oldName.isEmpty();
}

void ClangSignatureAssistant::parseJobFinished(ParseJob* job)
{
    if (!isUseful() || !m_view || job->document().toUrl() != m_view.data()->document()->url()) {
        //If we're here, we're not the active assistant, so there's no reason to reset
        return;
    }
    clearActions();

    DUChainReadLocker lock;
    KTextEditor::Cursor c = KTextEditor::Cursor(m_view.data()->cursorPosition());

    const ParseSession sourceSession(getSession(m_view.data()->document()->url()));
    if (!sourceSession.data()) {
        reset();
        return;
    }
    ParseSession targetSession({});
    CXTranslationUnit targetUnit = nullptr;
    if (!m_targetUnit.isEmpty()) {
        targetSession.setData(getSession(m_targetUnit));
        if (!targetSession.data()) {
            reset();
            return;
        }
        targetUnit = targetSession.unit();
    } else {
        targetUnit = sourceSession.unit();
    }

    CXFile otherFile = clang_getFile(targetUnit, m_otherLoc.document.byteArray().constData());
    CXCursor cursor = getFunctionCursor(c, sourceSession.unit(), sourceSession.file());
    CXCursor otherCursor = getFunctionCursor(m_otherLoc, targetUnit, otherFile);
    if (clang_Cursor_isNull(cursor)) {
        clangDebug() << "Couldn't get source cursor " << clang_getFileName(sourceSession.file()) << ":" << c;
        reset();
        return;
    }

    if (clang_Cursor_isNull(otherCursor)) {
        clangDebug() << "Couldn't get target cursor " << clang_getFileName(otherFile) << ":" << m_otherLoc;
        reset();
        return;
    }

    auto defaults = ClangUtils::getDefaultArguments(otherCursor);

    if (!fixDefaults(defaults, cursor, m_oldParamInfo)) {
        reset();
        return;
    }

    QString scope = ClangUtils::getScope(otherCursor);
    QString newSig = ClangUtils::getCursorSignature(cursor, scope, defaults);

    if (newSig == m_oldSig || m_oldName != ClangString(clang_getCursorSpelling(cursor)).toString()) {
        reset();
        return;
    }

    QUrl targetUrl = m_otherLoc.document.toUrl();
    KTextEditor::Document *targetDoc = ICore::self()->documentController()->documentForUrl(targetUrl)->textDocument();

    KTextEditor::Cursor end = findSignatureEnd(targetDoc, otherCursor);
    if (!end.isValid()) {
        reset();
        return;
    }

    KTextEditor::Range range = ClangRange(clang_getCursorExtent(otherCursor)).toRange();
    range.setEnd(end);

    IAssistantAction::Ptr action(new ClangAdaptSignatureAction(m_targetDecl, targetUrl, range, newSig, m_oldSig));
    connect(action.data(), SIGNAL(executed(IAssistantAction*)), SLOT(reset()));
    addAction(action);
    emit actionsChanged();
}

#include "moc_clangsignatureassistant.cpp"
