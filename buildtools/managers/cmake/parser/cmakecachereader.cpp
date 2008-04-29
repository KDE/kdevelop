#include "cmakecachereader.h"

#include <QString>
#include <QDebug>

CacheLine readLine(const QString& line)
{
    CacheLine c;
    int i;
    for(i=0; i<line.count() && line[i]!='='; i++)
    {
        if(line[i]==':')
        {
            c.colon=i;
            if(c.endName<0)
                c.endName=i;
        }
        else if(line[i]=='-')
        {
            c.dash=i;
                c.endName=i;
        }
    }
    c.equal=i;
    return c;
}

/*void readCache(const KUrl &path)
{
    QFile file(path.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        kDebug(9032) << "error. Could not find the file";
        return;
    }

    QHash cacheValues;
    int currentIdx=0;
    QStringList currentComment;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        else if(!line.isEmpty() && !line.startsWith("//")) //it is a variable
        {
            CacheLine c = readLine(line)s
            QString value = line.right(line.count()-c.equal);
        }
    }
}*/
