#include "cmakecachemodel.h"
#include <QFile>
#include <QRegExp>
#include <KDebug>
#include <KLocale>

//4 columns: name, type, value, comment
//name:type=value - comment
CMakeCacheModel::CMakeCacheModel(QObject *parent, const KUrl &path)
    : QStandardItemModel(parent)
{
    QStringList labels;
    labels.append(i18n("Name"));
    labels.append(i18n("Type"));
    labels.append(i18n("Value"));
    labels.append(i18n("Comment"));
    labels.append(i18n("isInternal"));
    setVerticalHeaderLabels(labels);
    
    int currentIdx=0;
    QStringList currentComment;
    QFile file(path.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        kDebug(9032) << "error. Could not find the file";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if(line.startsWith("//"))
            currentComment += line.right(line.count()-2);
        else if(!line.isEmpty()) //it is a variable
        {
            QRegExp rx(QString("(%1+):?(%1*)=(%1*)").arg("[0-9a-zA-Z_/; \\+'.\\-]"));
            if(rx.exactMatch(line))
            {
                QString isInternal;
                rx.indexIn(line);
                QStringList info = rx.capturedTexts();
                QList<QStandardItem*> lineItems;
                lineItems.append(new QStandardItem(info[1]));
                lineItems.append(new QStandardItem(info[2]));
                lineItems.append(new QStandardItem(info[3]));
                lineItems.append(new QStandardItem(currentComment.join("\n")));
                if(info[2]=="INTERNAL")
                {
                    isInternal="TRUE";
                }
                
                lineItems.append(new QStandardItem(isInternal));
                insertRow(currentIdx, lineItems);
//                 kDebug(9032) << "CacheLine: " << info;
                currentComment.clear();
                currentIdx++;
            }
            else if(!line.startsWith('#'))
            {
                kDebug(9032) << "unrecognized cache line: " << line;
            }
        }
    }
}

#include "cmakecachemodel.moc"

//kate: space-indent on; indent-width 4; replace-tabs on;
