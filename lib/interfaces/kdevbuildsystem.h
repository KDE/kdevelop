/**
 * The interface to the build system
 */

#ifndef _KDEVBUILDSYSTEM_H_
#define _KDEVBUILDSYSTEM_H_

#include <qobject.h>

#include <kurl.h>

#include <qmap.h>
#include <qvariant.h>
#include <qvaluelist.h>
#include <qwidget.h>
#include <qdom.h>

class BuildFileItem;
class BuildGroupItem;
class BuildTargetItem;
class KDialogBase;

class BuildBaseItem
{
public:
    enum
    {
	Group,
	Target,
	File
    };

public:
    BuildBaseItem( int type, BuildBaseItem* parent=0 );
    virtual ~BuildBaseItem();

    virtual int type() const { return m_type; }
    virtual QString name() const { return m_name; }

    BuildBaseItem* parent() { return m_parent; }
    const BuildBaseItem* parent() const { return m_parent; }

    const QMap<QString, QVariant>& attributes() const { return m_attributes; }

    bool hasAttribute( const QString& name ) const { return m_attributes.contains( name ); }
    QVariant attribute( const QString& name ) const { return m_attributes[ name ]; }
    void setAttribute( const QString& name, const QVariant& value ) { m_attributes[ name ] = value; }

    virtual QString path();

protected:
    QString m_name;

private:
    int m_type;
    BuildBaseItem* m_parent;
    QMap<QString, QVariant> m_attributes;

private:
    BuildBaseItem( const BuildBaseItem& source );
    void operator = ( const BuildBaseItem& source );
};

class BuildGroupItem: public BuildBaseItem
{
public:
    BuildGroupItem( const QString& name, BuildGroupItem* parent=0 );
    virtual ~BuildGroupItem();

    BuildGroupItem* parentGroup() { return m_parentGroup; }
    const BuildGroupItem* parentGroup() const { return m_parentGroup; }

    void insertGroup( BuildGroupItem* group );
    void removeGroup( BuildGroupItem* group );
    BuildGroupItem* takeGroup( BuildGroupItem* group );
    QValueList<BuildGroupItem*> groups() { return m_subGroups; }

    void insertTarget( BuildTargetItem* target );
    void removeTarget( BuildTargetItem* target );
    BuildTargetItem* takeTarget( BuildTargetItem* target );
    QValueList<BuildTargetItem*> targets() { return m_targets; }

private:
    BuildGroupItem* m_parentGroup;
    QValueList<BuildGroupItem*> m_subGroups;
    QValueList<BuildTargetItem*> m_targets;

private:
    BuildGroupItem( const BuildGroupItem& source );
    void operator = ( const BuildGroupItem& source );
};

class BuildTargetItem: public BuildBaseItem
{
public:
    BuildTargetItem( const QString& name, BuildGroupItem* parent );
    virtual ~BuildTargetItem();

    BuildGroupItem* parentGroup() { return m_parentGroup; }
    const BuildGroupItem* parentGroup() const { return m_parentGroup; }

    void insertFile( BuildFileItem* file );
    void removeFile( BuildFileItem* file );
    BuildFileItem* takeFile( BuildFileItem* file );
    QValueList<BuildFileItem*> files() { return m_files; }

private:
    BuildGroupItem* m_parentGroup;
    QValueList<BuildFileItem*> m_files;

private:
    BuildTargetItem( const BuildTargetItem& source );
    void operator = ( const BuildTargetItem& source );
};

class BuildFileItem: public BuildBaseItem
{
public:
    BuildFileItem( const KURL& url, BuildTargetItem* parentTarget );
    virtual ~BuildFileItem();

    BuildTargetItem* parentTarget() { return m_parentTarget; }
    const BuildTargetItem* parentTarget() const { return m_parentTarget; }

    const KURL& url() const { return m_url; }
    virtual QString name() const { return m_url.fileName(); }

private:
    KURL m_url;
    BuildTargetItem* m_parentTarget;

private:
    BuildFileItem( const BuildFileItem& source );
    void operator = ( const BuildFileItem& source );
};

class BuildItemConfigWidget: public QWidget{
    Q_OBJECT
public:
    BuildItemConfigWidget(BuildBaseItem *it, QWidget *parent = 0, const char *name = 0);
public slots:
    virtual void accept();
};

class ProjectConfigTab: public QWidget{
public:
    ProjectConfigTab(QWidget *parent = 0, const char *name = 0);
    virtual QDomDocument configuration() = 0;
};

class KDevBuildSystem : public QObject
{
    Q_OBJECT

public:
    KDevBuildSystem( QObject *parent=0, const char *name=0 );
    virtual ~KDevBuildSystem();

    /**the name of the build system for using in project configuration files*/
    virtual QString buildSystemName() = 0;
    /**reimplement this to create your build system like makefiles ant's xml
       files or shell scripts to build the currently selected project item
       (like target or group)*/
    virtual void createBuildSystem(BuildBaseItem*) = 0;

    /**implement this to allow building of files, targets and groups*/
    virtual void build(BuildBaseItem* it = 0) = 0;

    /**implement this to allow installing of targets and groups*/
    virtual void install(BuildBaseItem* it = 0) = 0;

    /**implement this to allow executing of targets and groups*/
    virtual void execute(BuildBaseItem* it = 0) = 0;

    /**implement this to allow cleaning of targets and groups*/
    virtual void clean(BuildBaseItem* it = 0) = 0;

    /**reimplement to show configure build item dialog*/
    virtual void configureBuildItem(KDialogBase *, BuildBaseItem*);
    /**reimplement to include project configuration widgets
       that will be included in Project|Project Options|Configure Options
       all those widgets will go to the tab bar below the configuration
       profiles widget*/
    virtual QValueList<ProjectConfigTab *> confWidgets() = 0;

    /**this provides a simple propery editor for the given build item
       if the build system don't want to provide its own configuration
       widget (there is no need to reimplement this)*/
    //TODO: adymo: implement
    virtual void addDefaultBuildWidget(KDialogBase *dlg, QWidget *parent, BuildBaseItem*);
};


#endif
