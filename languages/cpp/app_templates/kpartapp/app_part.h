
#ifndef _%{APPNAMEUC}PART_H_
#define _%{APPNAMEUC}PART_H_

#include <kparts/part.h>
#include <kparts/factory.h>

class QWidget;
class QPainter;
class KURL;
class Q3MultiLineEdit;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */
class %{APPNAME}Part : public KParts::ReadWritePart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    %{APPNAME}Part(QWidget *parentWidget, const char *widgetName,
                    QObject *parent, const char *name);

    /**
     * Destructor
     */
    virtual ~%{APPNAME}Part();

    /**
     * This is a virtual function inherited from KParts::ReadWritePart.  
     * A shell will use this to inform this Part if it should act
     * read-only
     */
    virtual void setReadWrite(bool rw);

    /**
     * Reimplemented to disable and enable Save action
     */
    virtual void setModified(bool modified);

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();

    /**
     * This must be implemented by each read-write part
     */
    virtual bool saveFile();

protected slots:
    void fileOpen();
    void fileSaveAs();

private:
    Q3MultiLineEdit *m_widget;
};

class KInstance;
class KAboutData;

class %{APPNAME}PartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    %{APPNAME}PartFactory();
    virtual ~%{APPNAME}PartFactory();
    virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
                                            QObject *parent, const char *name,
                                            const char *classname, const QStringList &args );
    static KInstance* instance();
 
private:
    static KInstance* s_instance;
    static KAboutData* s_about;
};

#endif // _%{APPNAMEUC}PART_H_
