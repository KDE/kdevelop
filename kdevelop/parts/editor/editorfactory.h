/***************************************************************************
                             editorfactory.h
                             ---------------
    copyright            : (C) 2000 by KDevelop team
    email                : kdevelop_team@kdevelop.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _EDITORFACTORY_H_
#define _EDITORFACTORY_H_

#include <klibloader.h>


class EditorFactory : public KLibFactory
{
    Q_OBJECT

public:
    EditorFactory( QObject *parent=0, const char *name=0 );
    ~EditorFactory();

    virtual QObject* create(QObject *parent, const char *name,
                            const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
