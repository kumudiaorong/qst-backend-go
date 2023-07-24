#ifndef APPOPT_H
#define APPOPT_H
#include <QByteArray>
#include <QMap>
#include <QPair>
#include <QSettings>
#include <QSharedPointer>
#include <QVector>

#include "app/appinfo.h"
#include "trie.hpp"
#ifdef Q_OS_WIN
QSettings regSettings(
  "HKEY_LOCAL_"
  "MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
  QSettings::NativeFormat);
QStringList appKeys = regSettings.childGroups();
for(const QString& appKey : appKeys) {
  regSettings.beginGroup(appKey);
  for(auto&& s : regSettings.allKeys())
    qDebug() << s;
  QString appName = regSettings.value("DisplayName").toString();
  if(!appName.isEmpty()) {
    qDebug() << "Application Name:" << appName;
  }
  regSettings.endGroup();
}
#elif defined(Q_OS_LINUX)

class Appopt {
  QSettings settings;
  Trie<AppInfo> apps;
public:
  Appopt();
};
#endif
#endif  // APPOPT_H
