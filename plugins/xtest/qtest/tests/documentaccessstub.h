/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERITAS_DOCUMENTACCESSSTUB_H
#define VERITAS_DOCUMENTACCESSSTUB_H

#include "../documentaccess.h"
#include <QList>

namespace QTest { namespace Test {

class DocumentAccessStub : public QTest::DocumentAccess
{
public:
    DocumentAccessStub() { m_count = 0; }
    virtual ~DocumentAccessStub() {}
    virtual QString text(const KUrl&, const KDevelop::SimpleRange& range) {
        m_ranges << range;
        QStringList lines;
        foreach(QString line, m_text.split("\n")) lines.append(line+"\n");
        if (lines.isEmpty()) return "";
        for (int i=lines.count(); i>range.end.line+1 && !lines.isEmpty(); i--)
            lines.removeLast();
        for (int i=0; i<range.start.line && !lines.isEmpty(); i++)
            lines.removeFirst();
        if (lines.isEmpty()) return "";
        lines[0] = lines[0].mid(range.start.column);
        if (range.start.line == range.end.line) {
            lines[0] = lines[0].left(range.end.column - range.start.column);
        } else {
            lines[lines.count()-1] = lines[lines.count()-1].left(range.end.column);
        }
        return lines.join("");
    }
    virtual QString text(const KUrl& url) {
        return m_text;
    }
    QString m_text;
    int m_count;
    QList<KDevelop::SimpleRange> m_ranges;
};

}}

#endif // VERITAS_DOCUMENTACCESSTUB_H
