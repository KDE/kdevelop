/**
 * This is the abstract base class which encapsulates everything
 * necessary for communicating with version control systems.
 */

#ifndef _KDEVVERSIONCONTROL_H_
#define _KDEVVERSIONCONTROL_H_

#include <qobject.h>
#include <qstring.h>
#include <qwidget.h>
#include <qmap.h>

#include "kdevplugin.h"


class KDevVersionControl : public KDevPlugin
{
    Q_OBJECT

public:
    enum State { invalid = -1, canBeCommited = 1, canBeAdded = 2 };

	/**
	* Builds the object.
	* 	@p pluginName	this _is_ *** unique *** identintifier between all available version controls.
	*   @p icon			symbolic name for the icon
	*	@p parent		the parent QObject for this version control system
	*	@p name			a name for this object; it may be null since it is usefull for debugging
	*/
	KDevVersionControl( const QString& pluginName, const QString& icon, QObject *parent, const char *name=0 );
	/**
	* Destructor. Removes current VCS from the list of available ones.
	*/
	~KDevVersionControl();

	/**
	* Return a pointer to the associated configuration widget for this vcs.
	*/
	virtual QWidget* newProjectWidget(QWidget */*parent*/) { return 0; };
	/**
	* Creates a new project in the passed path @p dir.
	*/
	virtual void createNewProject(const QString&/* dir*/) { return; }
    /**
     * Tells whether the given file is registered
     * in the version control system.
	 * Note: I see no need for it since KDevVersionControl implementations
	 * currently handle this internally and there is no request for external entities
	 * to access this. (marioS, 13-may-2003)
     */
     //    This will be replaced by a smarter solution
     //    virtual State registeredState(const QString &fileName) = 0;
	/**
	* Returns the unique identifier for this plugin (so it can be used for retrieving
	* it from collections).
	*/
	QString uid() const;
};

#endif
