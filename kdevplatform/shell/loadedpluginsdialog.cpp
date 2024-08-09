/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "loadedpluginsdialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QListView>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

#include <KAboutData>
#include <KAboutPluginDialog>
#include <KLocalizedString>
#include <KTitleWidget>
#include <KWidgetItemDelegate>

#include <util/scopeddialog.h>

#include "core.h"
#include "plugincontroller.h"

#define MARGIN 5

namespace {

KPluginMetaData pluginInfo(KDevelop::IPlugin* plugin)
{
    return KDevelop::Core::self()->pluginControllerInternal()->pluginInfo(plugin);
}

QString displayName(KDevelop::IPlugin* plugin)
{
    const auto name = pluginInfo(plugin).name();
    return !name.isEmpty() ? name : plugin->componentName();
}

bool sortPlugins(KDevelop::IPlugin* l, KDevelop::IPlugin* r)
{
    return displayName(l) < displayName(r);
}

}

class PluginsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ExtraRoles {
        DescriptionRole = Qt::UserRole+1
    };
    explicit PluginsModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
        m_plugins = KDevelop::Core::self()->pluginControllerInternal()->loadedPlugins();
        std::sort(m_plugins.begin(), m_plugins.end(), sortPlugins);
    }

    KDevelop::IPlugin *pluginForIndex(const QModelIndex& index) const
    {
        if (!index.isValid()) return nullptr;
        if (index.parent().isValid()) return nullptr;
        if (index.column() != 0) return nullptr;
        if (index.row() >= m_plugins.count()) return nullptr;
        return m_plugins[index.row()];
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        KDevelop::IPlugin* plugin = pluginForIndex(index);
        if (!plugin)
            return QVariant();

        switch (role) {
        case Qt::DisplayRole:
            return displayName(plugin);
        case DescriptionRole:
            return pluginInfo(plugin).description();
        case Qt::DecorationRole: {
                const QString iconName = pluginInfo(plugin).iconName();
                if (iconName.isEmpty()) {
                    return QStringLiteral("kdevelop");
                }
                return iconName;
            }
        default:
            return QVariant();
        };
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (!parent.isValid()) {
            return m_plugins.count();
        }
        return 0;
    }

private:
    QList<KDevelop::IPlugin*> m_plugins;
};

class LoadedPluginsDelegate : public KWidgetItemDelegate
{
    Q_OBJECT

public:

    explicit LoadedPluginsDelegate(QAbstractItemView *itemView, QObject *parent = nullptr)
        : KWidgetItemDelegate(itemView, parent)
        , pushButton(new QPushButton)
    {
        pushButton->setIcon(QIcon::fromTheme(QStringLiteral("help-about-symbolic"))); // only for getting size matters
    }

    ~LoadedPluginsDelegate() override
    {
        delete pushButton;
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(index);

        int i = 5;
        int j = 1;

        const int iconSize = QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize);
        QFont font = titleFont(option.font);
        QFontMetrics fmTitle(font);

        return QSize(
            iconSize + MARGIN * i + pushButton->sizeHint().width() * j,
            qMax(iconSize + MARGIN * 2, fmTitle.height() + option.fontMetrics.height() + MARGIN * 2)
        );

    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        if (!index.isValid()) {
            return;
        }

        painter->save();

        QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);

        int iconSize = option.rect.height() - MARGIN * 2;
        QIcon icon = QIcon::fromTheme(index.model()->data(index, Qt::DecorationRole).toString());
        icon.paint(painter, QRect(dependantLayoutValue(MARGIN + option.rect.left(), iconSize, option.rect.width()), MARGIN + option.rect.top(), iconSize, iconSize));
        QRect contentsRect(dependantLayoutValue(MARGIN * 2 + iconSize + option.rect.left(), option.rect.width() - MARGIN * 3 - iconSize, option.rect.width()), MARGIN + option.rect.top(), option.rect.width() - MARGIN * 3 - iconSize, option.rect.height() - MARGIN * 2);

        int lessHorizontalSpace = MARGIN * 2 + pushButton->sizeHint().width();

        contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.highlightedText().color());
        }

        if (itemView()->layoutDirection() == Qt::RightToLeft) {
            contentsRect.translate(lessHorizontalSpace, 0);
        }

        painter->save();

        painter->save();
        QFont font = titleFont(option.font);
        QFontMetrics fmTitle(font);
        painter->setFont(font);
        painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignTop, fmTitle.elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, contentsRect.width()));
        painter->restore();

        painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignBottom, option.fontMetrics.elidedText(index.model()->data(index, PluginsModel::DescriptionRole).toString(), Qt::ElideRight, contentsRect.width()));

        painter->restore();
        painter->restore();
    }

    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override
    {
        Q_UNUSED(index);

        auto *button = new QPushButton();
        button->setIcon(QIcon::fromTheme(QStringLiteral("help-about-symbolic")));
        setBlockedEventTypes(button, QList<QEvent::Type>{QEvent::MouseButtonPress, QEvent::MouseButtonRelease, QEvent::MouseButtonDblClick});

        connect(button, &QPushButton::clicked, this, &LoadedPluginsDelegate::info);
        return QList<QWidget*>()
            << button;
    }

    void updateItemWidgets(const QList<QWidget*>& widgets, const QStyleOptionViewItem& option,
                           const QPersistentModelIndex& index) const override
    {
        Q_UNUSED(index);
        if (widgets.isEmpty()) {
            return;
        }

        auto *aboutPushButton = static_cast<QPushButton*>(widgets[0]);
        QSize aboutPushButtonSizeHint = aboutPushButton->sizeHint();
        aboutPushButton->resize(aboutPushButtonSizeHint);
        aboutPushButton->move(dependantLayoutValue(option.rect.width() - MARGIN - aboutPushButtonSizeHint.width(), aboutPushButtonSizeHint.width(), option.rect.width()), option.rect.height() / 2 - aboutPushButtonSizeHint.height() / 2);
    }

    int dependantLayoutValue(int value, int width, int totalWidth) const
    {
        if (itemView()->layoutDirection() == Qt::LeftToRight) {
            return value;
        }
        return totalWidth - width - value;
    }

    QFont titleFont(const QFont &baseFont) const
    {
        QFont retFont(baseFont);
        retFont.setBold(true);
        return retFont;
    }

private Q_SLOTS:
    void info()
    {
        auto *m = static_cast<PluginsModel*>(itemView()->model());
        KDevelop::IPlugin *p = m->pluginForIndex(focusedIndex());
        if (p) {
            const KPluginMetaData pluginInfo = ::pluginInfo(p);
            if (!pluginInfo.name().isEmpty()) { // Be sure the about data is not completely empty
                KDevelop::ScopedDialog<KAboutPluginDialog> aboutPlugin(pluginInfo, itemView());
                aboutPlugin->exec();
                return;
            }
        }
    }
private:
    QPushButton *pushButton;
};

class PluginsView : public QListView
{
    Q_OBJECT
public:
    explicit PluginsView(QWidget* parent = nullptr)
        :QListView(parent)
    {
        setModel(new PluginsModel(this));
        setItemDelegate(new LoadedPluginsDelegate(this));
        setVerticalScrollMode(QListView::ScrollPerPixel);
    }

    ~PluginsView() override
    {
        // explicitly delete the delegate here since otherwise
        // we get spammed by warnings that the QPushButton we return
        // in createItemWidgets is deleted before the delegate
        // *sigh* - even dfaure says KWidgetItemDelegate is a crude hack
        delete itemDelegate();
    }

    QSize sizeHint() const override
    {
        QSize ret = QListView::sizeHint();
        ret.setWidth(qMax(ret.width(), sizeHintForColumn(0) + 30));
        return ret;
    }
};

LoadedPluginsDialog::LoadedPluginsDialog( QWidget* parent )
    : QDialog( parent )
{
    setWindowTitle(i18nc("@title:window", "Loaded Plugins"));

    auto* vbox = new QVBoxLayout(this);

    auto* title = new KTitleWidget(this);
    title->setIcon(qApp->windowIcon(), KTitleWidget::ImageLeft);
    title->setText(i18n("<html><font size=\"4\">Plugins loaded for <b>%1</b></font></html>",
                        KAboutData::applicationData().displayName()));
    vbox->addWidget(title);
    vbox->addWidget(new PluginsView());

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &LoadedPluginsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &LoadedPluginsDialog::reject);
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    vbox->addWidget(buttonBox);

    resize(800, 600);
}

#include "moc_loadedpluginsdialog.cpp"
#include "loadedpluginsdialog.moc"
