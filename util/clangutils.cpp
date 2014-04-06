/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "clangutils.h"

#include <QDir>
#include <QProcess>

using namespace KDevelop;

Path::List ClangUtils::defaultIncludeDirectories()
{
    static Path::List includePaths;

    if (!includePaths.isEmpty()) {
        return includePaths;
    }

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);

    // The following command will spit out a bnuch of information we don't care
    // about before spitting out the include paths.  The parts we care about
    // look like this:
    // #include "..." search starts here:
    // #include <...> search starts here:
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/i486-linux-gnu
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/backward
    //  /usr/local/include
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/include
    //  /usr/include
    // End of search list.
    proc.start("clang++", {"-std=c++11", "-xc++", "-E", "-v", "/dev/null"});
    if (!proc.waitForStarted(1000) || !proc.waitForFinished(1000)) {
        return {};
    }

    // We'll use the following constants to know what we're currently parsing.
    enum Status {
        Initial,
        FirstSearch,
        Includes,
        Finished
    };
    Status mode = Initial;

    foreach(const QString& line, QString::fromLocal8Bit(proc.readAllStandardOutput()).split('\n')) {
        switch (mode) {
            case Initial:
                if (line.indexOf("#include \"...\"") != -1) {
                    mode = FirstSearch;
                }
                break;
            case FirstSearch:
                if (line.indexOf("#include <...>") != -1) {
                    mode = Includes;
                    break;
                }
            case Includes:
                //if (!line.indexOf(QDir::separator()) == -1 && line != "." ) {
                //Detect the include-paths by the first space that is prepended. Reason: The list may contain relative paths like "."
                if (!line.startsWith(' ') ) {
                    // We've reached the end of the list.
                    mode = Finished;
                } else {
                    // This is an include path, add it to the list.
                    includePaths << Path(QDir::cleanPath(line.trimmed()));
                }
                break;
            default:
                break;
        }
        if (mode == Finished) {
            break;
        }
    }

    return includePaths;
}
