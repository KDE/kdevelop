/* KDevelop CMake Support
 *
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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

#include "cmakecachemodel.h"
#include <QFile>
#include <KLocalizedString>

#include "cmakecachereader.h"
#include <debug.h>

//4 columns: name, type, value, comment
//name:type=value - comment
CMakeCacheModel::CMakeCacheModel(QObject *parent, const KDevelop::Path &path)
    : QStandardItemModel(parent), m_filePath(path)
{
    read();
}

void CMakeCacheModel::reset()
{
    beginResetModel();
    clear();
    m_internal.clear();
    m_modifiedRows.clear();
    read();
    endResetModel();
}

void CMakeCacheModel::read()
{
    // Set headers
    const QStringList labels{
        i18nc("@title:column", "Name"),
        i18nc("@title:column", "Type"),
        i18nc("@title:column", "Value"),
        i18nc("@title:column", "Comment"),
        i18nc("@title:column", "Advanced"),
        i18nc("@title:column", "Strings"),
    };
    setHorizontalHeaderLabels(labels);

    QFile file(m_filePath.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCDebug(CMAKE) << "error. Could not find the file";
        return;
    }

    int currentIdx=0;
    QStringList currentComment;
    QTextStream in(&file);
    QHash<QString, int> variablePos;
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if(line.startsWith(QLatin1String("//")))
        {
            line.remove(0, 2);
            currentComment += line;
        } else if(!line.isEmpty() && !line.startsWith(QLatin1Char('#'))) //it is a variable
        {
            CacheLine c;
            c.readLine(line);

            if(c.isCorrect())
            {
                QString name=c.name(), flag=c.flag();

                QString type=c.type();
                QString value=c.value();

                const QList<QStandardItem*> lineItems{
                    new QStandardItem(name),
                    new QStandardItem(type),
                    new QStandardItem(value),
                    new QStandardItem(currentComment.join(QLatin1Char('\n'))),
                };

                if(flag==QLatin1String("INTERNAL"))
                {
                    m_internal.insert(name);
                } else if(flag==QLatin1String("ADVANCED") || flag==QLatin1String("STRINGS"))
                {
                    const auto posIt = variablePos.constFind(name);
                    if (posIt != variablePos.constEnd()) {
                        const int pos = *posIt;
                        
                        // if the flag is not ADVANCED, it's STRINGS.
                        // The latter is stored in column 5
                        int column = flag==QLatin1String("ADVANCED") ? 4 : 5;
                        QStandardItem *p = item(pos, column);
                        if(!p)
                        {
                            p=new QStandardItem(value);
                            setItem(pos, column, p);
                        }
                        else
                        {
                            p->setText(value);
                        }
                    }
                    else
                    {
                        qCDebug(CMAKE) << "Flag for an unknown variable";
                    }
                }

                if(!flag.isEmpty())
                {
                    lineItems[0]->setText(lineItems[0]->text() + QLatin1Char('-') + flag);
                }
                insertRow(currentIdx, lineItems);
                if (!variablePos.contains(name)) {
                    variablePos[name]=currentIdx;
                }
                currentIdx++;
                currentComment.clear();
            }
        }
        else if(line.startsWith(QLatin1Char('#')) && line.contains(QLatin1String("INTERNAL")))
        {
            m_internalBegin=currentIdx;
//                 qCDebug(CMAKE) << "Comment: " << line << " -.- " << currentIdx;
        }
    }
}

bool CMakeCacheModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool ret = QStandardItemModel::setData(index, value, role);
    if (ret) {
        const auto i = index.row();
        m_modifiedRows.insert(i);
        emit valueChanged(item(i, 0)->text(), item(i, 2)->text());
    }
    return ret;
}

QVariantMap CMakeCacheModel::changedValues() const
{
    QVariantMap ret;
    for(int i=0; i<rowCount(); i++)
    {
        if (!m_modifiedRows.contains(i))
            continue;

        QStandardItem* name = item(i, 0);
        QStandardItem* type = item(i, 1);
        QStandardItem* valu = item(i, 2);
//         QStandardItem* comment = item(i, 3);
        ret.insert(name->text()+QLatin1Char(':')+type->text(), valu->text());

    }
    return ret;
}

QString CMakeCacheModel::value(const QString & varName) const
{
    for(int i=0; i<rowCount(); i++)
    {
        QStandardItem* name = item(i, 0);
        if(name->text()==varName) {
            QStandardItem* valu = item(i, 2);
            return valu->text();
        }
    }
    return QString();
}

bool CMakeCacheModel::isAdvanced(int i) const
{
    QStandardItem *p=item(i, 4);
    bool isAdv= (p!=nullptr) || i>m_internalBegin;
    if(!isAdv)
    {
        p=item(i, 1);
        isAdv = p->text()==QLatin1String("INTERNAL") || p->text()==QLatin1String("STATIC");
    }

    return isAdv || m_internal.contains(item(i,0)->text());
}

bool CMakeCacheModel::isInternal(int i) const
{
    bool isInt= i>m_internalBegin;
    return isInt;
}

QList< QModelIndex > CMakeCacheModel::persistentIndices() const
{
    QList< QModelIndex > ret;
    for(int i=0; i<rowCount(); i++)
    {
        QStandardItem* type = item(i, 1);
        if(type->text()==QLatin1String("BOOL"))
        {
            QStandardItem* valu = item(i, 2);
            ret.append(valu->index());
        }
    }
    return ret;
}

KDevelop::Path CMakeCacheModel::filePath() const
{
    return m_filePath;
}


