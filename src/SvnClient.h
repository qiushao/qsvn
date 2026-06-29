#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

struct SvnResult {
    QString commandLine;
    QString standardOutput;
    QString standardError;
    int exitCode = -1;
    bool started = false;
    bool timedOut = false;

    bool ok() const;
    QString combinedOutput() const;
};

struct SvnStatus {
    QString code;
    QString path;
};

struct SvnLogEntry {
    QString revision;
    QString author;
    QString date;
    QString message;
    QStringList changedPaths;
};

struct SvnProperty {
    QString name;
    QString value;
};

struct SvnOptions {
    QString username;
    QString password;
    bool useAuthCache = true;
    bool trustServerCertificate = false;
};

class SvnClient : public QObject {
    Q_OBJECT

public:
    explicit SvnClient(QObject *parent = nullptr);

    void setOptions(const SvnOptions &options);
    SvnOptions options() const;
    QStringList buildArguments(const QStringList &arguments) const;
    SvnResult run(const QStringList &arguments, const QString &workingDirectory = QString()) const;
    QVector<SvnStatus> parseStatus(const QString &output) const;
    QVector<SvnLogEntry> parseLogXml(const QString &output) const;
    QVector<SvnProperty> parsePropertiesXml(const QString &output) const;

private:
    QString commandLineForArguments(const QStringList &arguments) const;
    QString quoteArgument(const QString &argument) const;

    SvnOptions m_options;
};
