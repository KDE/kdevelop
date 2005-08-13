/***************************************************************************
 *   Copyright (C) 2000 by Dimitri van Heesch                              *
 *   dimitri@stack.nl                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOXYGENCONFIGWIDGET_H_
#define _DOXYGENCONFIGWIDGET_H_

#include <qtabwidget.h>
#include <qdict.h>
#include <qptrlist.h>

class IInput;


class DoxygenConfigWidget : public QTabWidget
{
    Q_OBJECT
    
public:
    DoxygenConfigWidget( const QString &fileName, QWidget *parent=0, const char *name=0 );
    ~DoxygenConfigWidget();

public slots:
    void accept();

private slots:
    void toggle(const QString&, bool);
    void changed();

private:
    QSize sizeHint() const;
    void loadFile();
    void saveFile();
    void init();
    void addDependency(QDict<QObject> *switches,
                       const QCString &dep, const QCString &name);

    QString m_fileName;
    bool m_hasChanged;
    QDict<IInput> *m_inputWidgets;
    QDict< QPtrList<IInput> > *m_dependencies;
    QDict<QObject> *m_switches;
};

#endif
