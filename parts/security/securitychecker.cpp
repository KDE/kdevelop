/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "securitychecker.h"

#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <ktexteditor/editinterface.h>

#include "securitypart.h"
#include "securitywidget.h"

class SecurityPattern {
public:
    enum Kind { Problem, Warning };
    
    SecurityPattern(QRegExp regExp, const QString &problem, const QString &suggestion, Kind kind)
        : m_regExp(regExp), m_problem(problem), m_suggestion(suggestion), m_kind(kind) { }
    virtual ~SecurityPattern() {}
    
    virtual bool apply(const QString &code) {
        if (m_regExp.search(code) != -1)
            return true;
        return false;
    }
    
    QString problem() const { return m_problem; }
    QString suggestion() const { return m_suggestion; }
    Kind kind() const { return m_kind; }
    
private:
    QRegExp m_regExp;
    QString m_problem;
    QString m_suggestion;
    Kind m_kind;
};

SecurityChecker::SecurityChecker(SecurityPart *part, const char *name)
    :QObject(part, name), m_part(part)
{
    QString config = locate("data", "kdevsecurity/patterns");
    if (!config.isEmpty())
        initConfig(config);
}

void SecurityChecker::refresh(int line, int col, const QString &contents)
{
    kdDebug() << "SecurityChecker::refresh: " << line << 
        ", " << col << ", " << contents << endl;

    //@todo be smart and check only the function body where the change has taken place
    check();
}

void SecurityChecker::check()
{
    m_part->widget()->clearResultsForFile(m_part->activeFile());

    int lines = m_part->activeEditor()->numLines();
    for (int i = 0; i < lines; ++i)
    {
        for (QValueList<SecurityPattern*>::iterator it = m_patterns.begin();
                it != m_patterns.end(); ++it)
        {
            kdDebug() << "applying pattern for line " << i << ": " << endl;
            if ((*it)->apply(m_part->activeEditor()->textLine(i)))
            {
                kdDebug() << "........success" << endl;
                if ((*it)->kind() == SecurityPattern::Problem)
                    m_part->widget()->reportProblem(m_part->activeFile(), i, (*it)->problem(), (*it)->suggestion());
                else 
                    m_part->widget()->reportWarning(m_part->activeFile(), i, (*it)->problem(), (*it)->suggestion());
            }
            else
                kdDebug() << "........fail" << endl;
        }
    }
}

void SecurityChecker::initConfig(const QString &file)
{
    kdDebug() << "file: " << file << endl;
    KConfig config(file);
    
//    config.setGroup("Simple Warnings");
//    kdDebug() << "    test: " << config.readEntry("4") << endl;
    QMap<QString, QString> entries = config.entryMap("Simple Warnings");
    for (QMap<QString, QString>::const_iterator it = entries.constBegin();
            it != entries.constEnd(); ++it)
    {
        kdDebug() << "    data: " << it.data() << endl;
        QStringList data = QStringList::split("\t", it.data());
        kdDebug() << "0000000000000-------------- regexp: " << data[1] << " suggestion: " << data[2] << endl;
        m_patterns.append(new SecurityPattern(QRegExp(data[1]), data[0], data[2],  SecurityPattern::Warning));
    }

    entries = config.entryMap("Simple Errors");
    for (QMap<QString, QString>::const_iterator it = entries.constBegin();
            it != entries.constEnd(); ++it)
    {
        QStringList data = QStringList::split("\t", it.data());
        m_patterns.append(new SecurityPattern(QRegExp(data[1]), data[0], data[2],  SecurityPattern::Problem));
    }
}

#include "securitychecker.moc"
