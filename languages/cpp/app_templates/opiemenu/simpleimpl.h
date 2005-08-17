#ifndef %{APPNAME}_MENU_APPLET_H
#define %{APPNAME}_MENU_APPLET_H

#include <qpe/menuappletinterface.h>

#include <qobject.h>
//Added by qt3to4:
#include <Q3PopupMenu>

class %{APPNAME} : public QObject, public MenuAppletInterface
{

    Q_OBJECT

public:
    %{APPNAME} ( );
    virtual ~%{APPNAME} ( );

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual int position() const;
    
    virtual QString name ( ) const;
    virtual QIcon icon ( ) const;
    virtual QString text ( ) const;
    /* virtual QString tr( const char* ) const;
    virtual QString tr( const char*, const char* ) const;
    */
    virtual Q3PopupMenu *popup ( QWidget *parent ) const;
    virtual void activated ( );
};

#endif
