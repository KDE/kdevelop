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

#include <QString>
#include <QTextStream>

QString prefix = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
                 "<TestCase name=\"QTestCaseTest\">\n"
                 "<Environment>\n"
                 "<QtVersion>4.4.0-rc1</QtVersion>\n"
                 "<QTestVersion>4.4.0-rc1</QTestVersion>\n"
                 "</Environment>\n"
                 "<TestFunction name=\"initTestCase\">\n"
                 "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
                 "</TestFunction>\n";

QString suffix = "<TestFunction name=\"cleanupTestCase\">\n"
                 "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
                 "</TestFunction>\n"
                 "</TestCase>\n";

QString cmdSuccessXml(QString name)
{
    return QString("<TestFunction name=\"") + name + "\">\n"
           "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
           "</TestFunction>\n";
}

QString cmdFailXml(QString name, QString message, QString file, int line)
{
    return QString(
               "<TestFunction name=\"") + name + "\">\n"
           "<Incident type=\"fail\" file=\"" + file + "\" line=\"" + QString::number(line) + "\">\n"
           "<Description><![CDATA[" + message + "]]></Description>\n"
           "</Incident>\n"
           "</TestFunction>\n";
}

int main(int argc, char** argv)
{
    QTextStream cout(stdout);
    QString cmd1 = cmdSuccessXml("cmd1");
    QString cmd2 = cmdFailXml("cmd2", "failure message", "fakeqtest2.cpp", 2);
    if (argc == 2) {
        cout << prefix + cmd1 + cmd2 + suffix << endl;
    } else if (QString(argv[2]) == "cmd1") {
        cout << prefix + cmd1 + suffix << endl;
    } else if (QString(argv[2]) == "cmd2") {
        cout << prefix + cmd2 + suffix << endl;
    }
    return 0;
}
