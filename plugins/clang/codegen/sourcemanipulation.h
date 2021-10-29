/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    /// @param body function-body, including parens
    bool insertFunctionDeclaration(KDevelop::Declaration* decl, const KDevelop::Identifier& id, const QString& body = QString());

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
    KDevelop::DUContextPointer m_context;
    KDevelop::QualifiedIdentifier m_scope;
    KDevelop::TopDUContextPointer m_topContext;
    // Represents the whole code of the manipulated top-context for reading.
    // Must be checked for zero before using. It is zero if the file could not be read.
    const KDevelop::CodeRepresentation::Ptr m_codeRepresentation;
};

#endif // CLANG_SOURCEMANIPULATION_H
