/*
 * Copyright 2018, 2019 Amish K. Naidu <amhndu@gmail.com>
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
#include "headerguardassistant.h"
#include "util/clangutils.h"
#include "util/clangtypes.h"

#include <language/codegen/documentchangeset.h>
#include <language/codegen/coderepresentation.h>
#include <language/duchain/duchainlock.h>

#include <QDir>
#include <QRegularExpression>

#include <algorithm>


enum class GuardType
{
    Pragma,
    Macro
};

class AddHeaderGuardAction
    : public KDevelop::IAssistantAction
{
    Q_OBJECT

public:
    AddHeaderGuardAction(const GuardType type, const int startLine, const KDevelop::IndexedString& path)
        : m_type(type)
        , m_startLine(startLine)
        , m_path(path)
    {
    }

    virtual ~AddHeaderGuardAction() override = default;

    QString description() const override
    {
        switch (m_type) {
            case GuardType::Pragma: return i18n("Add #pragma once");
            case GuardType::Macro: return i18n("Add macro-based #ifndef/#define/#endif header guard");
        }
        return {};
    }

    void execute() override
    {
        KDevelop::DocumentChangeSet changes;
        switch (m_type) {
            case GuardType::Pragma:
            {
                KDevelop::DocumentChange change(m_path, KTextEditor::Range(m_startLine, 0, m_startLine, 0), QString(),
                                                QStringLiteral("#pragma once\n\n"));
                changes.addChange(change);
                break;
            }
            case GuardType::Macro:
            {
                const QString macro = m_path.toUrl()
                        .fileName(QUrl::PrettyDecoded)
                        .replace(QRegularExpression(QStringLiteral("[^a-zA-Z0-9]")), QLatin1String(" "))
                        .simplified()
                        .toUpper()
                        .replace(QLatin1Char(' '), QLatin1Char('_'))
                        .append(QLatin1String("_INCLUDED"));

                const auto representation = KDevelop::createCodeRepresentation(m_path);
                const auto lastLine = representation->lines() - 1;
                const auto lastColumn = representation->line(lastLine).length();

                // Add the #endif change before so that it applies correctly in case lastLine == m_startline
                changes.addChange(KDevelop::DocumentChange(m_path,
                                                           KTextEditor::Range(lastLine, lastColumn, lastLine, lastColumn),
                                                           QString(),
                                                           QStringLiteral("\n#endif // %1").arg(macro)));
                changes.addChange(KDevelop::DocumentChange(m_path,
                                                           KTextEditor::Range(m_startLine, 0, m_startLine, 0),
                                                           QString(),
                                                           QStringLiteral("#ifndef %1\n#define %1\n\n").arg(macro)));
                break;
            }
        }

        KDevelop::DUChainReadLocker lock;
        changes.setReplacementPolicy(KDevelop::DocumentChangeSet::WarnOnFailedChange);
        changes.applyAllChanges();
        emit executed(this);
    }

private:
    const GuardType m_type;
    const int m_startLine;
    const KDevelop::IndexedString m_path;
};

HeaderGuardAssistant::HeaderGuardAssistant(const CXTranslationUnit unit, const CXFile file)
    : m_line(std::max(ClangUtils::skipTopCommentBlock(unit, file), 1u) - 1) // skip license etc
    , m_path(QDir(ClangString(clang_getFileName(file)).toString()).canonicalPath())
{
}

QString HeaderGuardAssistant::title() const
{
    return QStringLiteral("Fix-Header");
}

void HeaderGuardAssistant::createActions()
{
    addAction(KDevelop::IAssistantAction::Ptr{new AddHeaderGuardAction(GuardType::Pragma, m_line, m_path)});
    addAction(KDevelop::IAssistantAction::Ptr{new AddHeaderGuardAction(GuardType::Macro, m_line, m_path)});
}

#include "headerguardassistant.moc"
