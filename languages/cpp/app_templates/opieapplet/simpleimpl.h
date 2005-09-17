#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H


/**
 * Opie and Qtopia uses a component system called QCOM
 * which was first part of the Qt 3.0 API but was made
 * prviate during the betas. Opie and Qtopia still use it
 * and we're happy with it.
 * Every starts with the QUnknownInterface. It supports functions
 * for reference counting and the most important one
 * is for a query. Every QCOM interface got a global unique id ( GUID,UUID )
 * query is used to see if a interface is supported by
 * a dynamic shared object ( dso / plugin )
 * For tasks like loading Applications, InputMethods, Today, MenuButton,
 * Taskbar, Style, Email Client there are specefic Interfaces you
 * need to implement. The interfaces inherits from QUnknownInterface and
 * you'll need inherit from the interface.
 * As example we will use the Taskbar interface
 */


/*
 * The taskbar applet interfaces wants us to implement position() and applet()
 * additionally we need to implement add(), release() and queryInterface for QUnknownInterface
 * luckiy there is a macro for the reference counting
 * We provide an Implementation of the interface.
 */
#include <qwidget.h>
#include <qptrlist.h>

#include <qpe/taskbarappletinterface.h>


/*
 * Because we only draw an Icon in a fixed width and height
 * we declare and define  %{APPNAME} here and you could use QLabel
 * setPixmap or use QWidget and draw yourself.
 * You might also want to reimplement mouse*Event to use some simple actions
 */
class  %{APPNAME} : public QWidget {
    Q_OBJECT
public:
     %{APPNAME}(QWidget *parent);
    ~%{APPNAME}();
private:
    void mousePressEvent( QMouseEvent* );
    void paintEvent( QPaintEvent* );
    QPixmap *m_pix;
};

class %{APPNAME}Impl : public TaskbarAppletInterface {
public:

    %{APPNAME}Impl();
    virtual ~%{APPNAME}Impl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );

    QWidget *applet( QWidget* parent );
    int position()const;

    /*
     * macro for reference countint
     * if reference drops to zero
     * delete this is called
     */
    Q_REFCOUNT

private:
    QList<%{APPNAME}> m_applets;
};


#endif
