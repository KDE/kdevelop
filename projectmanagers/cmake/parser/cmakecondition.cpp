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
    ret[CMakeCondition::IS_ABSOLUTE]=3;
    ret[CMakeCondition::LPR]=-1;
    ret[CMakeCondition::RPR]=-1;
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
    ret["IS_ABSOLUTE"]=CMakeCondition::IS_ABSOLUTE;
    ret["MATCHES"]=CMakeCondition::MATCHES;
    ret["LESS"]=CMakeCondition::LESS;
    ret["GREATER"]=CMakeCondition::GREATER;
    ret["EQUAL"]=CMakeCondition::EQUAL;
    ret["STRLESS"]=CMakeCondition::STRLESS;
    ret["STRGREATER"]=CMakeCondition::STRGREATER;
    ret["STREQUAL"]=CMakeCondition::STREQUAL;
    ret["DEFINED"]=CMakeCondition::DEFINED;
    ret["("]=CMakeCondition::LPR;
    ret[")"]=CMakeCondition::RPR;
    return ret;
}

QVector<int> CMakeCondition::m_priorities=initPriorities();
QMap<QString, CMakeCondition::conditionToken> CMakeCondition::nameToToken=initNameToToken();
QSet<QString> CMakeCondition::s_falseDefinitions=QSet<QString>() << "" << "0" << "N" << "NO" << "OFF" << "FALSE" << "NOTFOUND" ;
QSet<QString> CMakeCondition::s_trueDefinitions=QSet<QString>() << "1" << "ON" << "YES" << "TRUE" << "Y";

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

bool CMakeCondition::isTrue(const QStringList::const_iterator& it)
{
//     qDebug() << "+++++++ isTrue: " << varName;
    QString val = *it;
    QString valUpper = val.toUpper();
    bool ret;

//         Documentation currently say
//         * if(<constant>)
//         *    True if the constant is 1, ON, YES, TRUE, Y, or a non-zero number. False if the constant is 0, OFF, NO, FALSE, N, IGNORE, "", or ends in the suffix '-NOTFOUND'. Named boolean constants are case-insensitive.
//         Then, if not one of the named constant, it is apparently treated as a variables or an expression.
    if (!val.isEmpty() && val.at(val.size()-1).isDigit())
    {
        // Number case
        bool ok;
        int n = val.toInt(&ok);
        if (ok && n!=0) {
            ret = true;
        } else {
            ret = false;
        }
    }
    else if (s_trueDefinitions.contains(valUpper))
    {
        // TODO Don't go here if CMP0012 is OLD
        ret = true;
    }
    else if (s_falseDefinitions.contains(valUpper) || valUpper.endsWith("-NOTFOUND"))
    {
        // TODO Don't go here if CMP0012 is OLD
        ret = false;
    }
    else if(m_vars->contains(val))
    {
        //         A variable is expanded (dereferenced) and then checked if it equals one of the above
        //         FALSE constants.
        QString varName=*it;
        m_varUses.append(it);
        const QStringList valu=m_vars->value(varName);
        val = valu.join(";").toUpper();
        ret=!s_falseDefinitions.contains(val) && !val.endsWith("_NOTFOUND");
//         kDebug(9042) << "Checking" << varName << "is true ? >>>" << m_vars->value(varName) << "<<<";
    } else {
        // Treat as variable, and expand it to "", which evaluates to false.
        ret = false;
    }

    return ret;
}

QStringList::const_iterator CMakeCondition::prevOperator(QStringList::const_iterator it, QStringList::const_iterator itStop) const
{
    bool done=false;
    --it;

    while(!done && it!=itStop)
    {
        conditionToken c = typeName(*it);
        done = c>variable;
        if(!done)
            --it;
    }
    return it;
}

#define CHECK_PREV(it) if(it==this->conditionBegin) return false
#define CHECK_NEXT(it) if(it+1!=itEnd) return false

bool CMakeCondition::evaluateCondition(QStringList::const_iterator itBegin, QStringList::const_iterator itEnd)
{
//     qDebug() << "xxxx" << *itBegin << *itEnd;
    if(itBegin==itEnd)
    {
        return isTrue(itBegin);
    }
    
    bool last = false, done=false;
    last = isTrue(prevOperator(itEnd, itBegin)+1);
    while(!done && itBegin!=itEnd)
    {
        QStringList::const_iterator it2 = prevOperator(itEnd, itBegin);
        
        done=(itBegin==it2);
        conditionToken c = typeName(*it2);
        
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
                if(v.isEmpty())
                    kDebug(9042) << "error: no parameter to exist";
                else
                {
                    last=false;
                    
                    Q_ASSERT(m_vars->contains("CMAKE_CURRENT_SOURCE_DIR"));
                    QString dir=m_vars->value("CMAKE_CURRENT_SOURCE_DIR").first();
                    QFileInfo f(dir, v);
                    last=f.exists();
                }
                itEnd=it2;
            }   break;
            case IS_DIRECTORY: {
                CHECK_NEXT(it2);
                QFileInfo f(*(it2+1));
                last = f.isDir();
                itEnd=it2;
            }   break;
            case IS_ABSOLUTE: {
                CHECK_NEXT(it2);
                QFileInfo f(*(it2+1));
                last = f.isAbsolute();
                itEnd=it2;
            }   break;
            case DEFINED:
                CHECK_NEXT(it2);
                last=m_vars->contains(*(it2+1));
                itEnd=it2;
                break;
            case AND:
//                 qDebug() << "AND" << last;
                return evaluateCondition(itBegin, it2-1) && last;
            case OR:
//                 qDebug() << "OR" << last;
                return evaluateCondition(itBegin, it2-1) || last;
            case MATCHES: {
                CHECK_PREV(it2);
                CHECK_NEXT(it2);
                QRegExp rx(*(it2+1));
                if(m_vars->contains(*(it2-1)))
                    rx.indexIn(m_vars->value(*(it2-1)).join(QString()));
                else
                    rx.indexIn(*(it2-1));
                last=rx.matchedLength()>0;
                
                itEnd=it2-1;
            }   break;
            case LESS: {
                CHECK_PREV(it2);
                CHECK_NEXT(it2);
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                int a=strA.toInt(), b=strB.toInt();
                last= (a<b);
                itEnd=it2-1;
            }   break;
            case GREATER: {
                CHECK_PREV(it2);
                CHECK_NEXT(it2);
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                int a=strA.toInt(), b=strB.toInt();
                last= (a>b);
                itEnd=it2-1;
            }   break;
            case EQUAL: {
                CHECK_PREV(it2);
                CHECK_NEXT(it2);
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                int a=strA.toInt(), b=strB.toInt();
                last= (a==b);
                itEnd=it2-1;
            }   break;
            case STRLESS: {
                CHECK_PREV(it2);
                CHECK_NEXT(it2);
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                last= (strA<strB);
                itEnd=it2-1;
            }   break;
            case STRGREATER: {
                CHECK_PREV(it2);
                CHECK_NEXT(it2);
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                last= (strA>strB);
                itEnd=it2-1;
            }   break;
            case STREQUAL: {
                CHECK_PREV(it2);
                CHECK_NEXT(it2);
                QString strA=*(it2-1);
                QString strB=*(it2+1);
                
                if(m_vars->contains(strA))
                    strA=m_vars->value(strA).join(";");
                last= (strA==strB);
                
                itEnd=it2-1;
            }   break;
            case IS_NEWER_THAN: {
                CHECK_PREV(it2);
                CHECK_NEXT(it2);
                QFileInfo pathA(*(it2-1));
                QFileInfo pathB(*(it2+1));
//                 kDebug(9042) << "newer" << strA << strB;
                last= (pathA.lastModified()>pathB.lastModified());
                itEnd=it2-1;
            }   break;
            case LPR: {
                itEnd=it2;
            } break;
            case RPR: {
                QStringList::const_iterator itL=it2;
                int ind=0;
                while(it2!=itBegin)
                {
                    if(*itL=="(") ind--;
                    else if(*itL==")") ind++;
                    
                    if(ind==0)
                        break;
                    --itL;
                }
                last=evaluateCondition(itL, it2-1);
                itEnd=itL;
            } break;
            case variable:
                last = isTrue(it2);
                break;
            default:
                kWarning(9042) << "no support for operator:" << *it2;
                break;
        }
    }
    
    return last;
}

bool CMakeCondition::condition(const QStringList &expression)
{
    if( expression.isEmpty() ) 
    {
        return false;
    }
    QStringList::const_iterator it = expression.constBegin(), itEnd=expression.constEnd();
    conditionBegin=it;
    
    bool ret = evaluateCondition(it, itEnd-1);
    uint i=0;
    m_argUses.clear();
    for(; it!=itEnd; ++it, ++i)
    {
        if(m_varUses.contains(it))
            m_argUses.append(i);
    }
    
//     kDebug(9042) << "condition" << expression << "=>" << ret;
    return ret;
}
