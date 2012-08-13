/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdevfilters.h"

#include <KDebug>

using namespace KDevelop;

QString getSafeString(const QVariant& variant)
{
    return variant.value<Grantlee::SafeString>().get();
}

QStringList words(const QVariant& input)
{
    QString string = getSafeString(input);
    if (string == string.toLower() && !string.contains('_'))
    {
        return QStringList(string);
    }

    if (string.contains('_'))
    {
        return string.toLower().split('_');
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
    foreach (const QString& word, words(input))
    {
        QString w = word;
        w[0] = w[0].toUpper();
        ret += w;
    }
    return Grantlee::SafeString(ret);
}

QVariant LowerCamelCaseFilter::doFilter(const QVariant& input, const QVariant& /*argument*/,
                                        bool /*autoescape*/) const
{
    QString ret;
    foreach (const QString& word, words(input))
    {
        QString w = word;
        w[0] = w[0].toUpper();
        ret += w;
    }
    if (!ret.isEmpty())
    {
        ret[0] = ret[0].toUpper();
    }
    return Grantlee::SafeString(ret);
}

QVariant UnderscoreFilter::doFilter(const QVariant& input, const QVariant& /*argument*/,
                                    bool /*autoescape*/) const
{
    QString ret = words(input).join("_");
    return Grantlee::SafeString(ret);
}

QVariant UpperFirstFilter::doFilter(const QVariant& input, const QVariant& /*argument*/,
                                    bool /*autoescape*/) const
{
    QString in = getSafeString(input);
    if (!in.isEmpty())
    {
        in[0] = in[0].toUpper();
    }
    return Grantlee::SafeString(in);
}


QVariant SplitLinesFilter::doFilter(const QVariant& input, const QVariant& argument,
                                    bool /*autoescape*/) const
{
    QStringList retLines;
    QString start = getSafeString(argument);
    foreach (const QString& line, getSafeString(input).split('\n', QString::KeepEmptyParts))
    {
        retLines << start + line;
    }
    return Grantlee::SafeString(retLines.join(QString('\n')));
}


KDevFilters::KDevFilters(QObject* parent)
: QObject(parent)
{

}

KDevFilters::~KDevFilters()
{

}

QHash< QString, Grantlee::Filter* > KDevFilters::filters(const QString& name)
{
    Q_UNUSED(name);
    QHash< QString, Grantlee::Filter* > filters;

    filters["camel_case"] = new CamelCaseFilter();
    filters["camel_case_lower"] = new LowerCamelCaseFilter();
    filters["underscores"] = new UnderscoreFilter();
    filters["lines_prepend"] = new SplitLinesFilter();
    filters["upper_first"] = new UpperFirstFilter();

    return filters;
}

Q_EXPORT_PLUGIN2( kdev_filters, KDevelop::KDevFilters )