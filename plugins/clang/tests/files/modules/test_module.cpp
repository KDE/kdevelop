/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

module;

export module test.mod;

import module_example;
import test.submodule;

export namespace module_example::detail {
    class Module : public module_example::Module {
    public:
        Module();
        virtual ~Module() noexcept;

        // implementation of the parent type
        void copy(const module_example::Module& rhs) override;

        auto getSubModule() const -> const module_example::SubModule& override;
        auto getSubModule() -> module_example::SubModule& override;
        void setSubModule(const module_example::SubModule& sub) override;
    private:
        module_example::detail::SubModule m_sub;
    };
} // namespace module_example

extern "C++" {
    namespace module_example {
        Module::Module() = default;
        Module::~Module() noexcept = default;
        Module::Module(const Module&) = default;
        auto Module::operator=(const Module& rhs) -> Module& = default;
        Module::Module(Module&&) noexcept = default;
        auto Module::operator=(Module&& rhs) noexcept -> Module& = default;

        auto Module::create() -> Module& {
            return *new detail::Module();
        }

        auto Module::create(const Module& rhs) -> Module& {
            auto& cloned = create();
            cloned.copy(rhs);
            return cloned;
        }

        void Module::destroy(Module& rhs) {
            delete dynamic_cast<detail::Module*>(&rhs);
        }
    } // namespace module_example
}

namespace module_example {
    void detail::Module::copy(const module_example::Module& rhs) {
        m_sub.copy(rhs.getSubModule());
    }

    detail::Module::Module() = default;
    detail::Module::~Module() noexcept = default;

    auto detail::Module::getSubModule() const -> const module_example::SubModule& {
        return m_sub;
    }

    auto detail::Module::getSubModule() -> module_example::SubModule& {
        return m_sub;
    }

    void detail::Module::setSubModule(const module_example::SubModule& sub) {
        m_sub.copy(sub);
    }
} // namespace module_example
