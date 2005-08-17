%{H_TEMPLATE}

#ifndef _%{APPNAMEUC}PART_H_
#define _%{APPNAMEUC}PART_H_

#include <kparts/part.h>
#include <kparts/factory.h>

class Q3MultiLineEdit;
class QWidget;
class QPainter;
class KURL;
class KAboutData;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author %{AUTHOR}
 * @version %{VERSION}
 */
class %{APPNAMELC}Part : public KParts::ReadWritePart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    %{APPNAMELC}Part(QWidget *parentWidget, const char *widgetName,
                    QObject *parent, const char *name, const QStringList & /*args*/);

    /**
     * Destructor
     */
    virtual ~%{APPNAMELC}Part();

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

    /**
     * About data for the part
     */
    static KAboutData *createAboutData();
	    
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

#endif // _%{APPNAMEUC}PART_H_
