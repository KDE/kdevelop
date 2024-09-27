/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPQTDOC_H
#define QTHELPQTDOC_H

#include "qthelpproviderabstract.h"

class QtHelpQtDoc : public QtHelpProviderAbstract
{
    Q_OBJECT
public:
    explicit QtHelpQtDoc(QObject* parent, const QString& qmake, const QString& collectionFileName);
    ~QtHelpQtDoc();

    QIcon icon() const override;
    QString name() const override;
    void registerDocumentations();
    void loadDocumentation();
    void unloadDocumentation();

    /**
     * @return whether any .qch files exist in the @c QT_INSTALL_DOCS directory
     */
    bool isQtHelpAvailable() const;

    /** @return @c true once the async initialization has finished */
    bool isInitialized() const { return m_isInitialized; }

    /** @return available qmake binaries */
    static QStringList qmakeCandidates();

Q_SIGNALS:
    /** emitted when isInitialized() is set to @c true */
    void isInitializedChanged();

private:
    /**
     * Calls @p processQchFilePath(QString absoluteFilePath) for every
     * .qch file in each @c QT_INSTALL_DOCS (sub)directory.
     *
     * @pre @a !m_path.isEmpty()
     * @param processQchFilePath a function object that returns @c true if done,
     *        that is, when the iteration over .qch files should be aborted.
     * @return the return value of the last call to @p processQchFilePath or
     *         @c false if @p processQchFilePath was never called.
     */
    template<typename ProcessQchFilePath>
    bool visitQchFiles(ProcessQchFilePath processQchFilePath) const;

    QString m_qmake;
    QString m_path; ///< absolute path to the @c QT_INSTALL_DOCS directory
    bool m_isInitialized = false;
};

#endif // QTHELPQTDOC_H
