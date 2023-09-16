#ifndef QST_BACKEND_CORE_H
#define QST_BACKEND_CORE_H
#include <grpcpp/server_builder.h>
#include <grpcpp/support/status.h>
#include <vector>

#include "appinfo.h"
#include "processmanager.h"
#include "qst.grpc.pb.h"
// #include "spdlog/logger.h"
#include "trie.hpp"
#include "xcl/xcl.h"

namespace qst {

  class QstBackendCore : public qst_comm::Interact::Service {
  public:
    std::string addr;
    std::unique_ptr<grpc::Server> server;
    AppSearcher searcher;
    std::vector<AppInfo*> last_result;
    ProcessManager pm;
    // std::shared_ptr<spdlog::logger> logger;
    xcl::Xcl xcl;

    QstBackendCore(int argc, char *argv[]);
    ~QstBackendCore();
    void exec();
  protected:
    ::grpc::Status ListApp(
      ::grpc::ServerContext *context, const ::qst_comm::Input *request, ::qst_comm::DisplayList *response) override;
    ::grpc::Status RunApp(
      ::grpc::ServerContext *context, const ::qst_comm::ExecHint *request, ::qst_comm::Empty *response) override;
    void showHelp();
  };
}  // namespace qst
#endif