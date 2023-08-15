#ifndef QST_CORE_HPP
#define QST_CORE_HPP
#include <grpcpp/server_builder.h>
#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <unicode/umachine.h>
#include <unicode/urename.h>
#include <unicode/utypes.h>

#include <codecvt>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <ranges>
#include <string_view>

#include "comm/qst.grpc.pb.h"
#include "tree/trie.h"
namespace qst {
  class AppSearcher {
  public:
    Trie<AppInfo> apps;
  public:
    AppSearcher() {
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
      QFileInfoList files = dir.entryInfoList();
      for(auto& info : files) {
        if(info.suffix() == "desktop") {
          QFile f(info.absoluteFilePath());
          if(f.open(QIODevice::ReadOnly)) {
            QTextStream in(&f);
            qst::AppInfo app;
            while(!in.atEnd() && !in.readLine().startsWith("[Desktop Entry]"))
              ;
            while(!in.atEnd()) {
              QString l = in.readLine();
              if(l.startsWith("Name=")) {
                app.set_name(std::move(l.mid(5).toUtf8()));
              } else if(l.startsWith("Exec=")) {
                app.set_exec(l.mid(5).toUtf8());
              } else if(l.startsWith("[")) {
                break;
              }
            }
            // std::u8string(app.name().data())
            std::wstring_convert<std::codecvt<char16_t, char8_t, std::mbstate_t>, char8_t> convert;
            apps.insert(convert.from_bytes(app.name()), std::move(app));
          } else {
            qDebug() << f.errorString();
          }
        }
      }
#endif
#if defined(__linux__)
      // addr = "unix:/tmp/qst.sock";
      std::filesystem::path p("/usr/share/applications");
      for(auto& e : std::filesystem::directory_iterator(p)) {
        if(e.path().extension() == ".desktop") {
          std::ifstream f(e.path());
          std::string line;
          qst::AppInfo app;
          do
            std::getline(f, line);
          while(!f.eof() && line != "[Desktop Entry]");
          while(!f.eof()) {
            std::getline(f, line);
            if(line.starts_with("Name=")) {
              app.set_name(std::move(line.substr(5)));
            } else if(line.starts_with("Exec=")) {
              app.set_exec(std::move(line.substr(5)));
            } else if(line.starts_with("[")) {
              break;
            }
          }
          // std::cout << app.name() << "\t\t\t" << app.exec() << std::endl;
          apps.insert(app.name(), std::move(app));
        }
      }
#endif
    }
    std::vector<AppInfo *> search(std::string_view word) {
      return apps.find_prefix(word);
    }
  };
  class QstCore : public qst::Interact::Service {
  public:
    std::string addr;
    std::unique_ptr<grpc::Server> server;
    AppSearcher searcher;
  public:
    QstCore(int& argc, char **argv)
      : server()
      , addr("0.0.0.0:50051")
      , searcher() {
    }
    ::grpc::Status Query(
      ::grpc::ServerContext *context, const ::qst::Input *request, ::qst::AppInfo *response) override {
      response->set_name(request->str());
      response->set_exec("hello world");
      std::cout << "Query: " << request->str() << std::endl;
      return ::grpc::Status::OK;
    }
    ::grpc::Status ListApp(::grpc::ServerContext *context, const ::qst::Input *request,
      ::grpc::ServerWriter<::qst::AppInfo> *writer) override {
      std::cout << "ListApp: " << request->str() << std::endl;
      for(auto& info : searcher.search(request->str()))
        writer->Write(*info);
      return ::grpc::Status::OK;
    }
    void exec() {
      ::grpc::ServerBuilder builder;
      builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
      builder.RegisterService(this);
      server = builder.BuildAndStart();
      std::cout << "Server listening on " << addr.data() << std::endl;
      server->Wait();
    }
  };
}  // namespace qst
#endif