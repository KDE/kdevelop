/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

module;

export module test.submodule;

import std;
import module_example;

export namespace module_example::detail {
    class SubModule : public module_example::SubModule {
    public:
        SubModule();
        virtual ~SubModule() noexcept;
        // Rule of 5
        SubModule(const SubModule&);
        auto operator=(const SubModule& rhs) -> SubModule&;
        SubModule(SubModule&&) noexcept;
        auto operator=(SubModule&& rhs) noexcept -> SubModule&;

        // implementation of the parent type
        void copy(const module_example::SubModule& rhs) override;
        [[nodiscard]] auto getString() -> std::string& override;
        [[nodiscard]] auto getString() const -> const std::string& override;
        auto enableString() -> std::string& override;
        auto hasString() const -> bool override;
        void setString(const std::string& str) override;
    private:
        std::optional<std::string> m_str {std::nullopt};
    };
} // namespace module_example

extern "C++" {
    namespace module_example {
        SubModule::SubModule() = default;
        SubModule::SubModule(const SubModule&) = default;
        auto SubModule::operator=(const SubModule& rhs) -> SubModule& = default;
        SubModule::SubModule(SubModule&&) noexcept = default;
        auto SubModule::operator=(SubModule&& rhs) noexcept -> SubModule& = default;

        auto SubModule::create() -> SubModule& {
            return *new detail::SubModule();
        }

        auto SubModule::create(const SubModule& rhs) -> SubModule& {
            auto& cloned = create();
            cloned.copy(rhs);
            return cloned;
        }

        void SubModule::destroy(SubModule& rhs) {
            delete dynamic_cast<detail::SubModule*>(&rhs);
        }
    } // namespace module_example
}

namespace module_example {
    void detail::SubModule::copy(const module_example::SubModule& rhs) {
        if (rhs.hasString()) {
            m_str = rhs.getString();
        } else {
            m_str = std::nullopt;
        }
    }

    detail::SubModule::SubModule() = default;
    detail::SubModule::~SubModule() noexcept = default;
    detail::SubModule::SubModule(const SubModule&) = default;
    auto detail::SubModule::operator=(const SubModule& rhs) -> SubModule& = default;
    detail::SubModule::SubModule(SubModule&&) noexcept = default;
    auto detail::SubModule::operator=(SubModule&& rhs) noexcept -> SubModule& = default;

    auto detail::SubModule::getString() -> std::string& {
        return m_str.value();
    }

    auto detail::SubModule::getString() const -> const std::string& {
        return m_str.value();
    }

    auto detail::SubModule::enableString() -> std::string& {
        if (!m_str.has_value()) {
            m_str = std::string();
        }
        return m_str.value();
    }

    auto detail::SubModule::hasString() const -> bool {
        return m_str.has_value();
    }

    void detail::SubModule::setString(const std::string& str) {
        m_str = str;
    }
} // namespace module_example
