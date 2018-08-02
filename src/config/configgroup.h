/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CLANGTIDY_CONFIG_H
#define CLANGTIDY_CONFIG_H

#include <KConfig>
#include <KConfigGroup>

#include <array>
#include <tuple>

namespace ClangTidy
{
/**
 * \class
 * \brief Specializes KConfigGroup for clang-tidy, using a type system to avoid
 * configuration control by passing
 * strings around.
 */
class ConfigGroup
{
    //\typedef Typedef for pair of QStrings.
    using Option = std::pair<QString, QString>;
    ///\typedef definition of an array of options.
    using OptionArray = std::array<const Option*, 9>;

private:
    KConfigGroup m_group;

public:
    ConfigGroup() = default;
    ConfigGroup(const KConfigGroup& cg) { m_group = cg; }
    ~ConfigGroup() = default;
    ConfigGroup& operator=(const KConfigGroup& cg)
    {
        this->m_group = cg;
        return *this;
    }
    bool isValid() { return m_group.isValid(); }
    /// This technique allows us to avoid passing strings to KConfigGroup member
    /// functions, thus allowing compile time
    /// detection of wrong parameters.
    static const Option FilePath;
    static const Option BuildPath;
    static const Option AdditionalParameters;
    static const Option EnabledChecks;
    static const Option UseConfigFile;
    static const Option DumpConfig;
    static const Option ExportFixes;
    static const Option HeaderFilter;
    static const Option CheckSystemHeaders;

    static const OptionArray m_allOptions;

    QString readEntry(const Option& key) const { return m_group.readEntry(key.first); }

    template <typename T>
    void writeEntry(const Option& key, const T& value, KConfig::WriteConfigFlags pFlags = KConfig::Normal)
    {
        m_group.writeEntry(key.first, key.second.arg(value), pFlags);
    }

    void deleteEntry(const Option& key, KConfig::WriteConfigFlags pFlags = KConfig::Normal)
    {
        m_group.deleteEntry(key.first, pFlags);
    }

    void enableEntry(const Option& key, bool enable)
    {
        enable ? writeEntry(key, QString()) : deleteEntry(key);
    }
};
}

#endif // CLANGTIDY_CONFIG_H
