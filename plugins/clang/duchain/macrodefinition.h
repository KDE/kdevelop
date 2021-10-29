/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MACRODEFINITION_H
#define MACRODEFINITION_H

#include "clangprivateexport.h"

#include <language/duchain/declaration.h>
#include <language/duchain/duchainregister.h>
#include <serialization/indexedstring.h>

class MacroDefinitionData;

/**
 * @brief Represents a single C/C++ macro definition in the DUCHain
 *
 * This declaration represents a single macro definition, such as:
 * @code
 * #define FOO_BAR(x) do_something_with(x)
 * @endcode
 *
 * In this example, the identifer is 'FOO_BAR', and the macro is function-like
 *
 * @note API designed after of https://clang.llvm.org/doxygen/classclang_1_1MacroInfo.html
 */
class KDEVCLANGPRIVATE_EXPORT MacroDefinition : public KDevelop::Declaration
{
public:
    using Ptr = KDevelop::DUChainPointer<MacroDefinition>;

    MacroDefinition(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    explicit MacroDefinition(MacroDefinitionData& data);
    MacroDefinition(const MacroDefinition& rhs);
    ~MacroDefinition() override;

    /**
     * The definition text of this macro
     *
     * Example:
     * @code
     * #define FOO_BAR(x) do_something_with(x)
     * @endcode
     *
     * Here, "do_something_with(x)" is the definition text
     */
    KDevelop::IndexedString definition() const;
    void setDefinition(const KDevelop::IndexedString& definition);

    /**
     * Whether this macro is a function or not
     *
     * Example:
     * @code
     * #define FOO_BAR(x) x     // function-like
     * #define FOO_BAR x        // not function-like
     * @endcode
     */
    bool isFunctionLike() const;
    void setFunctionLike(bool isFunctionLike);

    const KDevelop::IndexedString* parameters() const;
    unsigned int parametersSize() const;
    void addParameter(const KDevelop::IndexedString& str);
    void clearParameters();

    QString toString() const override;

    enum {
        Identity = 143
    };

private:
    DUCHAIN_DECLARE_DATA(MacroDefinition)
};

DUCHAIN_DECLARE_TYPE(MacroDefinition)

#endif // MACRODEFINITION_H
