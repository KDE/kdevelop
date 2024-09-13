/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "filter.h"

#include <KConfigGroup>

#include <array>

using namespace KDevelop;

Filter::Filter()
    : targets(Files | Folders)
{
}

Filter::Filter(const SerializedFilter& filter)
    : pattern(QString(), Qt::CaseSensitive, QRegExp::WildcardUnix)
    , targets(filter.targets)
    , type(filter.type)
{
    QString pattern = filter.pattern;
    if (!filter.pattern.startsWith(QLatin1Char('/')) && !filter.pattern.startsWith(QLatin1Char('*'))) {
        // implicitly match against trailing relative path
        pattern.prepend(QLatin1String("*/"));
    }
    if (pattern.endsWith(QLatin1Char('/')) && targets != Filter::Files) {
        // implicitly match against folders
        targets = Filter::Folders;
        pattern.chop(1);
    }
    this->pattern.setPattern(pattern);
}

SerializedFilter::SerializedFilter()
    : targets(Filter::Files | Filter::Folders)

{

}

SerializedFilter::SerializedFilter(const QString& pattern, Filter::Targets targets, Filter::Type type)
    : pattern(pattern)
    , targets(targets)
    , type(type)
{

}

namespace KDevelop {

static SerializedFilters generateDefaultFilters()
{
    SerializedFilters ret;

    // filter hidden files
    ret << SerializedFilter(QStringLiteral(".*"), Filter::Targets(Filter::Files | Filter::Folders));
    // but do show some with special meaning

    const std::array configFiles = {
        // Version control
        QStringLiteral(".gitignore"),
        QStringLiteral(".gitmodules"),
        // https://pre-commit.com/
        QStringLiteral(".pre-commit-config.yaml"),

        // CI config files

        // https://docs.gitlab.com/ee/ci/yaml/
        QStringLiteral(".gitlab-ci.yml"),
        // https://travis-ci.org/
        QStringLiteral(".travis.yml"),
        // https://community.kde.org/Infrastructure/Continuous_Integration_System#The_.kde-ci.yml_file
        QStringLiteral(".kde-ci.yml"),

        // Code quality configs

        //   https://github.com/c4urself/bump2version/
        QStringLiteral(".bumpversion.cfg"),
        //   https://clang.llvm.org/docs/ClangFormat.html
        QStringLiteral(".clang-format"),
        //   https://github.com/rust-lang/rust-clippy#configure-the-behavior-of-some-lints
        QStringLiteral(".clippy.toml"),
        //   https://github.com/codespell-project/codespell#using-a-config-file
        QStringLiteral(".codespellrc"),
        //   https://editorconfig.org/
        QStringLiteral(".editorconfig"),
        //   https://pycqa.github.io/isort/docs/configuration/config_files.html
        QStringLiteral(".isort.cfg"),
        //   https://mypy.readthedocs.io/en/stable/config_file.html
        QStringLiteral(".mypy.ini"),
        //   https://pep8.readthedocs.io
        QStringLiteral(".pep8"),
        //   https://prettier.io/
        QStringLiteral(".prettierignore"),
        QStringLiteral(".prettierrc*"),
        //   https://www.pydocstyle.org/en/stable/usage.html#configuration-files
        QStringLiteral(".pydocstyle*"),
        //   https://pylint.readthedocs.io/en/stable/user_guide/usage/run.html#command-line-options
        QStringLiteral(".pylintrc"),
        //   https://docs.readthedocs.io/en/stable/config-file/v2.html
        QStringLiteral(".readthedocs.y*ml"),
        //   https://yamllint.readthedocs.io/en/stable/configuration.html#configuration
        QStringLiteral(".yamllint*"),

        // KDE's standard location of the flatpak recipe
        QStringLiteral(".flatpak-manifest.json"),
    };
    for (const QString& file : configFiles) {
        ret << SerializedFilter(file, Filter::Files, Filter::Inclusive);
    }

    const std::array configFolders = {
        // CI config folders

        // https://circleci.com/docs/
        QStringLiteral(".circleci"),
    };
    for (const QString& folder : configFolders) {
        ret << SerializedFilter(folder, Filter::Folders, Filter::Inclusive);
    }

    // common vcs folders which we want to hide
    const std::array invalidFolders = {
        QStringLiteral(".git"), QStringLiteral("CVS"), QStringLiteral(".svn"), QStringLiteral("_svn"),
        QStringLiteral("SCCS"), QStringLiteral("_darcs"), QStringLiteral(".hg"), QStringLiteral(".bzr"), QStringLiteral("__pycache__")
    };
    for (const QString& folder : invalidFolders) {
        ret << SerializedFilter(folder, Filter::Folders);
    }

    // common files which we want to hide
    const std::array filePatterns = {
        // binary files (Unix)
        QStringLiteral("*.o"), QStringLiteral("*.a"), QStringLiteral("*.so"), QStringLiteral("*.so.*"),
        // binary files (Windows)
        QStringLiteral("*.obj"), QStringLiteral("*.lib"), QStringLiteral("*.dll"), QStringLiteral("*.exp"), QStringLiteral("*.pdb"),
        // generated files
        QStringLiteral("moc_*.cpp"), QStringLiteral("*.moc"), QStringLiteral("ui_*.h"), QStringLiteral("*.qmlc"), QStringLiteral("qrc_*.cpp"),
        // backup files
        QStringLiteral("*~"), QStringLiteral("*.orig"), QStringLiteral(".*.kate-swp"), QStringLiteral(".*.swp"),
        // python cache and object files
        QStringLiteral("*.pyc"), QStringLiteral("*.pyo")
    };
    for (const QString& filePattern : filePatterns) {
        ret << SerializedFilter(filePattern, Filter::Files);
    }

    return ret;
}

SerializedFilters defaultFilters()
{
    static const SerializedFilters ret = generateDefaultFilters();
    return ret;
}

SerializedFilters readFilters(const KSharedConfigPtr& config)
{
    if (!config->hasGroup(QStringLiteral("Filters"))) {
        return defaultFilters();
    }
    const KConfigGroup& group = config->group(QStringLiteral("Filters"));
    const int size = group.readEntry("size", -1);
    if (size == -1) {
        // fallback
        return defaultFilters();
    }

    SerializedFilters filters;
    filters.reserve(size);
    for (int i = 0; i < size; ++i) {
        const QString subGroup = QString::number(i);
        if (!group.hasGroup(subGroup)) {
            continue;
        }
        const KConfigGroup& subConfig = group.group(subGroup);
        const QString pattern = subConfig.readEntry("pattern", QString());
        Filter::Targets targets(subConfig.readEntry("targets", 0));
        Filter::Type type = static_cast<Filter::Type>(subConfig.readEntry("inclusive", 0));
        filters << SerializedFilter(pattern, targets, type);
    }

    return filters;
}

void writeFilters(const SerializedFilters& filters, KSharedConfigPtr config)
{
    // clear existing
    config->deleteGroup(QStringLiteral("Filters"));

    // write new
    KConfigGroup group = config->group(QStringLiteral("Filters"));
    group.writeEntry("size", filters.size());
    int i = 0;
    for (const SerializedFilter& filter : filters) {
        KConfigGroup subGroup = group.group(QString::number(i++));
        subGroup.writeEntry("pattern", filter.pattern);
        subGroup.writeEntry("targets", static_cast<int>(filter.targets));
        subGroup.writeEntry("inclusive", static_cast<int>(filter.type));
    }
    config->sync();
}

Filters deserialize(const SerializedFilters& filters)
{
    Filters ret;
    ret.reserve(filters.size());
    for (const SerializedFilter& filter : filters) {
        ret << Filter(filter);
    }
    return ret;
}

}
