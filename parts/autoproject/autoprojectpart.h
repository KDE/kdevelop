/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
*                                                                         *
*   Copyright (C) 2002 by Victor Röder                                    *
*   victor_roeder@gmx.de                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _AUTOPROJECTPART_H_
#define _AUTOPROJECTPART_H_

#include <qdict.h>
#include <qguardedptr.h>
#include <qmap.h>
#include <qdatetime.h>
#include <kgenericfactory.h>

#include "kdevproject.h"

class QDomElement;
class QStringList;
class KDialogBase;
class AutoProjectWidget;
class KSelectAction;
class TargetItem;


class AutoProjectPart : public KDevProject
{
    Q_OBJECT

public:
    AutoProjectPart( QObject *parent, const char *name, const QStringList &args );
    ~AutoProjectPart();

    /**
     * Implementation of the KDevProject interface.
     */
    virtual QString projectDirectory();
    virtual QString projectName();
    virtual QString mainProgram();
    virtual QString activeDirectory();
    virtual QStringList allFiles();
    virtual void addFile(const QString &fileName);
    virtual void addFiles ( const QStringList& fileList );
    virtual void removeFile(const QString &fileName);
    virtual void removeFiles ( const QStringList& fileList );
    virtual QString buildDirectory();
	
	/**
	 * Implementation of the KDevPlugin interface.
	 */
	virtual void restorePartialProjectSession ( const QDomElement* el );
	virtual void savePartialProjectSession ( QDomElement* el );

    /**
     * automake specific methods.
     */
    QStringList allBuildConfigs();
    QString currentBuildConfig();
    QString topsourceDirectory();
    void startMakeCommand(const QString &dir, const QString &target, bool withKdesu = false);
    void buildTarget(QString relpath, TargetItem* titem);

    void needMakefileCvs();
    bool isDirty();

protected:
    /**
     * Reimplemented from KDevProject. These methods are only
     * for use by the application core.
     */
    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();

private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void slotAddTranslation();
    void slotBuild();
    void slotBuildActiveTarget();
    void slotCompileFile();
    void slotClean();
    void slotDistClean();
    void slotInstall();
    void slotInstallWithKdesu();
    void slotMakefilecvs();
    void slotMakeMessages();
    void slotConfigure();
    void slotExecute();
    void slotBuildConfigChanged(const QString &config);
    void slotBuildConfigAboutToShow();
    void slotCommandFinished( const QString& command );
    //void slotImportExisting();
    
private:
    QGuardedPtr<AutoProjectWidget> m_widget;
    QString m_projectName;
    QString m_projectPath;
    KSelectAction *buildConfigAction;
    QString makeEnvironment();
    void setWantautotools();
    QString makefileCvsCommand();
    QString configureCommand();
    QMap<QString, QDateTime> m_timestamp;
    bool m_executeAfterBuild;
    QString m_buildCommand;
    bool m_needMakefileCvs;

    // Enble AutoProjectWidget to emit our signals
    friend class AutoProjectWidget;
    friend class AddSubprojectDialog;

    // For make commands queueing
    QString constructMakeCommandLine(const QString &dir, const QString &target);
    void queueInternalLibDependenciesBuild(TargetItem* titem);
};

typedef KGenericFactory<AutoProjectPart> AutoProjectFactory;

#endif
