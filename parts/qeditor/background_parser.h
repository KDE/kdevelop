/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef background_parser_h
#define background_parser_h

#include <qobject.h>

class QEditor;
class QTextParag;
class QTimer;

class BackgroundParser: public QObject
{
    Q_OBJECT
public:
    BackgroundParser( QEditor* =0, const char* =0 );
    virtual ~BackgroundParser();

    QEditor* editor() const { return m_editor; }

    int interval() const { return m_interval; }
    void setInterval( int interval ) { m_interval = interval; }

signals:
    void parsed();

public slots:
    virtual void reparse();
    virtual void sync();

private slots:
    void slotTextChanged();

protected:
    void setParsed( QTextParag*, bool );
    bool isParsed( QTextParag* ) const;

    virtual void invalidate( QTextParag*, QTextParag* );
    virtual void ensureParsed( QTextParag* );

    virtual void parseParag( QTextParag* );
    virtual QTextParag* findGoodStartParag( QTextParag* p ) { return p; }
    virtual QTextParag* findGoodEndParag( QTextParag* p ) { return p; }

private slots:
    void continueParsing();

private:
    QEditor* m_editor;
    QTimer* m_timer;
    QTextParag* m_lastParsed;
    int m_interval;
};


#endif
