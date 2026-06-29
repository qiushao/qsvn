#include "ExternalToolCommand.h"

#include <QProcess>

bool ExternalToolCommand::isValid() const
{
    return !program.isEmpty();
}

ExternalToolCommand buildExternalToolCommand(const QString &commandLine,
                                             const QMap<QString, QString> &placeholders,
                                             const QStringList &defaultArguments)
{
    QStringList command = QProcess::splitCommand(commandLine);
    if (command.isEmpty()) {
        return {};
    }

    ExternalToolCommand result;
    result.program = command.takeFirst();

    auto replacePlaceholders = [&placeholders, &result](QString value) {
        for (auto it = placeholders.constBegin(); it != placeholders.constEnd(); ++it) {
            const QString token = QStringLiteral("{") + it.key() + QStringLiteral("}");
            if (value.contains(token)) {
                result.usedPlaceholder = true;
                value.replace(token, it.value());
            }
        }
        return value;
    };

    result.program = replacePlaceholders(result.program);
    for (const QString &argument : command) {
        result.arguments.push_back(replacePlaceholders(argument));
    }

    if (!result.usedPlaceholder) {
        result.arguments.append(defaultArguments);
    }

    return result;
}
