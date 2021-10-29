/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGUTILS_H
#define CLANGUTILS_H

#include <util/path.h>

#include <clang-c/Index.h>

#include "clangprivateexport.h"
#include "../duchain/unsavedfile.h"

#include <language/duchain/classfunctiondeclaration.h>

namespace ClangUtils
{
    /**
     * Finds the most specific CXCursor which applies to the specified line and column
     * in the given translation unit and file.
     *
     * @param line The 0-indexed line number at which to search.
     * @param column The 0-indexed column number at which to search.
     * @param unit The translation unit to examine.
     * @param file The file in the translation unit to examine.
     *
     * @return The cursor at the specified location
     */
    CXCursor getCXCursor(int line, int column, const CXTranslationUnit& unit, const CXFile& file);

    enum DefaultArgumentsMode
    {
        FixedSize, ///< The vector will have length equal to the number of arguments to the function
                   /// and any arguments without a default parameter will be represented with an empty string.
        MinimumSize ///< The vector will have a length equal to the number of default values
    };

    /**
     * Given a cursor representing a function, returns a vector containing the string
     * representations of the default arguments of the function which are defined at
     * the occurrence of the cursor. Note that this is not necessarily all of the default
     * arguments of the function.
     *
     * @param cursor The cursor to examine
     * @return a vector of QStrings representing the default arguments, or an empty
     *         vector if cursor does not represent a function
     */
    QVector<QString> getDefaultArguments(CXCursor cursor, DefaultArgumentsMode mode = FixedSize);

    /**
     * @return true when the cursor kind references a named scope.
     */
    bool isScopeKind(CXCursorKind kind);

    /**
     * @brief Retrieve a list of all unsaved files.
     *
     * @note Since this reads text from the editor widget, it must be called from the
     *       GUI thread or with the foreground lock held.
     *
     * @return vector of all unsaved files and their current contents
     */
    KDEVCLANGPRIVATE_EXPORT QVector<UnsavedFile> unsavedFiles();

    /**
     * Given a cursor and destination context, returns the string representing the
     * cursor's scope at its current location.
     *
     * @param cursor The cursor to examine
     * @param context The destination context from which the cursor should be referenced.
     *                By default this will be set to the cursors lexical parent.
     * @return the cursor's scope as a string
     */
    KDEVCLANGPRIVATE_EXPORT QString getScope(CXCursor cursor, CXCursor context = clang_getNullCursor());

    /**
     * Given a cursor representing some sort of function, returns its signature. The
     * effect of this function when passed a non-function cursor is undefined.
     *
     * @param cursor The cursor to work with
     * @param scope The scope of the cursor (e.g. "SomeNS::SomeClass")
     * @return A QString of the function's signature
     */
    QString getCursorSignature(CXCursor cursor, const QString& scope, const QVector<QString>& defaultArgs = QVector<QString>());

    /**
     * Given a cursor representing the template argument list, return a
     * list of the argument types.
     *
     * @param cursor The cursor to work with
     * @return A QStringList of the template's arguments
     */
    KDEVCLANGPRIVATE_EXPORT QStringList templateArgumentTypes(CXCursor cursor);

    /**
     * Extract the raw contents of the range @p range
     *
     * @note This will return the exact textual representation of the code,
     *   no whitespace stripped, etc.
     *
     * @param unit Translation unit this range is part of
     */
    KDEVCLANGPRIVATE_EXPORT QString getRawContents(CXTranslationUnit unit, CXSourceRange range);

    /**
     * @brief Return true if file @p file1 and file @p file2 are equal
     *
     * @see clang_File_isEqual
     */
    inline bool isFileEqual(CXFile file1, CXFile file2)
    {
        return clang_File_isEqual(file1, file2);
    }

    /**
     * @brief Return true if the cursor @p cursor refers to an explicitly deleted/defaulted function
     * such as the default constructor in "struct Foo { Foo() = delete; }"
     *
     * TODO: do we need isExplicitlyDefaulted() + isExplicitlyDeleted()?
     * Currently this is only used by the implements completion to hide deleted+defaulted functions so
     * we don't need to know the difference.
     */
    bool isExplicitlyDefaultedOrDeleted(CXCursor cursor);

    /**
    * Extract the range of the path-spec inside the include-directive in line @p line
    *
    * Example: line = "#include <vector>" => returns {0, 10, 0, 16}
    *
    * @param originalRange This is the range that the resulting range will be based on
    *
    * @return Range pointing to the path-spec of the include or invalid range if there is no #include directive on the line.
    */
    KDEVCLANGPRIVATE_EXPORT KTextEditor::Range rangeForIncludePathSpec(const QString& line, const KTextEditor::Range& originalRange = KTextEditor::Range());

    /**
     * Returns special attributes (isFinal, isQtSlot, ...) given a @p cursor representing a CXXmethod
     */
    KDevelop::ClassFunctionFlags specialAttributes(CXCursor cursor);

    /**
     * @return the top most line in a file skipping any comment block
     */
    unsigned int skipTopCommentBlock(CXTranslationUnit unit, CXFile file);
}

#endif // CLANGUTILS_H
