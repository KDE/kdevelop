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
#include <QFile>
#include <QTextStream>

int main(int argc, char** argv)
{
    QFile f(argv[argc-1]);
    f.open(QIODevice::WriteOnly);
    QTextStream cout(&f);
    cout << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
            "<TestCase name=\"QTestCaseTest\">\n"
            "<Environment>\n"
            "<QtVersion>4.4.0-rc1</QtVersion>\n"
            "<QTestVersion>4.4.0-rc1</QTestVersion>\n"
            "</Environment>\n"
            "<TestFunction name=\"initTestCase\">\n"
            "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
            "</TestFunction>\n"
            "<TestFunction name=\"cmd1\">\n"
                "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
            "</TestFunction>\n";
    // do not terminate this xml output properly.
    f.close();
    return 0;
}
