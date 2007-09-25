%{H_TEMPLATE}
#ifndef %{APPNAMELC}VIEW_H
#define %{APPNAMELC}VIEW_H

#include <QtGui/QWidget>

#include "ui_%{APPNAMELC}view_base.h"

class QPainter;
class KUrl;

/**
 * This is the main view class for %{APPNAME}.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */

class %{APPNAME}View : public QWidget, public Ui::%{APPNAMELC}view_base
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    %{APPNAME}View(QWidget *parent);

    /**
     * Destructor
     */
    virtual ~%{APPNAME}View();

private:
    Ui::%{APPNAMELC}view_base ui_%{APPNAMELC}view_base;

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

private slots:
    void switchColors();
    void settingsChanged();
};

#endif // %{APPNAME}VIEW_H
