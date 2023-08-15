#ifndef QST_SERVER_HPP
#define QST_SERVER_HPP
#include <grpcpp/server_builder.h>
#include <grpcpp/support/status.h>

#include <string>
#include <string_view>

#include "comm/qst.grpc.pb.h"
#include "comm/qst.pb.h"

namespace qst {
  class Server {
  private:
    class InteractImpl final : public qst::Interact::Service {
    public:
      InteractImpl() {
      }
      ~InteractImpl() {
      }
      // sample rpc
      ::grpc::Status Query(
        ::grpc::ServerContext *context, const ::qst::Input *request, ::qst::AppInfo *response) override {
        response->set_name(request->str());
        response->set_exec("hello world");
        std::cout << "Query: " << request->str() << std::endl;
        return ::grpc::Status::OK;
      }
      ::grpc::Status ListApp(::grpc::ServerContext *context, const ::qst::Input *request,
        ::grpc::ServerWriter< ::qst::AppInfo> *writer) override {
        qst::AppInfo info{};
        for(auto i = 0; i < 10; ++i) {
          info.set_name("hello");
          info.set_exec("world");
          writer->Write(info);
        }
        return ::grpc::Status::OK;
      }
    };
  public:
    Server()
      : service(new InteractImpl())
      , server() {
    }
    ~Server() {
    }
    void Run(std::string_view addr) {
      grpc::ServerBuilder builder;
      builder.AddListeningPort(std::string(addr), grpc::InsecureServerCredentials());
      builder.RegisterService(service.get());
      server = builder.BuildAndStart();
      std::cout << "Server listening on " << addr << std::endl;
    }
  private:
    std::unique_ptr<grpc::Server> server;
    std::unique_ptr<qst::Interact::Service> service;
  };
}  // namespace qst

#endif