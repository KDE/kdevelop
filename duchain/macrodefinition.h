/*
 * Copyright 2014  Kevin Funk <kfunk@kde.org>
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
 */

#ifndef MACRODEFINITION_H
#define MACRODEFINITION_H

#include "duchainexport.h"

#include <language/duchain/declaration.h>
#include <language/duchain/indexedstring.h>

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
 * @note API designed after of http://clang.llvm.org/doxygen/classclang_1_1MacroInfo.html
 */
class KDEVCLANGDUCHAIN_EXPORT MacroDefinition : public KDevelop::Declaration
{
public:
    using Ptr = KDevelop::DUChainPointer<MacroDefinition>;

    MacroDefinition(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    MacroDefinition(MacroDefinitionData& data);
    MacroDefinition(const MacroDefinition& rhs);
    virtual ~MacroDefinition();

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

    enum {
        Identity = 143
    };

private:
    DUCHAIN_DECLARE_DATA(MacroDefinition);
};

#endif // MACRODEFINITION_H
