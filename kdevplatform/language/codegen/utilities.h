/*
    SPDX-FileCopyrightText: 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CODEGEN_UTILITIES_H
#define KDEVPLATFORM_CODEGEN_UTILITIES_H

#include <language/languageexport.h>

#include <QValidator>

namespace KDevelop {
class IndexedString;
class DUContext;
class Declaration;

namespace CodeGenUtils {
/*!
 * A validator object that  verifies if a string would be an acceptable identifier
 * If inserted into the given context, including if it conflicts with any other identifier
 */
class KDEVPLATFORMLANGUAGE_EXPORT IdentifierValidator
    : public QValidator
{
    Q_OBJECT

public:
    explicit IdentifierValidator(DUContext* context);
    ~IdentifierValidator() override;

    State validate(QString& input, int&) const override;

private:

    DUContext* m_context;
};

/**
 * @brief Search for the file that contains the implementation of a specified type
 *
 * Search for the file that contains the implementation of @p targetClass. For languages that
 * allow implementation of a type through multiple files, the file with the most implementations of
 * class methods will be chosen, if a tie is found, then the file with the most uses will be chosen.
 * Else the file that contains the declaration is chosen.
 *
 * @note If called with a Forward declaration, the real declaration will be searched for.
 *
 * @return The file that matched best
 */
KDEVPLATFORMLANGUAGE_EXPORT IndexedString
fetchImplementationFileForClass(const Declaration& targetClass);
}
}

#endif  //KDEVPLATFORM_CODEGEN_UTILITIES_H
