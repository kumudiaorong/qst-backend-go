#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "core.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
namespace qst {
  AppSearcher::AppSearcher() {
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
            std::string::size_type pos = 5;
            do {
              pos = line.find_first_of('%', pos);
              if(pos == std::string::npos) {
                break;
              }
              if(line[++pos] == '%') {
                continue;
              } else if(line[pos] == 'f') {
                app.add_flag(qst::AppInfoFlags::HasArgFile);
              } else if(line[pos] == 'F') {
                app.add_flag(qst::AppInfoFlags::HasArgFiles);
              } else if(line[pos] == 'u') {
                app.add_flag(qst::AppInfoFlags::HasArgUrl);
              } else if(line[pos] == 'U') {
                app.add_flag(qst::AppInfoFlags::HasArgUrls);
              } else if(line[pos] == 'd' || line[pos] == 'D' || line[pos] == 'n' || line[pos] == 'N' || line[pos] == 'v'
                        || line[pos] == 'm') {
                line.erase(pos, 2);
              } else if(line[pos] == 'i') {
              } else if(line[pos] == 'c') {
              } else if(line[pos] == 'k') {
              }
            } while(pos != std::string::npos);
            app.set_exec(std::move(line.substr(5)));
            // app.set_exec(std::move(line.substr(5)));
          } else if(line.starts_with("[")) {
            break;
          }
        }
        spdlog::trace("Add app: name={} exec={} flags={}", app.name(), app.exec(), app.flags());
        apps.insert(app.name(), std::move(app));
        app.set_flags(0);
      }
    }
#endif
  }
  std::vector<AppInfo *> AppSearcher::search(std::string_view word) {
    return apps.find_prefix(word);
  }
  QstBackendCore::QstBackendCore(int argc, char *argv[])
    : server()
    , addr()
    , searcher()
    , logger(spdlog::create_async<spdlog::sinks::stdout_color_sink_mt>("backend")) {
    spdlog::set_default_logger(logger);
    if(argc < 2) {
      showHelp();
    }
    for(int i = 1; i < argc; ++i) {
      if(std::strcmp(argv[i], "--addr") == 0) {
        addr = argv[++i];
        spdlog::debug("Set address to listen on: {}", addr);
      } else if(std::strcmp(argv[i], "--front-end") == 0) {
        frontEnd = argv[++i];
        spdlog::debug("Set front-end path: {}", frontEnd);
      } else if(std::strcmp(argv[i], "--help") == 0) {
        showHelp();
      }
    }
  }
  void QstBackendCore::exec() {
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(this);
    server = builder.BuildAndStart();
    spdlog::trace("Server started");
    process(frontEnd + " --addr " + addr);
    server->Wait();
  }
  ::grpc::Status QstBackendCore::ListApp(
    ::grpc::ServerContext *context, const ::qst::Input *request, ::grpc::ServerWriter<::qst::Display> *writer) {
    spdlog::debug("ListApp: input={}", request->str());
    Display display;
    for(auto& info : searcher.search(request->str())) {
      display.set_name(info->name());
      display.set_flags(info->flags());
      writer->Write(display);
    }
    return ::grpc::Status::OK;
  }
  ::grpc::Status QstBackendCore::RunApp(
    ::grpc::ServerContext *context, const ::qst::ExecHint *request, ::qst::Empty *response) {
    AppInfo *info = searcher.search(request->name())[0];
    spdlog::debug("RunApp: name={} exec={} flags={}", info->name(), info->exec(), info->flags());
    std::string args(info->exec());
    if(info->flags() & static_cast<uint32_t>(AppInfoFlags::HasArgFile)) {
      args.replace(args.find("%f"), 2, request->has_file() ? request->file() : "");
    }
    if(info->flags() & static_cast<uint32_t>(AppInfoFlags::HasArgFiles)) {
      args.replace(args.find("%F"), 2, "");
    }
    if(info->flags() & static_cast<uint32_t>(AppInfoFlags::HasArgUrl)) {
      args.replace(args.find("%u"), 2, request->has_url() ? request->url() : "");
    }
    if(info->flags() & static_cast<uint32_t>(AppInfoFlags::HasArgUrls)) {
      args.replace(args.find("%U"), 2, "");
    }
    process(std::move(args));
    return ::grpc::Status::OK;
  }
  void QstBackendCore::process(std::string args) {
    spdlog::debug("Process: args={}", args);
    pid_t pid = fork();
    if(pid == 0) {
      // fclose(stdin);
      // fclose(stdout);
      // fclose(stderr);
      setpgid(0, 0);
      std::system(args.data());
      exit(0);
    }
  }
  void QstBackendCore::showHelp() {
    std::cout << "Usage: qst [options]" << std::endl
              << "Options:" << std::endl
              << "  --addr <addr>       Set the address to listen on" << std::endl
              << "  --front-end <path>  Set the path of front-end" << std::endl
              << "  --help              Show this help message" << std::endl;
    std::exit(0);
  }
}  // namespace qst