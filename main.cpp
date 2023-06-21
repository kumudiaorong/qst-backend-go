#include <QCoreApplication>
#include <QSettings>

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  QSettings regSettings(
      "HKEY_LOCAL_"
      "MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
      QSettings::NativeFormat);
  QStringList appKeys = regSettings.childGroups();
  for (const QString &appKey : appKeys) {
    regSettings.beginGroup(appKey);
    for (auto &&s : regSettings.allKeys())
      qDebug() << s;
    QString appName = regSettings.value("DisplayName").toString();
    if (!appName.isEmpty()) {
      qDebug() << "Application Name:" << appName;
    }
    regSettings.endGroup();
  }
  return a.exec();
}
