/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdevfilters.h"

#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>

using namespace KDevelop;

static
QString safeString(const QVariant& variant)
{
    if (variant.canConvert<KTextTemplate::SafeString>()) {
        return variant.value<KTextTemplate::SafeString>().get();
    } else {
        return variant.toString();
    }
}

QStringList words(const QVariant& input)
{
    QString string = safeString(input);
    if (string == string.toLower() && !string.contains(QLatin1Char('_'))) {
        return QStringList(string);
    }

    if (string.contains(QLatin1Char('_'))) {
        return string.toLower().split(QLatin1Char('_'));
    }

    int n = string.size();
    QStringList ret;
    int last = 0;
    for (int i = 1; i < n; ++i)
    {
        if (string[i].isUpper())
        {
            ret << string.mid(last, i-last).toLower();
            last = i;
        }
    }
    ret << string.mid(last).toLower();
    return ret;
}

QVariant CamelCaseFilter::doFilter(const QVariant& input, const QVariant& /*argument*/,
                                   bool /*autoescape*/) const
{
    QString ret;
    const auto words = ::words(input);
    for (const QString& word : words) {
        QString w = word;
        w[0] = w[0].toUpper();
        ret += w;
    }
    return KTextTemplate::SafeString(ret);
}

QVariant LowerCamelCaseFilter::doFilter(const QVariant& input, const QVariant& /*argument*/,
                                        bool /*autoescape*/) const
{
    QString ret;
    const auto words = ::words(input);
    for (const QString& word :  words) {
        QString w = word;
        w[0] = w[0].toUpper();
        ret += w;
    }
    if (!ret.isEmpty())
    {
        ret[0] = ret[0].toUpper();
    }
    return KTextTemplate::SafeString(ret);
}

QVariant UnderscoreFilter::doFilter(const QVariant& input, const QVariant& /*argument*/,
                                    bool /*autoescape*/) const
{
    QString ret = words(input).join(QLatin1Char('_'));
    return KTextTemplate::SafeString(ret);
}

QVariant UpperFirstFilter::doFilter(const QVariant& input, const QVariant& /*argument*/,
                                    bool /*autoescape*/) const
{
    QString in = safeString(input);
    if (!in.isEmpty())
    {
        in[0] = in[0].toUpper();
    }
    return KTextTemplate::SafeString(in);
}


QVariant SplitLinesFilter::doFilter(const QVariant& input, const QVariant& argument,
                                    bool /*autoescape*/) const
{
    QStringList retLines;
    QString start = safeString(argument);
    const QString inputString = safeString(input);
    const auto lines = QStringView{inputString}.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    retLines.reserve(lines.size());
    for (const auto line : lines) {
        retLines << start + line;
    }
    return KTextTemplate::SafeString(retLines.join(QLatin1Char('\n')));
}

QVariant ArgumentTypeFilter::doFilter (const QVariant& input, const QVariant& /*argument*/,
                                       bool /*autoescape*/) const
{
    auto type = safeString(input);

    auto visit = [&type](const IndexedDeclaration& indexedDeclaration) {
        auto declaration = indexedDeclaration.declaration();
        if (!declaration || declaration->isForwardDeclaration()) {
            return PersistentSymbolTable::VisitorState::Continue;
        }

        // Check if it's a class/struct/etc
        if (declaration->type<StructureType>()) {
            type = QLatin1String("const %1&").arg(type);
            return PersistentSymbolTable::VisitorState::Break;
        }

        return PersistentSymbolTable::VisitorState::Continue;
    };

    DUChainReadLocker locker(DUChain::lock());
    PersistentSymbolTable::self().visitDeclarations(IndexedQualifiedIdentifier(QualifiedIdentifier(type)), visit);

    return KTextTemplate::SafeString(type);
}

KDevFilters::KDevFilters(QObject* parent, const QVariantList &)
: QObject(parent)
{
}

KDevFilters::~KDevFilters()
{

}

QHash<QString, KTextTemplate::Filter*> KDevFilters::filters(const QString& name)
{
    Q_UNUSED(name);
    QHash<QString, KTextTemplate::Filter*> filters;

    filters[QStringLiteral("camel_case")] = new CamelCaseFilter();
    filters[QStringLiteral("camel_case_lower")] = new LowerCamelCaseFilter();
    filters[QStringLiteral("underscores")] = new UnderscoreFilter();
    filters[QStringLiteral("lines_prepend")] = new SplitLinesFilter();
    filters[QStringLiteral("upper_first")] = new UpperFirstFilter();
    filters[QStringLiteral("arg_type")] = new ArgumentTypeFilter();

    return filters;
}

#include "moc_kdevfilters.cpp"
