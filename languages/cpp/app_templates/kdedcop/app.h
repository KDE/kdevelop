%{H_TEMPLATE}

#ifndef _%{APPNAMEUC}_H_
#define _%{APPNAMEUC}_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kmainwindow.h>

#include "mainclass.h"

/**
 * @short Application Main Window
 * @author %{AUTHOR} <%{EMAIL}>
 * @version 0.1
 */
class %{APPNAME} : public KMainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    %{APPNAME}();

    /**
     * Default Destructor
     */
    virtual ~%{APPNAME}();
private:
    MainClass *m_mainClass;
};

#endif // _%{APPNAMEUC}_H_
