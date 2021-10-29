/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IDEFINESANDINCLUDES_H
#define IDEFINESANDINCLUDES_H

#include <QHash>
#include <QPointer>
#include <QString>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>

#include <util/path.h>

namespace KDevelop
{
class ProjectBaseItem;
class IProject;

using Defines = QHash<QString, QString>;

/** An interface that provides language plugins with include directories/files and defines.
* Call IDefinesAndIncludesManager::manager() to get the instance of the plugin.
**/
class IDefinesAndIncludesManager
{
public:
    /// The type of includes/defines
    enum Type {
        CompilerSpecific = 1, ///< Those that compiler provides
        ProjectSpecific = 2, ///< Those that project manager provides
        UserDefined = 4,    ///< Those that user defines
        All = CompilerSpecific | ProjectSpecific | UserDefined
     };

    /**
     * Class that actually does all the work of calculating i/d.
     *
     * Implement one in e.g. project manager and register it with @see registerProvider
     * To unregister it use @see unregisterProvider
     *
     * @sa BackgroundProvider
    **/
    class Provider
    {
    public:
        virtual ~Provider() = default;

        virtual Defines defines( const QString& path ) const = 0;
        virtual Defines defines( ProjectBaseItem* item ) const = 0;

        virtual Path::List includes( const QString& path ) const = 0;
        virtual Path::List includes( ProjectBaseItem* item ) const = 0;

        virtual Path::List frameworkDirectories( const QString& path ) const = 0;
        virtual Path::List frameworkDirectories( ProjectBaseItem* item ) const = 0;

        /// @return the type of i/d this provider provides
        virtual Type type() const = 0;
    };

    /**
     * Use this as base class for provider, if computing includes/defines can takes a long time (e.g. parsing Makefile by running make).
     *
     * This provider will be queried for includes/defines in a background thread.
     *
     * @sa Provider
    **/
    class BackgroundProvider
    {
    public:
        virtual ~BackgroundProvider() = default;

        virtual Path::List includesInBackground( const QString& path ) const = 0;

        virtual Path::List frameworkDirectoriesInBackground( const QString& path ) const = 0;

        virtual Defines definesInBackground( const QString& path ) const = 0;

        virtual QString parserArgumentsInBackground(const QString& path) const = 0;

        /// @return the type of i/d this provider provides
        virtual Type type() const = 0;
    };

    ///@param item project item
    ///@param type Data sources to be used.
    ///@return list of defines for @p item
    ///NOTE: call it from the foreground thread only.
    virtual Defines defines( ProjectBaseItem* item, Type type = All ) const = 0;

    ///@param item project item
    ///@param type Data sources to be used.
    ///@return list of include directories/files for @p item
    ///NOTE: call it from the foreground thread only.
    virtual Path::List includes( ProjectBaseItem* item, Type type = All ) const = 0;

    ///@param item project item
    ///@param type Data sources to be used.
    ///@return list of framework directories for @p item
    ///NOTE: call it from the foreground thread only.
    virtual Path::List frameworkDirectories( ProjectBaseItem* item, Type type = All ) const = 0;

    ///@param path path to an out-of-project file.
    ///@param type Data sources to be used.
    ///@return list of defines for @p path
    ///NOTE: call it from the foreground thread only.
    virtual Defines defines( const QString& path, Type type = All ) const = 0;

    ///@param path path to an out-of-project file.
    ///@param type Data sources to be used.
    ///@return list of include directories/files for @p path
    ///NOTE: call it from the foreground thread only.
    virtual Path::List includes( const QString& path, Type type = All ) const = 0;

    ///@param path path to an out-of-project file.
    ///@param type Data sources to be used.
    ///@return list of framework directories for @p path
    ///NOTE: call it from the foreground thread only.
    virtual Path::List frameworkDirectories( const QString& path, Type type = All ) const = 0;

    /**
     * Computes include directories in background thread.
     *
     * This is especially useful for CustomMake projects.
     *
     * Call it from background thread if possible.
    **/
    virtual Path::List includesInBackground( const QString& path ) const = 0;

    /**
     * Computes framework directories in background thread.
     *
     * This is especially useful for CustomMake projects.
     *
     * Call it from background thread if possible.
    **/
    virtual Path::List frameworkDirectoriesInBackground( const QString& path ) const = 0;

    /**
     * Computes defined macros in background thread.
     *
     * Call it from background thread if possible.
    **/
    virtual Defines definesInBackground( const QString& path ) const = 0;

    /**
     * @param item project item. Use nullptr to get default arguments
     * @return The parser command-line arguments used to parse the @p item
     */
    virtual QString parserArguments(ProjectBaseItem* item) const = 0;

    virtual QString parserArguments(const QString& path) const = 0;
    virtual QString parserArgumentsInBackground(const QString& path) const = 0;

    ///@return the instance of the plugin.
    inline static IDefinesAndIncludesManager* manager();

    virtual ~IDefinesAndIncludesManager() = default;

    /**
     * Register the @p provider
     */
    virtual void registerProvider(Provider* provider) = 0;

    /**
     * Unregister the provider
     *
     * @return true on success, false otherwise (e.g. if not registered)
     */
    virtual bool unregisterProvider(Provider* provider) = 0;

    /**
     * Use this to register the background provider
     *
     * This provider will be queried for includes/defines in a background thread.
     */
    virtual void registerBackgroundProvider(BackgroundProvider* provider) = 0;

    /**
     * Unregister the background provider.
     *
     * @sa registerBackgroundProvider
     */
    virtual bool unregisterBackgroundProvider(BackgroundProvider* provider) = 0;

    /// Opens a configuration dialog for @p pathToFile to modify include directories/files and defined macros.
    virtual void openConfigurationDialog(const QString& pathToFile) = 0;
};

inline IDefinesAndIncludesManager* IDefinesAndIncludesManager::manager()
{
    static QPointer<IPlugin> manager;
    if (!manager) {
        manager = ICore::self()->pluginController()->pluginForExtension( QStringLiteral("org.kdevelop.IDefinesAndIncludesManager") );
    }
    Q_ASSERT(manager);

    auto extension = manager->extension<IDefinesAndIncludesManager>();
    return extension;
}

}

Q_DECLARE_INTERFACE( KDevelop::IDefinesAndIncludesManager, "org.kdevelop.IDefinesAndIncludesManager" )

#endif
