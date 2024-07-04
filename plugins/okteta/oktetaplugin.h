/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OKTETAPLUGIN_H
#define OKTETAPLUGIN_H

// KDev
#include <interfaces/iplugin.h>
// Qt
#include <QVariantList>

namespace Kasten
{
class ByteArrayViewProfileManager;
}

namespace KDevelop
{
class OktetaDocumentFactory;


class OktetaPlugin: public IPlugin
{
    Q_OBJECT

  public:
      explicit OktetaPlugin(QObject* parent, const KPluginMetaData& metaData,
                            const QVariantList& args = QVariantList());

      ~OktetaPlugin() override;

  public: // KDevelop::IPlugin API
    ContextMenuExtension contextMenuExtension(Context* context, QWidget* parent) override;

  public:
    Kasten::ByteArrayViewProfileManager* viewProfileManager() const { return mViewProfileManager; }

  private Q_SLOTS:
    void onOpenTriggered();

  protected:
    OktetaDocumentFactory* mDocumentFactory;
    Kasten::ByteArrayViewProfileManager* const mViewProfileManager;
};

}
#endif
