#ifndef TESTSTUBS_PLUGINSTUB_H
#define TESTSTUBS_PLUGINSTUB_H

#include "interfaces/iplugin.h"
#include "interfaces/contextmenuextension.h"
#include "../coverageexport.h"

namespace TestStubs
{
class VERITAS_COVERAGE_EXPORT Plugin : public KDevelop::IPlugin
{
Q_OBJECT
public:
    Plugin(const KComponentData &instance, QObject *parent) : KDevelop::IPlugin(instance, parent) {}
    virtual ~Plugin() {}
    virtual bool isCentralPlugin() const { Q_ASSERT(0); return true; }
    virtual void unload() { Q_ASSERT(0); }
    KIconLoader* iconLoader() const { Q_ASSERT(0); return 0; }
    KDevelop::ICore *core() const { Q_ASSERT(0); return 0; }
    virtual KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* context ) { return KDevelop::ContextMenuExtension(); }

/*public Q_SLOTS:
    void newIconLoader() const;

protected:
    void addExtension( const QString& );
    virtual void initializeGuiState();*/
};
}

#endif
