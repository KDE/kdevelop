/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SIMPLE_TOOL_VIEW_FACTORY_H
#define KDEVPLATFORM_SIMPLE_TOOL_VIEW_FACTORY_H

#include <interfaces/iuicontroller.h>

#include <QPointer>
#include <QString>

#include <utility>

namespace KDevelop {

class SimpleToolViewFactoryBase : public IToolViewFactory
{
public:
    explicit SimpleToolViewFactoryBase(QString id, Qt::DockWidgetArea defaultPosition)
        : m_id{std::move(id)}
        , m_defaultPosition{defaultPosition}
    {
    }

    [[nodiscard]] QString id() const override
    {
        return m_id;
    }

    [[nodiscard]] Qt::DockWidgetArea defaultPosition() const override
    {
        return m_defaultPosition;
    }

private:
    const QString m_id;
    const Qt::DockWidgetArea m_defaultPosition;
};

template<typename Data, typename StoredData = Data*>
class SimpleDataToolViewFactoryBase : public SimpleToolViewFactoryBase
{
public:
    explicit SimpleDataToolViewFactoryBase(Data* data, QString id, Qt::DockWidgetArea defaultPosition)
        : SimpleToolViewFactoryBase(std::move(id), defaultPosition)
        , m_data{data}
    {
    }

protected:
    [[nodiscard]] Data* data() const
    {
        return m_data;
    }

private:
    const StoredData m_data;
};

template<class Widget, typename Data, typename StoredData = Data*>
class SimpleDataToolViewFactory : public SimpleDataToolViewFactoryBase<Data, StoredData>
{
public:
    using SimpleDataToolViewFactoryBase<Data, StoredData>::SimpleDataToolViewFactoryBase;

    [[nodiscard]] QWidget* create(QWidget* parent = nullptr) override
    {
        return new Widget(this->data(), parent);
    }
};

template<class Widget, typename Data>
using SimpleGuardedDataToolViewFactory = SimpleDataToolViewFactory<Widget, Data, QPointer<Data>>;

} // namespace KDevelop

#endif // KDEVPLATFORM_SIMPLE_TOOL_VIEW_FACTORY_H
