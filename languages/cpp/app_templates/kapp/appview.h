
#ifndef _%{APPNAMEUC}VIEW_H_
#define _%{APPNAMEUC}VIEW_H_

#include <qwidget.h>
#include <kparts/part.h>
#include <%{APPNAMELC}iface.h>

class QPainter;
class KURL;

/**
 * This is the main view class for %{APPNAME}.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * This %{APPNAMELC} uses an HTML component as an example.
 *
 * @short Main view
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */
class %{APPNAME}View : public QWidget, public %{APPNAME}Iface
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

    /**
     * Random 'get' function
     */
    QString currentURL();

    /**
     * Random 'set' function accessed by DCOP
     */
    virtual void openURL(QString url);

    /**
     * Random 'set' function
     */
    virtual void openURL(const KURL& url);

    /**
     * Print this view to any medium -- paper or not
     */
    void print(QPainter *, int height, int width);

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
    void slotOnURL(const QString& url);
    void slotSetTitle(const QString& title);

private:
    KParts::ReadOnlyPart *m_html;
};

#endif // _%{APPNAMEUC}VIEW_H_
