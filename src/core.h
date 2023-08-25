#ifndef QST_BACKEND_CORE_H
#define QST_BACKEND_CORE_H
#include <grpcpp/server_builder.h>
#include <grpcpp/support/status.h>
#include <unistd.h>

#include "appinfo.hpp"
#include "qst.grpc.pb.h"
// #include "spdlog/logger.h"
#include "trie.hpp"

namespace qst {
  class AppSearcher {
  public:
    Trie<AppInfo> apps;
  public:
    AppSearcher();
    std::vector<AppInfo *> search(std::string_view word);
  };
  class QstBackendCore : public Interact::Service {
  public:
    std::string addr;
    std::unique_ptr<grpc::Server> server;
    AppSearcher searcher;
    // std::shared_ptr<spdlog::logger> logger;

    QstBackendCore(int argc, char *argv[]);
    void exec();
  protected:
    ::grpc::Status ListApp(::grpc::ServerContext *context, const ::qst::Input *request,
      ::grpc::ServerWriter<::qst::Display> *writer) override;
    ::grpc::Status RunApp(
      ::grpc::ServerContext *context, const ::qst::ExecHint *request, ::qst::Empty *response) override;
    void process(std::string args,bool stdio = false);
    void showHelp();
  };
}  // namespace qst
#endif