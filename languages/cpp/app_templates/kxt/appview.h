
#ifndef _%{APPNAMEUC}VIEW_H_
#define _%{APPNAMEUC}VIEW_H_

#include <qwidget.h>

#include "%{APPNAME}view_base.h"

class QPainter;
class KURL;

/**
 * This is the main view class for %{APPNAME}.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */
class %{APPNAME}View : public %{APPNAME}view_base
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

#endif // _%{APPNAMEUC}VIEW_H_
