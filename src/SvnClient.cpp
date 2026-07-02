#include "SvnClient.h"

#include <QProcess>
#include <QStandardPaths>
#include <QXmlStreamReader>

bool SvnResult::ok() const
{
    return started && !timedOut && exitCode == 0;
}

QString SvnResult::combinedOutput() const
{
    QString output = standardOutput;
    if (!standardError.isEmpty()) {
        if (!output.isEmpty() && !output.endsWith('\n')) {
            output.append('\n');
        }
        output.append(standardError);
    }
    return output;
}

SvnClient::SvnClient(QObject *parent)
    : QObject(parent)
{
}

void SvnClient::setOptions(const SvnOptions &options)
{
    m_options = options;
}

SvnOptions SvnClient::options() const
{
    return m_options;
}

QStringList SvnClient::buildArguments(const QStringList &arguments) const
{
    QStringList actualArguments = arguments;
    if (!actualArguments.contains(QStringLiteral("--non-interactive"))) {
        const int insertIndex = actualArguments.isEmpty() ? 0 : 1;
        actualArguments.insert(insertIndex, QStringLiteral("--non-interactive"));
    }

    if (!m_options.username.isEmpty()) {
        actualArguments.append({QStringLiteral("--username"), m_options.username});
    }
    if (!m_options.password.isEmpty()) {
        actualArguments.append({QStringLiteral("--password"), m_options.password});
    }
    if (!m_options.useAuthCache) {
        actualArguments.append(QStringLiteral("--no-auth-cache"));
    }
    if (m_options.trustServerCertificate) {
        actualArguments.append(QStringLiteral("--trust-server-cert"));
    }

    return actualArguments;
}

SvnResult SvnClient::run(const QStringList &arguments, const QString &workingDirectory) const
{
    const QStringList actualArguments = buildArguments(arguments);
    return runExecutable(QStringLiteral("svn"), actualArguments, workingDirectory);
}

SvnResult SvnClient::runAdmin(const QStringList &arguments, const QString &workingDirectory) const
{
    return runExecutable(QStringLiteral("svnadmin"), arguments, workingDirectory);
}

SvnResult SvnClient::runExecutable(const QString &executableName, const QStringList &arguments, const QString &workingDirectory) const
{
    SvnResult result;

    const QString program = QStandardPaths::findExecutable(executableName);
    if (program.isEmpty()) {
        result.standardError = executableName + QStringLiteral(" executable was not found in PATH.");
        return result;
    }

    result.commandLine = commandLineForArguments(executableName, arguments);

    QProcess process;
    if (!workingDirectory.isEmpty()) {
        process.setWorkingDirectory(workingDirectory);
    }
    process.start(program, arguments);

    result.started = process.waitForStarted();
    if (!result.started) {
        result.standardError = process.errorString();
        return result;
    }

    constexpr int timeoutMs = 30 * 60 * 1000;
    if (!process.waitForFinished(timeoutMs)) {
        result.timedOut = true;
        process.kill();
        process.waitForFinished(3000);
    }

    result.exitCode = process.exitCode();
    result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
    result.standardError = QString::fromLocal8Bit(process.readAllStandardError());
    if (result.timedOut) {
        if (!result.standardError.isEmpty() && !result.standardError.endsWith('\n')) {
            result.standardError.append('\n');
        }
        result.standardError.append(executableName + QStringLiteral(" command timed out."));
    }
    return result;
}

QVector<SvnStatus> SvnClient::parseStatus(const QString &output) const
{
    QVector<SvnStatus> statuses;
    const QStringList lines = output.split('\n');
    for (QString line : lines) {
        line.remove('\r');
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith('>') || line.startsWith(QStringLiteral("--- Changelist")) || line.startsWith(QStringLiteral("Status against revision:"))) {
            continue;
        }

        SvnStatus status;
        status.code = line.left(7).trimmed();
        status.path = line.length() > 8 ? line.mid(8).trimmed() : QString();
        QString remoteStatus = line.length() > 7 ? line.mid(7).trimmed() : QString();
        if (remoteStatus.startsWith('*')) {
            status.code = status.code.isEmpty() ? QStringLiteral("*") : status.code + QStringLiteral(" *");
            remoteStatus = remoteStatus.mid(1).trimmed();
        }

        int revisionEnd = 0;
        while (revisionEnd < remoteStatus.size() && remoteStatus.at(revisionEnd).isDigit()) {
            ++revisionEnd;
        }
        if (revisionEnd > 0 && revisionEnd < remoteStatus.size() && remoteStatus.at(revisionEnd).isSpace()) {
            status.path = remoteStatus.mid(revisionEnd).trimmed();
        }

        if (!status.path.isEmpty()) {
            statuses.push_back(status);
        }
    }
    return statuses;
}

QVector<SvnLogEntry> SvnClient::parseLogXml(const QString &output) const
{
    QVector<SvnLogEntry> entries;
    QXmlStreamReader reader(output);

    SvnLogEntry currentEntry;
    QString currentPathAction;
    bool inLogEntry = false;
    bool inPath = false;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            const QStringView name = reader.name();
            if (name == QStringLiteral("logentry")) {
                currentEntry = SvnLogEntry();
                currentEntry.revision = reader.attributes().value(QStringLiteral("revision")).toString();
                inLogEntry = true;
            } else if (inLogEntry && name == QStringLiteral("author")) {
                currentEntry.author = reader.readElementText();
            } else if (inLogEntry && name == QStringLiteral("date")) {
                currentEntry.date = reader.readElementText();
            } else if (inLogEntry && name == QStringLiteral("msg")) {
                currentEntry.message = reader.readElementText();
            } else if (inLogEntry && name == QStringLiteral("path")) {
                currentPathAction = reader.attributes().value(QStringLiteral("action")).toString();
                inPath = true;
            }
        } else if (reader.isCharacters() && inPath) {
            const QString path = reader.text().toString().trimmed();
            if (!path.isEmpty()) {
                currentEntry.changedPaths.push_back(currentPathAction + QStringLiteral("  ") + path);
            }
        } else if (reader.isEndElement()) {
            const QStringView name = reader.name();
            if (name == QStringLiteral("path")) {
                inPath = false;
                currentPathAction.clear();
            } else if (name == QStringLiteral("logentry") && inLogEntry) {
                entries.push_back(currentEntry);
                inLogEntry = false;
            }
        }
    }

    if (reader.hasError()) {
        return {};
    }

    return entries;
}

QVector<SvnProperty> SvnClient::parsePropertiesXml(const QString &output) const
{
    QVector<SvnProperty> properties;
    QXmlStreamReader reader(output);

    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isStartElement() || reader.name() != QStringLiteral("property")) {
            continue;
        }

        SvnProperty property;
        property.name = reader.attributes().value(QStringLiteral("name")).toString();
        property.value = reader.readElementText();
        if (!property.name.isEmpty()) {
            properties.push_back(property);
        }
    }

    if (reader.hasError()) {
        return {};
    }

    return properties;
}

QString SvnClient::commandLineForArguments(const QString &executableName, const QStringList &arguments) const
{
    QStringList displayArguments;
    displayArguments.reserve(arguments.size());
    for (int i = 0; i < arguments.size(); ++i) {
        if (i > 0 && arguments.at(i - 1) == QStringLiteral("--password")) {
            displayArguments.push_back(QStringLiteral("<hidden>"));
        } else {
            displayArguments.push_back(quoteArgument(arguments.at(i)));
        }
    }
    return executableName + QLatin1Char(' ') + displayArguments.join(' ');
}

QString SvnClient::quoteArgument(const QString &argument) const
{
    if (argument.isEmpty()) {
        return QStringLiteral("\"\"");
    }

    const bool needsQuotes = argument.contains(' ') || argument.contains('\t') || argument.contains('"');
    if (!needsQuotes) {
        return argument;
    }

    QString quoted = argument;
    quoted.replace('"', QStringLiteral("\\\""));
    return QStringLiteral("\"") + quoted + QStringLiteral("\"");
}
