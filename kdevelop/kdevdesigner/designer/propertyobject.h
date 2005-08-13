#ifndef PROPERTYOBJECT_H
#define PROPERTYOBJECT_H

#include <qobject.h>
#include <qwidgetlist.h>
#include <qcursor.h>

class PropertyObject : public QObject
{
public:
    QMetaObject *metaObject() const { return (QMetaObject*)mobj; }
    const char *className() const { return "PropertyObject"; }
    QObject* qObject() { return (QObject*)this; }
private:
    static QMetaObject *metaObj;

public:
    PropertyObject( const QWidgetList &objs );

    bool setProperty( const char *name, const QVariant& value );
    QVariant property( const char *name ) const;

    void mdPropertyChanged( const QString &property, bool changed );
    bool mdIsPropertyChanged( const QString &property );
    void mdSetPropertyComment( const QString &property, const QString &comment );
    QString mdPropertyComment( const QString &property );
    void mdSetFakeProperty( const QString &property, const QVariant &value );
    QVariant mdFakeProperty( const QString &property );
    void mdSetCursor( const QCursor &c );
    QCursor mdCursor();
    void mdSetPixmapKey( int pixmap, const QString &arg );
    QString mdPixmapKey( int pixmap );
    void mdSetExportMacro( const QString &macro );
    QString mdExportMacro();
    QWidgetList widgetList() const { return objects; }

private:
    QWidgetList objects;
    const QMetaObject *mobj;

};

#endif
