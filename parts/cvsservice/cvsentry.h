//
// C++ Interface: cvsentry
//
// Description: This is a simple class for parsing CVS/Entries files found in
// any directory of CVS-managed projects.
//
// Author: Mario Scalas <mario.scalas@libero.it>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef CVSENTRY_H
#define CVSENTRY_H

#include <qstringlist.h>

class CVSEntry
{
public:
    enum  EntryType { invalidEntry, fileEntry, directoryEntry };

    static const QString invalidMarker;
    static const QString directoryMarker;
    static const QString fileMarker;
    static const QString entrySeparator;

    static void parse( const QString &aLine, CVSEntry &entry );

    CVSEntry();
    CVSEntry( const QString &aLine );

    void clean();

    bool isValid() const { return type() != invalidEntry; }

    EntryType type() const;
    QString fileName() const;
    QString revision() const;
    QString timeStamp() const;
    QString options() const;
    QString tag() const;

private:
    EntryType m_type;
    QStringList m_fields;
};

#endif
