#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#ifdef __linux__
#include <unistd.h>
#endif
#include "core.h"
// #include <wayland-client.h>
// #include "spdlog/async.h"
// #include "spdlog/sinks/stdout_color_sinks.h"
#include "qst.pb.h"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"
namespace qst {

  QstBackendCore::QstBackendCore(int argc, char *argv[])
    : server()
    , addr()
    , searcher()
  // , logger(
  // spdlog::create_async<spdlog::sinks::stdout_color_sink_mt>("backend")
  // )
  {
    // spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::trace);
    if(argc < 2) {
      showHelp();
    }
    for(int i = 1; i < argc; ++i) {
      if(std::strcmp(argv[i], "--addr") == 0) {
        addr = argv[++i];
        spdlog::trace("Set address to listen on: {}", addr);
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
    spdlog::trace("Start server");
    server->Wait();
  }
  ::grpc::Status QstBackendCore::ListApp(
    ::grpc::ServerContext *context, const ::qst_comm::Input *request, ::qst_comm::DisplayList *response) {
    spdlog::trace("ListApp\t: input\t= {}", request->str());
    ::qst_comm::Display display;
    // Display display;
    for(auto& info : searcher.search(request->str())) {
      spdlog::trace("ListApp\t: name\t= {}", info->name());
      display.set_name(std::string(info->name()));
      display.set_flags(info->flags());
      response->add_list()->CopyFrom(display);
    }
    return ::grpc::Status::OK;
  }
  ::grpc::Status QstBackendCore::RunApp(
    ::grpc::ServerContext *context, const ::qst_comm::ExecHint *request, ::qst_comm::Empty *response) {
    AppInfo *info = searcher.search(request->name())[0];
    spdlog::trace("RunApp\t: name\t= {}", info->name());
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
  void QstBackendCore::process(std::string args, bool stdio) {
    spdlog::trace("Process start: args={}", args);
#if defined(_WIN32) || defined(_WIN64)
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    auto err = CreateProcess(nullptr, args.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, nullptr, nullptr);
    if(err == 0) {
      spdlog::error("CreateProcess failed: {}", GetLastError());
    }
    spdlog::trace("Process end");
#elif defined(__linux__)
    pid_t pid = fork();
    if(pid == 0) {
      if(!stdio) {
        fclose(stdin);
        fclose(stdout);
        fclose(stderr);
      }
      setpgid(0, 0);
      std::system(args.data());
      exit(0);
    }
#endif
  }
  void QstBackendCore::showHelp() {
    std::cout << "Usage: qst [options]" << std::endl
              << "Options:" << std::endl
              << "  --addr <addr>       Set the address to listen on" << std::endl
              << "  --help              Show this help message" << std::endl;
    std::exit(0);
  }
}  // namespace qst