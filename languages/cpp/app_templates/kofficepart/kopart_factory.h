
/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/

#ifndef %{APPNAME}_FACTORY_H
#define %{APPNAME}_FACTORY_H

#include <koFactory.h>

class KComponentData;
class KAboutData;

class %{APPNAME}Factory : public KoFactory
{
    Q_OBJECT
public:
    %{APPNAME}Factory( QObject* parent = 0, const char* name = 0 );
    ~%{APPNAME}Factory();

    virtual KParts::Part *createPartObject( QWidget *parentWidget = 0, const char *widgetName = 0, QObject *parent = 0, const char *name = 0, const char *classname = "KoDocument", const QStringList &args = QStringList() );

    static const KComponentData &global();

    // _Creates_ a KAboutData but doesn't keep ownership
    static KAboutData* aboutData();

private:
    static KComponentData* s_global;
    static KAboutData* s_aboutData;
};

#endif
