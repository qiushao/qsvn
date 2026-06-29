#include "ApplicationSettings.h"

#include <QSettings>

ApplicationSettings ApplicationSettings::load()
{
    QSettings settings;

    ApplicationSettings values;
    values.svn.username = settings.value(QStringLiteral("svn/username")).toString();
    values.svn.password = settings.value(QStringLiteral("svn/password")).toString();
    values.svn.useAuthCache = settings.value(QStringLiteral("svn/useAuthCache"), true).toBool();
    values.svn.trustServerCertificate = settings.value(QStringLiteral("svn/trustServerCertificate"), false).toBool();
    values.externalDiffCommand = settings.value(QStringLiteral("tools/externalDiffCommand")).toString();
    values.externalMergeCommand = settings.value(QStringLiteral("tools/externalMergeCommand")).toString();
    return values;
}

void ApplicationSettings::save() const
{
    QSettings settings;
    settings.setValue(QStringLiteral("svn/username"), svn.username);
    settings.setValue(QStringLiteral("svn/password"), svn.password);
    settings.setValue(QStringLiteral("svn/useAuthCache"), svn.useAuthCache);
    settings.setValue(QStringLiteral("svn/trustServerCertificate"), svn.trustServerCertificate);
    settings.setValue(QStringLiteral("tools/externalDiffCommand"), externalDiffCommand);
    settings.setValue(QStringLiteral("tools/externalMergeCommand"), externalMergeCommand);
}
