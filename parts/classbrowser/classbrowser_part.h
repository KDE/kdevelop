/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __KDEVPART_CLASSBROWSER_H__
#define __KDEVPART_CLASSBROWSER_H__

#include <qguardedptr.h>
#include <kdevplugin.h>

#include <tag.h>
#include <catalog.h>

class ClassBrowserWidget;
class KSelectAction;

namespace ClassBrowserUtils
{
    inline QValueList<Tag> simplifyNamespaces( const QValueList<Tag>& tags )
    {
	QMap<QString, Tag> map;
	QValueList<Tag>::ConstIterator it = tags.begin();
	while( it != tags.end() ){
	    const Tag& tag = *it;
	    ++it;

	    QString name = tag.name();

	    if( !name.isEmpty() && !map.contains(name) )
		map.insert( name, tag );
	}
	return map.values();
    }

}

class ClassBrowserPart : public KDevPlugin
{
    Q_OBJECT
public:
    ClassBrowserPart(QObject *parent, const char *name, const QStringList &);
    virtual ~ClassBrowserPart();

public slots:
    void refresh();
    void selectNamespace( const QString& name );
    void selectClass( const QString& name );
    void selectMethod( const QString& name );

private slots:
    void slotProjectOpened();
    void slotProjectClosed();
    void slotCatalogAdded( Catalog* catalog );
    void slotCatalogRemoved( Catalog* catalog );
    void slotCatalogChanged( Catalog* catalog );
    void refreshClasses();
    void refreshMethods();

private:
    void setupActions();
    void adjust();

private:
    QGuardedPtr<ClassBrowserWidget> m_widget;
    KSelectAction* m_actionNamespaces;
    KSelectAction* m_actionClasses;
    KSelectAction* m_actionMethods;
    QString m_selectedFileName;
};


#endif
