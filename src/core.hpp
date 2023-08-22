#ifndef QST_CORE_HPP
#define QST_CORE_HPP
#include <grpcpp/server_builder.h>
#include <grpcpp/support/status.h>
#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <string_view>

#include "appinfo.hpp"
#include "qst.grpc.pb.h"
#include "qst.pb.h"
#include "trie.hpp"

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
            qst::Display app;
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
  class QstCore : public Interact::Service {
  public:
    std::string addr;
    std::string frontEnd;
    std::unique_ptr<grpc::Server> server;
    AppSearcher searcher;

    QstCore(int argc, char *argv[])
      : server()
      , addr()
      , searcher() {
      if(argc < 2) {
        showHelp();
      }
      for(int i = 1; i < argc; ++i) {
        if(std::strcmp(argv[i], "--addr") == 0) {
          addr = argv[++i];
        } else if(std::strcmp(argv[i], "--front-end") == 0) {
          frontEnd = argv[++i];
        } else if(std::strcmp(argv[i], "--help") == 0) {
          showHelp();
        }
      }
    }
    void exec() {
      ::grpc::ServerBuilder builder;
      builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
      builder.RegisterService(this);
      server = builder.BuildAndStart();
      std::cout << "Server listening on " << addr.data() << std::endl;
      process(frontEnd, frontEnd + " --addr " + addr);
      server->Wait();
    }
  protected:
    ::grpc::Status ListApp(::grpc::ServerContext *context, const ::qst::Input *request,
      ::grpc::ServerWriter<::qst::Display> *writer) override {
      std::cout << "ListApp: " << request->str() << std::endl;
      Display display;
      for(auto& info : searcher.search(request->str())){
        display.set_name(info->name());
        display.set_flags(info->flags());
        writer->Write(display);
      }
      return ::grpc::Status::OK;
    }
    virtual ::grpc::Status RunApp(
      ::grpc::ServerContext *context, const ::qst::ExecHint *request, ::qst::Empty *response) override {
      AppInfo *info = searcher.search(request->name())[0];
      process(info->exec().substr(0, info->exec().find_first_of(' ')), info->exec());
      std::cout << "RunApp: " << info->name() << std::endl;
      std::cout << "Exec: " << info->exec() << std::endl;
      return ::grpc::Status::OK;
    }
    void process(std::string_view path, std::string_view args) {
      std::cout << "exec: " << path.data() << " " << args.data() << std::endl;
      pid_t pid = fork();
      if(pid == 0) {
        fclose(stdin);
        fclose(stdout);
        fclose(stderr);
        setpgid(0, 0);
        int err = execlp(path.data(), args.data(), nullptr);
        exit(0);
      }
    }
    void showHelp() {
      std::cout << "Usage: qst [options]" << std::endl
                << "Options:" << std::endl
                << "  --addr <addr>       Set the address to listen on" << std::endl
                << "  --front-end <path>  Set the path of front-end" << std::endl
                << "  --help              Show this help message" << std::endl;
      std::exit(0);
    }
  };
}  // namespace qst
#endif