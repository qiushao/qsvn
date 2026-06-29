#pragma once

#include <QMap>
#include <QString>
#include <QStringList>

struct ExternalToolCommand {
    QString program;
    QStringList arguments;
    bool usedPlaceholder = false;

    bool isValid() const;
};

ExternalToolCommand buildExternalToolCommand(const QString &commandLine,
                                             const QMap<QString, QString> &placeholders,
                                             const QStringList &defaultArguments);
