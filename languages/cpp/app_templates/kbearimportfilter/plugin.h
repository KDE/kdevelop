 
#ifndef __KBEAR_%{APPNAMEUC}_IMPORT_FILTER_H__
#define __KBEAR_%{APPNAMEUC}_IMPORT_FILTER_H__

//////////////////////////////////////////////////////////////////////
// Qt specific include files
#include <qstring.h>
//////////////////////////////////////////////////////////////////////
// Application specific include files
#include <kbear/kbearsiteimportfilterpluginiface.h>

class QDomDocument;

class %{APPNAME}ImportFilterPlugin : public KBear::SiteImportFilterPluginIface
{
    Q_OBJECT
public:
    %{APPNAME}ImportFilterPlugin(QObject *parent=0, const char *name=0, const QStringList &args = QStringList());
    virtual ~%{APPNAME}ImportFilterPlugin();

    /**
     * This function should return a @ref QDomDocument containing the imported bookmarks.
     * The document should have the format:
     * <pre>
     *    group label="APPNAME-imports"
     *        group label="group1"
     *            site label="site1"
     *            ............
     *            /site
     *            .................
     *        /group
     *        ...................
     *    /group
     *    ..................
     *</pre>
     * @return the @ref QDomDocument s @ref QString representation containing the imported bookmarks.
     */
    virtual QString getDomDocument();

    /**
     * Function should start the import
     * @param fileName is the full path to the file to import
     */
    virtual void import( const QString& fileName );

    /**
     * This function is used by the caller to find out whether or not
     * the import was successfull. If anything went wrong during
     * import this function should return true.
     * This stop update of database.
     * @return false om succes or true on error.
     */
    virtual bool hasError() const { return m_hasError; }
private:
    QDomDocument m_domDocument;
    bool m_hasError;
};

#endif // __KBEAR_%{APPNAMEUC}_IMPORT_FILTER_H__

