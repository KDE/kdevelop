/*
 * This file is part of KDevelop
 *
 * Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#ifndef CLANG_SOURCEMANIPULATION_H
#define CLANG_SOURCEMANIPULATION_H

#include <language/codegen/coderepresentation.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/declaration.h>

class SourceCodeInsertion : public QSharedData
{
public:
    explicit SourceCodeInsertion(KDevelop::TopDUContext* topContext);
    ~SourceCodeInsertion();

    /// Set optional sub-scope into which the code should be inserted, under 'context'
    void setSubScope(const KDevelop::QualifiedIdentifier& scope);

    struct SignatureItem {
        KDevelop::AbstractType::Ptr type;
        QString name;
    };

    /// @param body function-body, including parens
    bool insertFunctionDeclaration(const KDevelop::Identifier& name, const KDevelop::AbstractType::Ptr& returnType,
                                           const QList<SourceCodeInsertion::SignatureItem>& signature,
                                           bool isConstant = false, const QString& body = QString());

    KDevelop::DocumentChangeSet changes();

private:

    /// Returns the exact position where the item should be inserted so it is in the given line.
    /// The inserted item has to start with a newline, and does not need to end with a newline.
    KTextEditor::Range insertionRange(int line);

    /// Returns a line for inserting the given declaration
    int findInsertionPoint() const;

    // Should apply m_scope to the given declaration string
    QString applySubScope(const QString& decl) const;

    QString indentation() const;
    QString applyIndentation(const QString& decl) const;

    KTextEditor::Cursor end() const;

private:
    KDevelop::DocumentChangeSet m_changeSet;
    KDevelop::DUContext* m_context;
    KDevelop::QualifiedIdentifier m_scope;
    KDevelop::TopDUContext* m_topContext;
    // Represents the whole code of the manipulated top-context for reading.
    // Must be checked for zero before using. It is zero if the file could not be read.
    const KDevelop::CodeRepresentation::Ptr m_codeRepresentation;
};

#endif // CLANG_SOURCEMANIPULATION_H
