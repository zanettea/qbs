/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef QBS_LOGGER_H
#define QBS_LOGGER_H

#include "ilogsink.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QVariant;
QT_END_NAMESPACE

namespace qbs {
namespace Internal {

class LogWriter
{
public:
    LogWriter(ILogSink *logSink, LoggerLevel level);

    // log writer has move semantics and the last instance of
    // a << chain prints the accumulated data
    LogWriter(const LogWriter &other);
    ~LogWriter();
    const LogWriter &operator=(const LogWriter &other);

    void write(char c);
    void write(const char *str);
    void write(const QChar &c);
    void write(const QString &message);

    void setMessageTag(const QString &tag);

private:
    ILogSink *m_logSink;
    LoggerLevel m_level;
    mutable QString m_message;
    QString m_tag;
};

class MessageTag
{
public:
    explicit MessageTag(const QString &tag) : m_tag(tag) {}

    const QString &tag() const { return m_tag; }

private:
    QString m_tag;
};

LogWriter operator<<(LogWriter w, const char *str);
LogWriter operator<<(LogWriter w, const QByteArray &byteArray);
LogWriter operator<<(LogWriter w, const QString &str);
LogWriter operator<<(LogWriter w, const QStringList &strList);
LogWriter operator<<(LogWriter w, const QSet<QString> &strSet);
LogWriter operator<<(LogWriter w, const QVariant &variant);
LogWriter operator<<(LogWriter w, int n);
LogWriter operator<<(LogWriter w, qint64 n);
LogWriter operator<<(LogWriter w, bool b);
LogWriter operator<<(LogWriter w, const MessageTag &tag);

class Logger
{
public:
    Logger(ILogSink *logSink = 0);

    ILogSink *logSink() const { return m_logSink; }

    bool debugEnabled() const;
    bool traceEnabled() const;

    LogWriter qbsLog(LoggerLevel level) const;
    LogWriter qbsWarning() const { return qbsLog(LoggerWarning); }
    LogWriter qbsInfo() const { return qbsLog(LoggerInfo); }
    LogWriter qbsDebug() const { return qbsLog(LoggerDebug); }
    LogWriter qbsTrace() const { return qbsLog(LoggerTrace); }

private:
    ILogSink *m_logSink;
};


class TimedActivityLogger
{
public:
    TimedActivityLogger(const Logger &logger, const QString &activity,
                        const QString &prefix = QString(), LoggerLevel logLevel = LoggerDebug);
    void finishActivity();
    ~TimedActivityLogger();

private:
    class TimedActivityLoggerPrivate;
    TimedActivityLoggerPrivate *d;
};

} // namespace Internal
} // namespace qbs

#endif // QBS_LOGGER_H
