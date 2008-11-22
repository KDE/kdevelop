/* KDevelop CMake Support
 *
 * Copyright 2007-2008 Aleix Pol <aleixpol@gmail.com>
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

#include "cmakecondition.h"
#include "cmakeprojectvisitor.h"
#include <language/duchain/declaration.h>
#include <KDebug>
#include <QRegExp>
#include <QFileInfo>
#include <QDateTime>

#include "astfactory.h"

QVector<int> initPriorities()
{
    QVector<int> ret(CMakeCondition::Last);
    for(int i=CMakeCondition::None; i<CMakeCondition::Last; i++) {
        ret[i]=-1;
    }
    ret[CMakeCondition::AND]=0;
    ret[CMakeCondition::OR]=0;
    ret[CMakeCondition::NOT]=1;
    ret[CMakeCondition::IS_NEWER_THAN]=2;
    ret[CMakeCondition::MATCHES]=2;
    ret[CMakeCondition::LESS]=2;
    ret[CMakeCondition::GREATER]=2;
    ret[CMakeCondition::EQUAL]=2;
    ret[CMakeCondition::STRLESS]=2;
    ret[CMakeCondition::STRGREATER]=2;
    ret[CMakeCondition::STREQUAL]=2;
    ret[CMakeCondition::DEFINED]=3;
    ret[CMakeCondition::COMMAND]=3;
    ret[CMakeCondition::EXISTS]=3;
    ret[CMakeCondition::IS_DIRECTORY]=3;
    return ret;
}

QMap<QString, CMakeCondition::conditionToken> initNameToToken()
{
    QMap<QString, CMakeCondition::conditionToken> ret;
    ret["NOT"]=CMakeCondition::NOT;
    ret["AND"]=CMakeCondition::AND;
    ret["OR"]=CMakeCondition::OR;
    ret["COMMAND"]=CMakeCondition::COMMAND;
    ret["EXISTS"]=CMakeCondition::EXISTS;
    ret["IS_NEWER_THAN"]=CMakeCondition::IS_NEWER_THAN;
    ret["IS_DIRECTORY"]=CMakeCondition::IS_DIRECTORY;
    ret["MATCHES"]=CMakeCondition::MATCHES;
    ret["LESS"]=CMakeCondition::LESS;
    ret["GREATER"]=CMakeCondition::GREATER;
    ret["EQUAL"]=CMakeCondition::EQUAL;
    ret["STRLESS"]=CMakeCondition::STRLESS;
    ret["STRGREATER"]=CMakeCondition::STRGREATER;
    ret["STREQUAL"]=CMakeCondition::STREQUAL;
    ret["DEFINED"]=CMakeCondition::DEFINED;
    return ret;
}

QVector<int> CMakeCondition::m_priorities=initPriorities();
QMap<QString, CMakeCondition::conditionToken> CMakeCondition::nameToToken=initNameToToken();

CMakeCondition::CMakeCondition(const CMakeProjectVisitor* v) : m_vars(v->variables()), m_visitor(v)
{
}

CMakeCondition::conditionToken CMakeCondition::typeName(const QString& _name)
{
    if(nameToToken.contains(_name))
        return nameToToken[_name];
    else 
        return variable;
}

bool CMakeCondition::isTrue(const QString& varName) const
{
//     kDebug(9042) << "+++++++ isTrue: " << varName;
    
    if(m_vars->contains(varName))
    {
        const QStringList valu=m_vars->value(varName);

//         kDebug(9042) << "Checking" << varName << "is true ? >>>" << m_vars->value(varName) << "<<<";
        QString val = valu.join(";").toUpper();
        return !val.isEmpty() && val!="0" && val!="N" && val!="NO" && val!="OFF" && val!="FALSE" && val!="NOTFOUND" && !val.endsWith("_NOTFOUND");
    }
    else
        return false;
}

QStringList::const_iterator CMakeCondition::prevOperator(QStringList::const_iterator it, QStringList::const_iterator itStop) const
{
    bool done=false;
    it--;
//     kDebug(9042) << "it" << *it;
    while(!done && it!=itStop)
    {
//         qDebug() << "oper " << *it;
        conditionToken c = typeName(*it);
        done = c>variable;
        if(!done)
            it--;
    }
    return it;
}

bool CMakeCondition::evaluateCondition(QStringList::const_iterator itBegin, QStringList::const_iterator itEnd) const
{
    if(itBegin==itEnd)
    {
        return isTrue(*itBegin);
    }
    
    bool last = false, done=false;
    last = isTrue(*(prevOperator(itEnd, itBegin)+1));
    while(!done && itBegin!=itEnd)
    {
        QStringList::const_iterator it2;
        it2 = prevOperator(itEnd, itBegin);
        
        done=(itBegin==it2);
        conditionToken c = typeName(*it2);

//         qDebug() << "operator" << *it2 << done;
        QString cmd;
        
        switch(c)
        {
            case NOT:
                last = !last;
                itEnd=it2;
                break;
            case COMMAND:
                last = AstFactory::self()->contains((it2+1)->toLower()) || m_visitor->hasMacro((it2+1)->toLower());
                itEnd=it2;
                break;
            case EXISTS:
            {
                QString v=*(it2+1);
//                 kDebug(9042) << "EXISTS" << v << *it2;
                if(v.isEmpty())
                    kDebug(9042) << "error: no";
                else
                {
                    last=false;
                    QFileInfo f(v);
                    if(f.exists())
                    {
                        last=true;
                    }
                }
                itEnd=it2;
            }   break;
            case IS_DIRECTORY: {
                QFileInfo f(*(it2+1));
                last = f.isDir();
                itEnd=it2;
            }   break;
            case DEFINED:
                last=m_vars->contains(*(it2+1));
                itEnd=it2;
                break;
            case AND:
                return evaluateCondition(itBegin, it2-1) && last;
            case OR:
                return evaluateCondition(itBegin, it2-1) || last;
            case MATCHES: {
                QRegExp rx(*(it2+1));
                if(m_vars->contains(*(it2-1)))
                    rx.indexIn(m_vars->value(*(it2-1)).join(""));
                else
                    rx.indexIn(*(it2-1));
                last=rx.matchedLength()>0;
                itEnd=it2-1;
            }   break;
            case LESS: {
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                int a=strA.toInt(), b=strB.toInt();
                last= (a<b);
                itEnd=it2-1;
            }   break;
            case GREATER: {
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                int a=strA.toInt(), b=strB.toInt();
                last= (a>b);
                itEnd=it2-1;
            }   break;
            case EQUAL: {
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                int a=strA.toInt(), b=strB.toInt();
                last= (a==b);
                itEnd=it2-1;
            }   break;
            case STRLESS: {
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                last= (strA<strB);
                itEnd=it2-1;
            }   break;
            case STRGREATER: {
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                last= (strA>strB);
                itEnd=it2-1;
            }   break;
            case STREQUAL: {
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                last= (strA==strB);
                itEnd=it2-1;
            }   break;
            case IS_NEWER_THAN: {
                QFileInfo pathA(*(it2-1));
                QFileInfo pathB(*(it2+1));
//                 kDebug(9042) << "newer" << strA << strB;
                last= (pathA.lastModified()>pathB.lastModified());
                itEnd=it2-1;
            }
            default:
                kWarning(9042) << "no support for operator:" << *it2;
                break;
        }
    }
    return last;
}

bool CMakeCondition::condition(const QStringList &expression) const
{
    if( expression.isEmpty() ) 
    {
        return false;
    }
    bool ret = evaluateCondition(expression.constBegin(), expression.constEnd()-1);
//     kDebug(9042) << "condition" << expression << "=>" << ret;
    return ret;
}
