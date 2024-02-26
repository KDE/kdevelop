/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2006, 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "breakpoint.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <QIcon>

#include <KLocalizedString>
#include <KConfigGroup>
#include <KTextEditor/Document>
#include <KTextEditor/MarkInterface>
#include <KTextEditor/MovingCursor>
#include <KTextEditor/MovingInterface>

#include "breakpointmodel.h"

#include <array>

namespace {
bool isSupportedBreakpointUrl(const QUrl& url)
{
    return url.isEmpty() || (!url.isRelative() && !url.fileName().isEmpty());
}
} // unnamed namespace

using namespace KDevelop;

static const std::array<QString, Breakpoint::LastBreakpointKind> BREAKPOINT_KINDS = {
    QStringLiteral("Code"),
    QStringLiteral("Write"),
    QStringLiteral("Read"),
    QStringLiteral("Access"),
};

static Breakpoint::BreakpointKind stringToKind(const QString& kindString)
{
    for (int i = 0; i < Breakpoint::LastBreakpointKind; ++i) {
        if (kindString == BREAKPOINT_KINDS[i]) {
            return (Breakpoint::BreakpointKind)i;
        }
    }
    return Breakpoint::CodeBreakpoint;
}

Breakpoint::Breakpoint(BreakpointModel *model, BreakpointKind kind)
    : m_model(model), m_enabled(true)
    , m_deleted(false)
    , m_state(NotStartedState)
    , m_kind(kind)
    , m_line(-1)
    , m_movingCursor(nullptr)
    , m_hitCount(0)
    , m_ignoreHits(0)
{
    if (model) {
        model->registerBreakpoint(this);
    }
}

Breakpoint::Breakpoint(BreakpointModel *model, const KConfigGroup& config)
    : m_model(model), m_enabled(true)
    , m_deleted(false)
    , m_state(NotStartedState)
    , m_line(-1)
    , m_movingCursor(nullptr)
    , m_hitCount(0)
    , m_ignoreHits(0)
{
    m_kind = stringToKind(config.readEntry("kind", ""));
    m_enabled = config.readEntry("enabled", false);
    m_url = config.readEntry("url", QUrl());
    m_line = config.readEntry("line", -1);
    m_expression = config.readEntry("expression", QString());
    m_condition = config.readEntry("condition", "");
    m_ignoreHits = config.readEntry("ignoreHits", 0);

    if (model) {
        model->registerBreakpoint(this);
    }
}

BreakpointModel *Breakpoint::breakpointModel()
{
    return m_model;
}

bool Breakpoint::setData(int index, const QVariant& value)
{
    if (index == EnableColumn)
    {
        m_enabled = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;

        // enabled affects the breakpoint mark type.
        updateMarkType();
    }

    if (index == LocationColumn) {
        QString s = value.toString();
        QRegExp rx(QStringLiteral("^(.+):([0-9]+)$"));
        int idx = rx.indexIn(s);
        if (m_kind == CodeBreakpoint && idx != -1) {
            const auto url = QUrl::fromLocalFile(rx.cap(1));
            const auto line = rx.cap(2).toInt() - 1;

            if (isSupportedBreakpointUrl(url)) {
                m_expression.clear();
                setLocation(url, line);
                return true; // reportChange() is already called by setLocation()
            }
        }
        m_expression = s;
        stopDocumentLineTracking();
        m_url.clear();
        m_line = -1;
    }

    if (index == ConditionColumn) {
        m_condition = value.toString();
    }

    reportChange(static_cast<Column>(index));

    return true;
}

QVariant Breakpoint::data(int column, int role) const
{
    if (column == EnableColumn)
    {
        if (role == Qt::CheckStateRole)
            return m_enabled ? Qt::Checked : Qt::Unchecked;
        else if (role == Qt::DisplayRole)
            return QVariant();
        else
            return QVariant();
    }

    if (column == StateColumn)
    {
        if (role == Qt::DecorationRole) {
            if (!errorText().isEmpty()) {
                return QIcon::fromTheme(QStringLiteral("dialog-warning"));
            }
            switch (state()) {
                case NotStartedState:
                    return QVariant();
                case DirtyState:
                    return QIcon::fromTheme(QStringLiteral("system-switch-user"));
                case PendingState:
                    return QIcon::fromTheme(QStringLiteral("help-contents"));
                case CleanState:
                    return QIcon::fromTheme(QStringLiteral("dialog-ok-apply"));
            }
        } else if (role == Qt::ToolTipRole) {
            if (!errorText().isEmpty()) {
                return i18nc("@info:tooltip", "Error");
            }
            switch (state()) {
                case NotStartedState:
                    return QString();
                case DirtyState:
                    return i18nc("@info:tooltip", "Dirty");
                case PendingState:
                    return i18nc("@info:tooltip", "Pending");
                case CleanState:
                    return i18nc("@info:tooltip", "Clean");
            }
        } else if (role == Qt::DisplayRole) {
            return QVariant();
        }
        return QVariant();
    }

    if (column == TypeColumn && role == Qt::DisplayRole)
    {
        return BREAKPOINT_KINDS[m_kind];
    }

    if (column == ConditionColumn && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        return m_condition;
    }

    if (column == LocationColumn) {
        if (role == LocationRole || role == Qt::EditRole || role == Qt::ToolTipRole || role == Qt::DisplayRole) {
            QString ret;
            const auto currentLine = line();
            if (m_kind == CodeBreakpoint && currentLine != -1) {
                    if (role == Qt::DisplayRole) {
                        ret = m_url.fileName();
                    } else {
                        ret = m_url.toDisplayString(QUrl::PreferLocalFile | QUrl::StripTrailingSlash);
                    }
                    ret += QLatin1Char(':') + QString::number(currentLine + 1);
            } else {
                    ret = m_expression;
            }
            //FIXME: there should be proper columns for function name and address.
            if (!m_address.isEmpty() && role == Qt::DisplayRole) {
                ret = i18nc("location (address)", "%1 (%2)", ret, m_address);
            }
            return ret;
        }
    }

    return QVariant();
}

int Breakpoint::line() const {
    return m_movingCursor ? m_movingCursor->line() : m_line;
}

int Breakpoint::savedLine() const
{
    return m_line;
}

void Breakpoint::setLine(int line) {
    setLocation(m_url, line);
}
void Breakpoint::setUrl(const QUrl& url) {
    setLocation(url, line());
}
QUrl Breakpoint::url() const {
    return m_url;
}
void Breakpoint::setLocation(const QUrl& url, int line)
{
    Q_ASSERT(m_kind == CodeBreakpoint);
    Q_ASSERT(isSupportedBreakpointUrl(url));

    updateMovingCursor(url, line);

    m_url = url;
    m_line = line;
    reportChange(LocationColumn);
}

QString KDevelop::Breakpoint::location() {
    return data(LocationColumn, LocationRole).toString();
}


void Breakpoint::save(KConfigGroup& config)
{
    config.writeEntry("kind", BREAKPOINT_KINDS[m_kind]);
    config.writeEntry("enabled", m_enabled);
    config.writeEntry("url", m_url);
    config.writeEntry("line", m_line);
    config.writeEntry("expression", m_expression);
    config.writeEntry("condition", m_condition);
    config.writeEntry("ignoreHits", m_ignoreHits);
}

Breakpoint::BreakpointKind Breakpoint::kind() const
{
    return m_kind;
}

void Breakpoint::setAddress(const QString& address)
{
    m_address = address;
    //reportChange();
}

QString Breakpoint::address() const
{
    return m_address;
}

void Breakpoint::setHitCount(int hits)
{
    if (m_hitCount == hits)
        return;

    m_hitCount = hits;

    // hit count affects the breakpoint mark type.
    updateMarkType();

    reportChange(HitCountColumn);
}

int Breakpoint::hitCount() const
{
    return m_hitCount;
}

bool Breakpoint::deleted() const
{
    return m_deleted;
}

bool Breakpoint::enabled() const
{
    return m_enabled;
}

void Breakpoint::stopDocumentLineTracking()
{
    if (!m_movingCursor)
        return;

    // Remove the associated breakpoint mark.
    auto* const imark = qobject_cast<KTextEditor::MarkInterface*>(m_movingCursor->document());
    if (imark) {
        const auto guard = m_model->markChangeGuard();
        imark->removeMark(m_movingCursor->line(), BreakpointModel::AllBreakpointMarks);
    }

    delete m_movingCursor;
    m_movingCursor = nullptr;
}

void Breakpoint::restartDocumentLineTrackingAt(KTextEditor::MovingCursor* cursor)
{
    Q_ASSERT(cursor);
    Q_ASSERT(cursor != m_movingCursor);

    stopDocumentLineTracking();

    m_movingCursor = cursor;

    // Add a breakpoint mark at the new cursor's location.
    auto* const imark = qobject_cast<KTextEditor::MarkInterface*>(m_movingCursor->document());
    if (imark) {
        const auto guard = m_model->markChangeGuard();
        qCritical()<< "restartDocumentLineTrackingAt addMark():" << m_movingCursor->line();
        imark->addMark(m_movingCursor->line(), markType());
    }
}

void KDevelop::Breakpoint::pauseDocumentLineTracking()
{
    Q_ASSERT(m_movingCursor);
    auto* const imark = qobject_cast<KTextEditor::MarkInterface*>(m_movingCursor->document());
    Q_ASSERT(imark);
    Q_ASSERT((imark->mark(m_movingCursor->line()) & BreakpointModel::AllBreakpointMarks) == 0);
    Q_ASSERT((imark->mark(m_line) & BreakpointModel::AllBreakpointMarks) == 0);

    delete m_movingCursor;
    m_movingCursor = nullptr;
}

void KDevelop::Breakpoint::resumeDocumentLineTracking(KTextEditor::Document& document)
{
    Q_ASSERT(m_line >= 0);
    Q_ASSERT(m_line < document.lines());
    Q_ASSERT(!m_movingCursor);

    auto* const imark = qobject_cast<KTextEditor::MarkInterface*>(&document);
    Q_ASSERT(imark);
    Q_ASSERT((imark->mark(m_line) & BreakpointModel::AllBreakpointMarks) == markType());

    auto* const movingInterface = qobject_cast<KTextEditor::MovingInterface*>(&document);
    Q_ASSERT(movingInterface);
    m_movingCursor = movingInterface->newMovingCursor(KTextEditor::Cursor(m_line, 0));
}

KTextEditor::MovingCursor* KDevelop::Breakpoint::movingCursor() const {
    return m_movingCursor;
}

void Breakpoint::setIgnoreHits(int c)
{
    if (m_ignoreHits != c) {
        m_ignoreHits = c;
        reportChange(IgnoreHitsColumn);
    }
}

int Breakpoint::ignoreHits() const
{
    return m_ignoreHits;
}


void Breakpoint::setCondition(const QString& c)
{
    if (c != m_condition) {
        m_condition = c;
        reportChange(ConditionColumn);
    }
}

QString Breakpoint::condition() const
{
    return m_condition;
}

void Breakpoint::setExpression(const QString& e)
{
    if (e != m_expression) {
        m_expression = e;
        reportChange(LocationColumn);
    }
}

QString Breakpoint::expression() const
{
    return m_expression;
}

void Breakpoint::setState(BreakpointState state)
{
    if (m_state == state)
        return;

    m_state = state;

    // BreakpointState affects the breakpoint mark type.
    updateMarkType();

    reportChange(StateColumn);
}

Breakpoint::BreakpointState Breakpoint::state() const
{
    return m_state;
}

QString Breakpoint::errorText() const
{
    return m_errorText;
}

uint Breakpoint::markType() const
{
    if (!m_enabled) {
        return BreakpointModel::DisabledBreakpointMark;
    } else if (m_hitCount > 0) {
        return BreakpointModel::ReachedBreakpointMark;
    } else if (m_state == PendingState) {
        return BreakpointModel::PendingBreakpointMark;
    } else {
        return BreakpointModel::BreakpointMark;
    }
}

void Breakpoint::updateMarkType() const
{
    // Do we even have a mark?

    if (!m_model)
        return;

    if (!m_movingCursor)
        return;

    auto* const imark = qobject_cast<KTextEditor::MarkInterface*>(m_movingCursor->document());
    if (!imark)
        return;

    // Yes, but don't update if type would not change:
    const auto newMarkType = markType();
    Q_ASSERT(newMarkType);
    const auto docLine = m_movingCursor->line();
    const auto oldMarkType = imark->mark(docLine) & BreakpointModel::AllBreakpointMarks;

    if (oldMarkType == newMarkType) {
        return;
    }

    const auto guard = m_model->markChangeGuard();
    imark->removeMark(docLine, BreakpointModel::AllBreakpointMarks);
    imark->addMark(docLine, newMarkType);
}

void Breakpoint::saveMovingCursorLine()
{
    Q_ASSERT(m_movingCursor);
    m_line = m_movingCursor->line();
}

void KDevelop::Breakpoint::reportChange(Column c)
{
    if (!breakpointModel())
        return;

    breakpointModel()->reportChange(this, c);
}

void Breakpoint::updateMovingCursor(const QUrl& url, int line)
{
    // Can a moving cursor even be enabled?
    if (!m_model || line < 0 || url.isEmpty()) {
        stopDocumentLineTracking();
        return;
    }

    if (m_movingCursor) {
        // Cursor is attached already, is it possible to retain it?
        const auto* const document = m_movingCursor->document();
        if (document && document->url() == url) {
            if (line >= document->lines()) {
                stopDocumentLineTracking();
            } else if (m_movingCursor->line() != line) {
                auto* const imark = qobject_cast<KTextEditor::MarkInterface*>(document);
                if (imark) {
                    const auto guard = m_model->markChangeGuard();
                    imark->removeMark(m_movingCursor->line(), BreakpointModel::AllBreakpointMarks);
                    imark->addMark(line, markType());
                }
                m_movingCursor->setLine(line);
            }
            return;
        }
    }

    // Find the document:
    const auto* const documentController = ICore::self()->documentController();
    const auto* const document = documentController ? documentController->documentForUrl(url) : nullptr;
    if (document) {
        // Either document changed or the breakpoint has no moving cursor yet.
        const auto textDocument = document->textDocument();
        if (textDocument && line < textDocument->lines()) {
            m_model->setupMovingCursor(this, textDocument, line);
            return;
        }
    }

    // No document was found, or the location is after the last line of the new document.
    stopDocumentLineTracking();
}
