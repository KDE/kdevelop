/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_STRINGHELPERS_H
#define KDEVPLATFORM_STRINGHELPERS_H

#include <language/languageexport.h>

#include <QChar>
#include <QScopedPointer>

class QByteArray;
class QString;
class QStringView;

namespace KDevelop {
class ParamIteratorPrivate;

/**
 * @return true if QChar::isSpace() returns true for all elements of @p str.
 */
bool KDEVPLATFORMLANGUAGE_EXPORT consistsOfWhitespace(QStringView str);

/**
 * Searches in the given string for a ',' or a closing bracket equal to @p validEnd,
 * while skipping everything between opened brackets, string and character literals, comments.
 * @param str string to search
 * @param pos position where to start searching
 * @param validEnd a closing bracket type that stops the search
 * @return  On fail, str.length() is returned, else the position of the closing character.
 */
int KDEVPLATFORMLANGUAGE_EXPORT findCommaOrEnd(QStringView str, int pos, QChar validEnd);

/**
 * Extracts the interesting information out of a comment.
 * For example it removes all the stars at the beginning, and re-indents the text.
 */
QString KDEVPLATFORMLANGUAGE_EXPORT formatComment(const QString& comment);

/**
 * Extracts the interesting information out of a comment.
 * For example it removes all the stars at the beginning, and re-indents the text.
 */
QByteArray KDEVPLATFORMLANGUAGE_EXPORT formatComment(const QByteArray& comment);

/**
 * Removes all whitespace from the string
 */
QString KDEVPLATFORMLANGUAGE_EXPORT removeWhitespace(const QString& str);

/**
 * Can be used to iterate through different kinds of parameters, for example template-parameters
 */
class KDEVPLATFORMLANGUAGE_EXPORT ParamIterator
{
public:
    /**
     * @param parens Should be a string containing the two parens between which the parameters are searched.
     * Example: "<>" or "()" Optionally it can also contain one third end-character.
     * If that end-character is encountered in the prefix, the iteration will be stopped.
     *
     * Example: When "<>:" is given, ParamIterator will only parse the first identifier of a C++ scope
     *
     * @warning The QStringView arguments must remain valid and unchanged during ParamIterator's lifetime.
     */
    explicit ParamIterator(QStringView parens, QStringView source, int start = 0);
    ~ParamIterator();

    ParamIterator& operator ++();

    /**
     * Returns current found parameter
     */
    QStringView operator*() const;

    /**
     * Returns whether there is a current found parameter
     */
    operator bool() const;

    /**
     * Returns the text in front of the first opening-paren(if none found then the whole text)
     */
    QStringView prefix() const;

    uint position() const;

private:
    const QScopedPointer<class ParamIteratorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ParamIterator)
};
}

#endif
