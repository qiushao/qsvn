#include "ExternalToolCommand.h"
#include "CommandLine.h"
#include "SvnClient.h"

#include <QtTest/QtTest>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>

class SvnClientTests : public QObject {
    Q_OBJECT

private slots:
    void parseCommandLineReadsActions();
    void parseCommandLineReportsMissingPath();
    void buildExternalToolCommandReplacesPlaceholders();
    void buildExternalToolCommandAppendsDefaultArgumentsWithoutPlaceholders();
    void buildArgumentsAddsAuthenticationOptions();
    void parseStatusReadsCodesAndPaths();
    void parseLogXmlReadsEntriesAndChangedPaths();
    void parsePropertiesXmlReadsNamesAndValues();
    void runHandlesLocalRepositoryWorkflow();
    void runHandlesUpdateToRevisionWorkflow();
    void runHandlesCheckRepositoryWorkflow();
    void runHandlesBranchSwitchAndMergeWorkflow();
    void runHandlesPatchWorkflow();
    void runHandlesRevisionDiffWorkflow();
    void runHandlesRevisionBlameWorkflow();
    void runHandlesImportExportWorkflow();
    void runHandlesRelocateWorkflow();
    void runHandlesPropertyWorkflow();
    void runHandlesIgnoreWorkflow();
    void runHandlesCopyWorkflow();
    void runHandlesDeleteWorkflow();
    void runHandlesRenameWorkflow();
    void runHandlesLockWorkflow();
    void runHandlesRemoteRepositoryEditingWorkflow();
    void runHandlesChangelistWorkflow();
};

void SvnClientTests::parseCommandLineReadsActions()
{
    CommandLineRequest request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--diff"), QStringLiteral("/tmp/wc/file.txt")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Diff));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/file.txt"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--update"), QStringLiteral("/tmp/wc")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Update));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--update-revision"), QStringLiteral("/tmp/wc")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::UpdateRevision));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--export"), QStringLiteral("/tmp/wc")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Export));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--check-repository"), QStringLiteral("/tmp/wc")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::CheckRepository));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--import"), QStringLiteral("/tmp/project")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Import));
    QCOMPARE(request.path, QStringLiteral("/tmp/project"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--commit"), QStringLiteral("/tmp/wc")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Commit));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--add"), QStringLiteral("/tmp/wc/new.txt")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Add));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/new.txt"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--ignore"), QStringLiteral("/tmp/wc/generated.tmp")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Ignore));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/generated.tmp"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--copy"), QStringLiteral("/tmp/wc/file.txt")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Copy));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/file.txt"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--delete"), QStringLiteral("/tmp/wc/old.txt")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Delete));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/old.txt"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--rename"), QStringLiteral("/tmp/wc/file.txt")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Rename));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/file.txt"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--revert"), QStringLiteral("/tmp/wc/file.txt")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Revert));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/file.txt"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--lock"), QStringLiteral("/tmp/wc/file.txt")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Lock));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/file.txt"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--unlock"), QStringLiteral("/tmp/wc/file.txt")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Unlock));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/file.txt"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--cleanup"), QStringLiteral("/tmp/wc")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Cleanup));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--properties"), QStringLiteral("/tmp/wc/file.txt")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Properties));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc/file.txt"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--conflicts"), QStringLiteral("/tmp/wc")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Conflicts));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc"));

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--repo-browser")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::RepoBrowser));
    QCOMPARE(request.path, QString());

    request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("/tmp/wc")});
    QVERIFY(request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Open));
    QCOMPARE(request.path, QStringLiteral("/tmp/wc"));
}

void SvnClientTests::parseCommandLineReportsMissingPath()
{
    const CommandLineRequest request = parseCommandLine({QStringLiteral("qsvn"), QStringLiteral("--log")});
    QVERIFY(!request.ok());
    QCOMPARE(static_cast<int>(request.action), static_cast<int>(CommandLineAction::Log));
    QVERIFY(!request.error.isEmpty());
}

void SvnClientTests::buildExternalToolCommandReplacesPlaceholders()
{
    const ExternalToolCommand command = buildExternalToolCommand(QStringLiteral("kdiff3 {base} {mine} {theirs} -o {working}"),
                                                                {
                                                                    {QStringLiteral("base"), QStringLiteral("/tmp/base")},
                                                                    {QStringLiteral("mine"), QStringLiteral("/tmp/mine")},
                                                                    {QStringLiteral("theirs"), QStringLiteral("/tmp/theirs")},
                                                                    {QStringLiteral("working"), QStringLiteral("/tmp/working")},
                                                                },
                                                                {});

    QVERIFY(command.isValid());
    QCOMPARE(command.program, QStringLiteral("kdiff3"));
    QCOMPARE(command.arguments, QStringList({QStringLiteral("/tmp/base"),
                                             QStringLiteral("/tmp/mine"),
                                             QStringLiteral("/tmp/theirs"),
                                             QStringLiteral("-o"),
                                             QStringLiteral("/tmp/working")}));
    QVERIFY(command.usedPlaceholder);
}

void SvnClientTests::buildExternalToolCommandAppendsDefaultArgumentsWithoutPlaceholders()
{
    const ExternalToolCommand command = buildExternalToolCommand(QStringLiteral("meld"),
                                                                {
                                                                    {QStringLiteral("base"), QStringLiteral("/tmp/base")},
                                                                    {QStringLiteral("working"), QStringLiteral("/tmp/working")},
                                                                },
                                                                {QStringLiteral("/tmp/base"), QStringLiteral("/tmp/working")});

    QVERIFY(command.isValid());
    QCOMPARE(command.program, QStringLiteral("meld"));
    QCOMPARE(command.arguments, QStringList({QStringLiteral("/tmp/base"), QStringLiteral("/tmp/working")}));
    QVERIFY(!command.usedPlaceholder);
}

void SvnClientTests::buildArgumentsAddsAuthenticationOptions()
{
    SvnClient client;
    SvnOptions options;
    options.username = QStringLiteral("alice");
    options.password = QStringLiteral("secret");
    options.useAuthCache = false;
    options.trustServerCertificate = true;
    client.setOptions(options);

    const QStringList arguments = client.buildArguments({
        QStringLiteral("checkout"),
        QStringLiteral("https://example.com/svn/trunk"),
        QStringLiteral("wc"),
    });

    QCOMPARE(arguments.at(0), QStringLiteral("checkout"));
    QCOMPARE(arguments.at(1), QStringLiteral("--non-interactive"));
    QCOMPARE(arguments.at(arguments.indexOf(QStringLiteral("--username")) + 1), QStringLiteral("alice"));
    QCOMPARE(arguments.at(arguments.indexOf(QStringLiteral("--password")) + 1), QStringLiteral("secret"));
    QVERIFY(arguments.contains(QStringLiteral("--no-auth-cache")));
    QVERIFY(arguments.contains(QStringLiteral("--trust-server-cert")));
}

void SvnClientTests::parseStatusReadsCodesAndPaths()
{
    SvnClient client;

    const QVector<SvnStatus> statuses = client.parseStatus(QStringLiteral(
        "M       src/main.cpp\n"
        "C       conflicted.txt\n"
        "--- Changelist 'review':\n"
        "?       file with spaces.txt\n"
        "      > moved from old-name.txt\n"
        "        > moved to new-name.txt\n"
        "        *        1   remote-only.txt\n"
        "M       *        2   modified-remote.txt\n"
        "?       123.txt\n"
        "Status against revision:      3\n"));

    QCOMPARE(statuses.size(), 6);
    QCOMPARE(statuses.at(0).code, QStringLiteral("M"));
    QCOMPARE(statuses.at(0).path, QStringLiteral("src/main.cpp"));
    QCOMPARE(statuses.at(1).code, QStringLiteral("C"));
    QCOMPARE(statuses.at(1).path, QStringLiteral("conflicted.txt"));
    QCOMPARE(statuses.at(2).code, QStringLiteral("?"));
    QCOMPARE(statuses.at(2).path, QStringLiteral("file with spaces.txt"));
    QCOMPARE(statuses.at(3).code, QStringLiteral("*"));
    QCOMPARE(statuses.at(3).path, QStringLiteral("remote-only.txt"));
    QCOMPARE(statuses.at(4).code, QStringLiteral("M *"));
    QCOMPARE(statuses.at(4).path, QStringLiteral("modified-remote.txt"));
    QCOMPARE(statuses.at(5).code, QStringLiteral("?"));
    QCOMPARE(statuses.at(5).path, QStringLiteral("123.txt"));
}

void SvnClientTests::parseLogXmlReadsEntriesAndChangedPaths()
{
    SvnClient client;

    const QVector<SvnLogEntry> entries = client.parseLogXml(QStringLiteral(
        "<?xml version=\"1.0\"?>"
        "<log>"
        "<logentry revision=\"3\">"
        "<author>alice</author>"
        "<date>2026-06-27T12:00:00.000000Z</date>"
        "<paths>"
        "<path action=\"M\">/trunk/file.txt</path>"
        "<path action=\"A\">/branches/feature</path>"
        "</paths>"
        "<msg>merge branch</msg>"
        "</logentry>"
        "</log>"));

    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.at(0).revision, QStringLiteral("3"));
    QCOMPARE(entries.at(0).author, QStringLiteral("alice"));
    QCOMPARE(entries.at(0).date, QStringLiteral("2026-06-27T12:00:00.000000Z"));
    QCOMPARE(entries.at(0).message, QStringLiteral("merge branch"));
    QCOMPARE(entries.at(0).changedPaths.size(), 2);
    QCOMPARE(entries.at(0).changedPaths.at(0), QStringLiteral("M  /trunk/file.txt"));
    QCOMPARE(entries.at(0).changedPaths.at(1), QStringLiteral("A  /branches/feature"));
}

void SvnClientTests::parsePropertiesXmlReadsNamesAndValues()
{
    SvnClient client;

    const QVector<SvnProperty> properties = client.parsePropertiesXml(QStringLiteral(
        "<?xml version=\"1.0\"?>"
        "<properties>"
        "<target path=\"file.txt\">"
        "<property name=\"custom:test\">hello</property>"
        "<property name=\"svn:ignore\">build\n*.o</property>"
        "</target>"
        "</properties>"));

    QCOMPARE(properties.size(), 2);
    QCOMPARE(properties.at(0).name, QStringLiteral("custom:test"));
    QCOMPARE(properties.at(0).value, QStringLiteral("hello"));
    QCOMPARE(properties.at(1).name, QStringLiteral("svn:ignore"));
    QCOMPARE(properties.at(1).value, QStringLiteral("build\n*.o"));
}

void SvnClientTests::runHandlesLocalRepositoryWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("first\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::Append | QIODevice::Text));
    QVERIFY(file.write("second\n") > 0);
    file.close();

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    const QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 1);
    QCOMPARE(statuses.at(0).code, QStringLiteral("M"));
    QCOMPARE(statuses.at(0).path, QStringLiteral("file.txt"));
}

void SvnClientTests::runHandlesUpdateToRevisionWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    const QString filePath = QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("first\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::Append | QIODevice::Text));
    QVERIFY(file.write("second\n") > 0);
    file.close();

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("second"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("update"), QStringLiteral("-r"), QStringLiteral("1"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString::fromUtf8(file.readAll()), QStringLiteral("first\n"));
    file.close();

    result = client.run({QStringLiteral("update"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString::fromUtf8(file.readAll()), QStringLiteral("first\nsecond\n"));
    file.close();
}

void SvnClientTests::runHandlesCheckRepositoryWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString firstWorkingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc1"));
    const QString secondWorkingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc2"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, firstWorkingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(firstWorkingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("first\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, firstWorkingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, firstWorkingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("checkout"), repositoryUrl, secondWorkingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::Append | QIODevice::Text));
    QVERIFY(file.write("second\n") > 0);
    file.close();

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("second"), QStringLiteral("file.txt")}, firstWorkingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("status"), QStringLiteral("-u")}, secondWorkingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 1);
    QCOMPARE(statuses.at(0).code, QStringLiteral("*"));
    QCOMPARE(statuses.at(0).path, QStringLiteral("file.txt"));

    result = client.run({QStringLiteral("update")}, secondWorkingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("status"), QStringLiteral("-u")}, secondWorkingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 0);
}

void SvnClientTests::runHandlesBranchSwitchAndMergeWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();
    const QString trunkUrl = repositoryUrl + QStringLiteral("/trunk");
    const QString branchUrl = repositoryUrl + QStringLiteral("/branches/feature");

    SvnResult result = client.run({
        QStringLiteral("mkdir"),
        trunkUrl,
        repositoryUrl + QStringLiteral("/branches"),
        repositoryUrl + QStringLiteral("/tags"),
        QStringLiteral("-m"),
        QStringLiteral("layout"),
    });
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("checkout"), trunkUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    const QString filePath = QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("trunk\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial trunk"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("copy"), trunkUrl, branchUrl, QStringLiteral("-m"), QStringLiteral("create branch")});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("switch"), branchUrl}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::Append | QIODevice::Text));
    QVERIFY(file.write("branch\n") > 0);
    file.close();

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("branch change"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("switch"), trunkUrl}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("merge"), branchUrl}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    const QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    bool foundMergedFile = false;
    for (const SvnStatus &status : statuses) {
        if (status.code == QStringLiteral("M") && status.path == QStringLiteral("file.txt")) {
            foundMergedFile = true;
            break;
        }
    }
    QVERIFY(foundMergedFile);
}

void SvnClientTests::runHandlesPatchWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));
    const QString patchPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("change.patch"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    const QString filePath = QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("first\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::Append | QIODevice::Text));
    QVERIFY(file.write("second\n") > 0);
    file.close();

    result = client.run({QStringLiteral("diff"), QStringLiteral("--internal-diff"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("+second")));

    QFile patchFile(patchPath);
    QVERIFY(patchFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(patchFile.write(result.standardOutput.toLocal8Bit()) > 0);
    patchFile.close();

    result = client.run({QStringLiteral("revert"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("patch"), patchPath}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    const QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 1);
    QCOMPARE(statuses.at(0).code, QStringLiteral("M"));
    QCOMPARE(statuses.at(0).path, QStringLiteral("file.txt"));
}

void SvnClientTests::runHandlesRevisionDiffWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("first\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::Append | QIODevice::Text));
    QVERIFY(file.write("second\n") > 0);
    file.close();

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("second"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("diff"), QStringLiteral("--internal-diff"), QStringLiteral("-c"), QStringLiteral("2"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("+second")));
    QVERIFY(result.standardOutput.contains(QStringLiteral("file.txt")));
}

void SvnClientTests::runHandlesRevisionBlameWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("first\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::Append | QIODevice::Text));
    QVERIFY(file.write("second\n") > 0);
    file.close();

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("second"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("blame"), QStringLiteral("-r"), QStringLiteral("2"), repositoryUrl + QStringLiteral("/file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("first")));
    QVERIFY(result.standardOutput.contains(QStringLiteral("second")));
}

void SvnClientTests::runHandlesImportExportWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString importPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("import-src"));
    const QString exportPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("exported"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    QVERIFY(QDir().mkpath(importPath));
    QFile importedFile(QDir(importPath).absoluteFilePath(QStringLiteral("imported.txt")));
    QVERIFY(importedFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(importedFile.write("imported\n") > 0);
    importedFile.close();

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();
    const QString importedUrl = repositoryUrl + QStringLiteral("/imported");

    SvnResult result = client.run({QStringLiteral("import"), importPath, importedUrl, QStringLiteral("-m"), QStringLiteral("import project")});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("list"), importedUrl});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("imported.txt")));

    result = client.run({QStringLiteral("export"), importedUrl, exportPath});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(QFileInfo::exists(QDir(exportPath).absoluteFilePath(QStringLiteral("imported.txt"))));
    QVERIFY(!QFileInfo::exists(QDir(exportPath).absoluteFilePath(QStringLiteral(".svn"))));
}

void SvnClientTests::runHandlesRelocateWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString relocatedRepositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo-moved"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("content\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("hotcopy"), repositoryPath, relocatedRepositoryPath}), 0);

    const QString relocatedRepositoryUrl = QUrl::fromLocalFile(relocatedRepositoryPath).toString();
    result = client.run({QStringLiteral("relocate"), relocatedRepositoryUrl, workingCopyPath}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("info"), QStringLiteral("--show-item"), QStringLiteral("url")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QCOMPARE(result.standardOutput.trimmed(), relocatedRepositoryUrl);
}

void SvnClientTests::runHandlesPropertyWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("content\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("propset"), QStringLiteral("custom:test"), QStringLiteral("hello"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("proplist"), QStringLiteral("--xml"), QStringLiteral("-v"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    const QVector<SvnProperty> properties = client.parsePropertiesXml(result.standardOutput);
    QCOMPARE(properties.size(), 1);
    QCOMPARE(properties.at(0).name, QStringLiteral("custom:test"));
    QCOMPARE(properties.at(0).value, QStringLiteral("hello"));

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 1);
    QCOMPARE(statuses.at(0).code, QStringLiteral("M"));
    QCOMPARE(statuses.at(0).path, QStringLiteral("file.txt"));

    result = client.run({QStringLiteral("propdel"), QStringLiteral("custom:test"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 0);
}

void SvnClientTests::runHandlesIgnoreWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile ignoredFile(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("ignored.tmp")));
    QVERIFY(ignoredFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(ignoredFile.write("generated\n") > 0);
    ignoredFile.close();

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("ignored.tmp")));

    result = client.run({QStringLiteral("propset"), QStringLiteral("svn:ignore"), QStringLiteral("ignored.tmp\n"), workingCopyPath}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("propget"), QStringLiteral("svn:ignore"), workingCopyPath}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("ignored.tmp")));

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(!result.standardOutput.contains(QStringLiteral("ignored.tmp")));
}

void SvnClientTests::runHandlesCopyWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("content\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("copy"), QStringLiteral("file.txt"), QStringLiteral("copy.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(QFileInfo(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt"))).exists());
    QVERIFY(QFileInfo(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("copy.txt"))).exists());

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    const QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 1);
    QCOMPARE(statuses.at(0).path, QStringLiteral("copy.txt"));
    QVERIFY(statuses.at(0).code.startsWith('A'));
    QVERIFY(statuses.at(0).code.contains('+'));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("copy file")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("list"), repositoryUrl}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("file.txt")));
    QVERIFY(result.standardOutput.contains(QStringLiteral("copy.txt")));
}

void SvnClientTests::runHandlesDeleteWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("old.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("old\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("old.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("old.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("delete"), QStringLiteral("old.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(!QFileInfo(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("old.txt"))).exists());

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    const QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 1);
    QCOMPARE(statuses.at(0).code, QStringLiteral("D"));
    QCOMPARE(statuses.at(0).path, QStringLiteral("old.txt"));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("delete old"), QStringLiteral("old.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("list"), repositoryUrl}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(!result.standardOutput.contains(QStringLiteral("old.txt")));
}

void SvnClientTests::runHandlesRenameWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("content\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("move"), QStringLiteral("file.txt"), QStringLiteral("renamed.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(!QFileInfo(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt"))).exists());
    QVERIFY(QFileInfo(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("renamed.txt"))).exists());

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    const QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 2);

    bool sawDelete = false;
    bool sawAddWithHistory = false;
    for (const SvnStatus &status : statuses) {
        if (status.path == QStringLiteral("file.txt")) {
            sawDelete = status.code == QStringLiteral("D");
        }
        if (status.path == QStringLiteral("renamed.txt")) {
            sawAddWithHistory = status.code.startsWith('A') && status.code.contains('+');
        }
    }
    QVERIFY(sawDelete);
    QVERIFY(sawAddWithHistory);

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("rename file")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("list"), repositoryUrl}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(!result.standardOutput.contains(QStringLiteral("file.txt")));
    QVERIFY(result.standardOutput.contains(QStringLiteral("renamed.txt")));
}

void SvnClientTests::runHandlesLockWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("content\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("lock"), QStringLiteral("-m"), QStringLiteral("editing"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 1);
    QCOMPARE(statuses.at(0).code, QStringLiteral("K"));
    QCOMPARE(statuses.at(0).path, QStringLiteral("file.txt"));

    result = client.run({QStringLiteral("unlock"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 0);
}

void SvnClientTests::runHandlesRemoteRepositoryEditingWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();
    const QString folderUrl = repositoryUrl + QStringLiteral("/folder");
    const QString copiedUrl = repositoryUrl + QStringLiteral("/copied");
    const QString renamedUrl = repositoryUrl + QStringLiteral("/renamed");

    SvnResult result = client.run({QStringLiteral("mkdir"), folderUrl, QStringLiteral("-m"), QStringLiteral("create folder")});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("list"), repositoryUrl});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("folder/")));

    result = client.run({QStringLiteral("copy"), folderUrl, copiedUrl, QStringLiteral("-m"), QStringLiteral("copy folder")});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("list"), repositoryUrl});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("folder/")));
    QVERIFY(result.standardOutput.contains(QStringLiteral("copied/")));

    result = client.run({QStringLiteral("move"), folderUrl, renamedUrl, QStringLiteral("-m"), QStringLiteral("rename folder")});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("list"), repositoryUrl});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(!result.standardOutput.contains(QStringLiteral("folder/")));
    QVERIFY(result.standardOutput.contains(QStringLiteral("copied/")));
    QVERIFY(result.standardOutput.contains(QStringLiteral("renamed/")));

    result = client.run({QStringLiteral("delete"), renamedUrl, QStringLiteral("-m"), QStringLiteral("delete folder")});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("delete"), copiedUrl, QStringLiteral("-m"), QStringLiteral("delete copied folder")});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("list"), repositoryUrl});
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(!result.standardOutput.contains(QStringLiteral("renamed/")));
    QVERIFY(!result.standardOutput.contains(QStringLiteral("copied/")));
}

void SvnClientTests::runHandlesChangelistWorkflow()
{
    if (QStandardPaths::findExecutable(QStringLiteral("svn")).isEmpty()) {
        QSKIP("svn executable is not available");
    }
    if (QStandardPaths::findExecutable(QStringLiteral("svnadmin")).isEmpty()) {
        QSKIP("svnadmin executable is not available");
    }

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString repositoryPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("repo"));
    const QString workingCopyPath = QDir(temporaryDir.path()).absoluteFilePath(QStringLiteral("wc"));

    QCOMPARE(QProcess::execute(QStringLiteral("svnadmin"), {QStringLiteral("create"), repositoryPath}), 0);

    SvnClient client;
    const QString repositoryUrl = QUrl::fromLocalFile(repositoryPath).toString();

    SvnResult result = client.run({QStringLiteral("checkout"), repositoryUrl, workingCopyPath}, temporaryDir.path());
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QFile file(QDir(workingCopyPath).absoluteFilePath(QStringLiteral("file.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QVERIFY(file.write("first\n") > 0);
    file.close();

    result = client.run({QStringLiteral("add"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("initial"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    QVERIFY(file.open(QIODevice::Append | QIODevice::Text));
    QVERIFY(file.write("second\n") > 0);
    file.close();

    result = client.run({QStringLiteral("changelist"), QStringLiteral("review"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(result.standardOutput.contains(QStringLiteral("--- Changelist 'review':")));

    QVector<SvnStatus> statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 1);
    QCOMPARE(statuses.at(0).code, QStringLiteral("M"));
    QCOMPARE(statuses.at(0).path, QStringLiteral("file.txt"));

    result = client.run({QStringLiteral("changelist"), QStringLiteral("--remove"), QStringLiteral("file.txt")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));

    result = client.run({QStringLiteral("status")}, workingCopyPath);
    QVERIFY2(result.ok(), qPrintable(result.combinedOutput()));
    QVERIFY(!result.standardOutput.contains(QStringLiteral("--- Changelist 'review':")));
    statuses = client.parseStatus(result.standardOutput);
    QCOMPARE(statuses.size(), 1);
    QCOMPARE(statuses.at(0).path, QStringLiteral("file.txt"));
}

QTEST_MAIN(SvnClientTests)

#include "svnclient_tests.moc"
