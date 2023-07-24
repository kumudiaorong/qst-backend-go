#include <qfileinfo.h>
#include <qlocale.h>

#include <QDir>
#include <QLocale>

#include "appopt.h"
Appopt::Appopt() {
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
  QDir dir("/usr/share/applications");
  QStringList files = dir.entryList();
  for(const QString& file : files) {
    QFileInfo info(file);
    if(info.isFile()) {
      qDebug() << info.absoluteFilePath();
      QFile f(info.absoluteFilePath());
      if(f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        while(!in.atEnd() && !in.readLine().startsWith("[Desktop Entry]"))
          ;
        while(!in.atEnd()) {
          QString l = in.readLine();
          if(l.startsWith("Name=")) {
            qDebug() << l;
          }
        }
      }
    }
  }
#endif
}
