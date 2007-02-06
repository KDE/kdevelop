
/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/

#ifndef %{APPNAME}_PART_H
#define %{APPNAME}_PART_H

#include <koDocument.h>

class %{APPNAME}Part : public KoDocument
{
    Q_OBJECT
public:
    %{APPNAME}Part( QWidget *parentWidget = 0, const char *widgetName = 0, QObject* parent = 0, const char* name = 0, bool singleViewMode = false );

    virtual void paintContent( QPainter& painter, const QRect& rect, bool transparent = FALSE, double zoomX = 1.0, double zoomY = 1.0 );

    virtual bool initDoc( InitDocFlags flags, QWidget* parentWidget = 0 );

    virtual bool loadOasis( const QDomDocument & doc, KoOasisStyles& oasisStyles, const QDomDocument & settings, KoStore* store );
    virtual bool loadXML( QIODevice *, const QDomDocument & );
    virtual QDomDocument saveXML();
    virtual bool saveOasis( KoStore* store, KoXmlWriter* manifestWriter );

protected:
    virtual KoView* createViewInstance( QWidget* parent, const char* name );
};

#endif
