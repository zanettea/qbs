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

#include <app/qbs/parser/commandlineparser.h>
#include <app/shared/logging/consolelogger.h>
#include <app/shared/qbssettings.h>
#include <tools/buildoptions.h>
#include <tools/error.h>
#include <tools/fileinfo.h>
#include <tools/hostosinfo.h>
#include <tools/profile.h>
#include <tools/settings.h>
#include <QDir>
#include <QTemporaryFile>
#include <QtTest>

using namespace qbs;
using namespace Internal;

static SettingsPtr settings = qbsSettings();

class TestTools : public QObject
{
    Q_OBJECT
public:
    TestTools()
    {
        ConsoleLogger::instance().logSink()->setEnabled(false);
    }

private slots:
    void testValidCommandLine()
    {
        QTemporaryFile projectFile;
        QVERIFY(projectFile.open());
        const QStringList fileArgs = QStringList() << "-f" << projectFile.fileName();
        QStringList args;
        args.append("-vvk");
        args.append("-v");
        args << "--products" << "blubb";
        args << "--changed-files" << "foo,bar" << fileArgs;
        args << "--force";
        CommandLineParser parser;

        QVERIFY(parser.parseCommandLine(args, settings.data()));
        QCOMPARE(ConsoleLogger::instance().logSink()->logLevel(), LoggerTrace);
        QCOMPARE(parser.command(), BuildCommandType);
        QCOMPARE(parser.products(), QStringList() << "blubb");
        QCOMPARE(parser.buildOptions().changedFiles.count(), 2);
        QVERIFY(parser.buildOptions().keepGoing);
        QVERIFY(parser.force());

        QVERIFY(parser.parseCommandLine(QStringList() << "-vvvqqq" << fileArgs, settings.data()));
        QCOMPARE(ConsoleLogger::instance().logSink()->logLevel(), defaultLogLevel());
        QVERIFY(!parser.force());

        QVERIFY(parser.parseCommandLine(QStringList() << "-vvqqq" << fileArgs, settings.data()));
        QCOMPARE(ConsoleLogger::instance().logSink()->logLevel(), LoggerWarning);

        QVERIFY(parser.parseCommandLine(QStringList() << "-vvvqq" << fileArgs, settings.data()));
        QCOMPARE(ConsoleLogger::instance().logSink()->logLevel(), LoggerDebug);

        QVERIFY(parser.parseCommandLine(QStringList() << "--log-level" << "trace" << fileArgs,
                                        settings.data()));
        QCOMPARE(ConsoleLogger::instance().logSink()->logLevel(), LoggerTrace);
    }

    void testInvalidCommandLine()
    {
        QTemporaryFile projectFile;
        QVERIFY(projectFile.open());
        const QStringList fileArgs = QStringList() << "-f" << projectFile.fileName();
        CommandLineParser parser;
        QVERIFY(!parser.parseCommandLine(QStringList() << "-x" << fileArgs, settings.data())); // Unknown short option.
        QVERIFY(!parser.parseCommandLine(QStringList() << "--xyz" << fileArgs, settings.data())); // Unknown long option.
        QVERIFY(!parser.parseCommandLine(QStringList() << "-vjv" << fileArgs, settings.data())); // Invalid position.
        QVERIFY(!parser.parseCommandLine(QStringList() << "-j" << fileArgs, settings.data()));  // Missing argument.
        QVERIFY(!parser.parseCommandLine(QStringList() << "-j" << "0" << fileArgs,
                                         settings.data())); // Wrong argument.
        QVERIFY(!parser.parseCommandLine(QStringList() << "--products" << fileArgs,
                                         settings.data()));  // Missing argument.
        QVERIFY(!parser.parseCommandLine(QStringList() << "--changed-files" << "," << fileArgs,
                                         settings.data())); // Wrong argument.
        QVERIFY(!parser.parseCommandLine(QStringList() << "--log-level" << "blubb" << fileArgs,
                                         settings.data())); // Wrong argument.
        QVERIFY(!parser.parseCommandLine(QStringList("properties") << fileArgs << "--force",
                settings.data())); // Invalid option for command.
    }

    void testFileInfo()
    {
        QCOMPARE(FileInfo::fileName("C:/waffl/copter.exe"), QString("copter.exe"));
        QCOMPARE(FileInfo::baseName("C:/waffl/copter.exe.lib"), QString("copter"));
        QCOMPARE(FileInfo::completeBaseName("C:/waffl/copter.exe.lib"), QString("copter.exe"));
        QCOMPARE(FileInfo::path("abc"), QString("."));
        QCOMPARE(FileInfo::path("/abc/lol"), QString("/abc"));
        QVERIFY(!FileInfo::isAbsolute("bla/lol"));
        QVERIFY(FileInfo::isAbsolute("/bla/lol"));
        if (HostOsInfo::isWindowsHost())
            QVERIFY(FileInfo::isAbsolute("C:\\bla\\lol"));
        QCOMPARE(FileInfo::resolvePath("/abc/lol", "waffl"), QString("/abc/lol/waffl"));
        QCOMPARE(FileInfo::resolvePath("/abc/def/ghi/jkl/", "../foo/bar"), QString("/abc/def/ghi/foo/bar"));
        QCOMPARE(FileInfo::resolvePath("/abc/def/ghi/jkl/", "../../foo/bar"), QString("/abc/def/foo/bar"));
        QCOMPARE(FileInfo::resolvePath("/abc", "../../../foo/bar"), QString("/foo/bar"));
        QCOMPARE(FileInfo("/does/not/exist").lastModified(), FileTime());
    }

    void testProjectFileLookup()
    {
        const QString srcDir = QLatin1String(SRCDIR);
        const QString noProjectsDir = srcDir + QLatin1String("data/dirwithnoprojects");
        const QString oneProjectDir = srcDir + QLatin1String("data/dirwithoneproject");
        const QString multiProjectsDir = srcDir + QLatin1String("data/dirwithmultipleprojects");
        Q_ASSERT(QDir(noProjectsDir).exists() && QDir(oneProjectDir).exists()
                && QDir(multiProjectsDir).exists());
        CommandLineParser parser;
        const QStringList args(QLatin1String("-f"));
        QString projectFilePath = multiProjectsDir + QLatin1String("/project.qbs");
        QVERIFY(parser.parseCommandLine(args + QStringList(projectFilePath), settings.data()));
        QCOMPARE(projectFilePath, parser.projectFilePath());
        projectFilePath = oneProjectDir + QLatin1String("/project.qbs");
        QVERIFY(parser.parseCommandLine(args + QStringList(oneProjectDir), settings.data()));
        QCOMPARE(projectFilePath, parser.projectFilePath());
        QVERIFY(!parser.parseCommandLine(args + QStringList(noProjectsDir), settings.data()));
        QVERIFY(!parser.parseCommandLine(args + QStringList(multiProjectsDir), settings.data()));
    }

    void testProfiles()
    {
        bool exceptionCaught;
        Profile parentProfile("parent", settings.data());
        Profile childProfile("child", settings.data());
        try {
            parentProfile.removeBaseProfile();
            parentProfile.remove("testKey");
            QCOMPARE(parentProfile.value("testKey", "none").toString(), QLatin1String("none"));
            parentProfile.setValue("testKey", "testValue");
            QCOMPARE(parentProfile.value("testKey").toString(), QLatin1String("testValue"));

            childProfile.remove("testKey");
            childProfile.removeBaseProfile();
            QCOMPARE(childProfile.value("testKey", "none").toString(), QLatin1String("none"));
            childProfile.setBaseProfile("blubb");
            QCOMPARE(childProfile.value("testKey", "none").toString(), QLatin1String("none"));
            childProfile.setBaseProfile("parent");
            QCOMPARE(childProfile.value("testKey").toString(), QLatin1String("testValue"));
            childProfile.setBaseProfile("foo");
            QCOMPARE(childProfile.value("testKey", "none").toString(), QLatin1String("none"));
            exceptionCaught = false;
        } catch (Error &) {
            exceptionCaught = true;
        }
        QVERIFY(!exceptionCaught);

        try {
            childProfile.setBaseProfile("parent");
            parentProfile.setBaseProfile("child");
            QVERIFY(!childProfile.value("blubb").isValid());
            exceptionCaught = false;
        } catch (Error &) {
            exceptionCaught = true;
        }
        QVERIFY(exceptionCaught);

        try {
            QVERIFY(!childProfile.allKeys(Profile::KeySelectionNonRecursive).isEmpty());
            exceptionCaught = false;
        } catch (Error &) {
            exceptionCaught = true;
        }
        QVERIFY(!exceptionCaught);

        try {
            QVERIFY(!childProfile.allKeys(Profile::KeySelectionRecursive).isEmpty());
            exceptionCaught = false;
        } catch (Error &) {
            exceptionCaught = true;
        }
        QVERIFY(exceptionCaught);
    }
};

QTEST_MAIN(TestTools)

#include "tst_tools.moc"
