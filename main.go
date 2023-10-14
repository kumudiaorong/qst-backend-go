package main

import (
	// "fmt"
	// "errors"

	// "log"
	// "main/trie"
	// "net"

	// "os/exec"
	// "strings"

	// "main/helloworld"

	// "net"

	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"qst-back-end/config"
	"qst-back-end/pb/daemon"
	"qst-back-end/pb/defs"

	// "github.com/kumudiaorong/xcl-go/xcl"
	// "google.golang.org/grpc"
	// "google.golang.org/grpc"
	"context"
	// "fmt"
	// "log"

	// "net"

	"google.golang.org/grpc"
)

type server struct {
	daemon.UnimplementedMainInteractServer
}

func (s *server) SetUp(context.Context, *defs.Empty) (*daemon.SetUpResult, error) {
	log.Printf("SetUp")
	var running = make(map[string]string)
	for prompt, ext := range config.Status.Attr.Exts {
		if ext.Addr != "" {
			running[prompt] = ext.Addr
		}
	}
	ret := &daemon.SetUpResult{}
	ret.Mresult = &daemon.SetUpResult_Ok{Ok: &daemon.SetUpResult_MOk{Running: running}}
	return ret, nil
}

func (s *server) GetConfig(ctx context.Context, _ *defs.Empty) (*daemon.ConfigResult, error) {
	log.Printf("GetConfig")

	exts := make(map[uint32]*daemon.ExtInfo)
	// var cfg = daemon.ConfigResult{Exts: make(map[uint32]*daemon.ExtInfo)}
	for prompt, ext := range config.Status.Attr.Exts {
		log.Printf("ext: %+v\n", ext)
		exts[ext.Id] = &daemon.ExtInfo{Prompt: prompt, Name: ext.Name, Dir: ext.Dir, Exec: ext.Exec}
	}
	var ret = &daemon.ConfigResult{}
	ret.Mresult = &daemon.ConfigResult_Ok{Ok: &daemon.ConfigResult_MOk{Exts: exts}}
	return ret, nil
}
func (s *server) SetConfig(ctx context.Context, cfg *daemon.ConfigHint) (*defs.MResult, error) {
	log.Printf("SetConfig: %+v\n", cfg)
	config.Status.Attr.Exts = make(map[string]*config.ExtAttr)
	for id, ext := range cfg.Exts {
		log.Printf("ext: %+v\n", ext)
		config.Status.Attr.Exts[ext.Prompt] = &config.ExtAttr{Id: id, Name: ext.Name, Dir: ext.Dir, Exec: ext.Exec}
	}
	var ret = &defs.MResult{}
	ret.Mresult = &defs.MResult_Ok{Ok: &defs.MResult_MOk{}}
	return ret, nil
}

func freePort() (int, error) {
	lis, err := net.Listen("tcp", ":0")
	if err != nil {
		return 0, err
	}
	defer lis.Close()
	return lis.Addr().(*net.TCPAddr).Port, nil
}

func (s *server) GetExtAddr(_ context.Context, prompt *daemon.Prompt) (*daemon.ExtAddrResult, error) {
	log.Printf("GetExtPort: %v", prompt)
	attr, ok := config.Status.Attr.Exts[prompt.Content]
	ret := &daemon.ExtAddrResult{}
	ret.Mresult = &daemon.ExtAddrResult_Status{Status: &defs.Status{}}
	if !ok {
		return ret, nil
	}
	if attr.Addr != "" {
		ret.Mresult = &daemon.ExtAddrResult_Ok{Ok: &daemon.ExtAddrResult_MOk{Addr: attr.Addr}}
		return ret, nil
	}
	//todo: check if the process is running
	extport, err := freePort()
	if err != nil {
		log.Printf("freePort: %v\n", err)
		return ret, nil
	}
	args := []string{attr.Exec, "-port", fmt.Sprintf("%d", extport)}
	proc, err := os.StartProcess(attr.Exec, args, &os.ProcAttr{Dir: attr.Dir, Files: []*os.File{os.Stdin, os.Stdout, os.Stderr}})
	if err != nil {
		log.Printf("StartProcess: %v\n", err)
		return ret, nil
	}
	attr.Addr = fmt.Sprintf("127.0.0.1:%d", extport)
	config.Status.RunStat[prompt.Content] = &config.ExtService{Process: proc}
	ret.Mresult = &daemon.ExtAddrResult_Ok{Ok: &daemon.ExtAddrResult_MOk{Addr: attr.Addr}}
	return ret, nil
}

// func (s *server) InputChange(ctx context.Context, in *defs.Input) (*daemon.DisplayResult, error) {
// 	log.Printf("InputChange: %v", in)
// 	var prompt string
// 	var ext *config.ExtType
// 	var ok bool
// 	if in.Content[0] == '\\' {
// 		end := strings.Index(in.Content, ":")
// 		if end != -1 {
// 			prompt = in.Content[1:end]
// 			log.Printf("prompt: %s\n", prompt)
// 			ext, ok = config.Config.Exts[prompt]
// 			if !ok {
// 				return &daemon.DisplayResult{Type: daemon.ResultType_ERROR}, nil
// 			}
// 		}
// 	} else {
// 		ext, ok = config.Config.Exts[""]
// 		if !ok {
// 			return &daemon.DisplayResult{Type: daemon.ResultType_ERROR}, nil
// 		}
// 		log.Printf("prompt: \n")
// 	}
// 	cli, ok := extStack[ext.Id]
// 	if !ok {
// 		port, err := freePort()
// 		if err != nil {
// 			log.Printf("freePort: %v\n", err)
// 			return &daemon.DisplayResult{Type: daemon.ResultType_ERROR}, nil
// 		}
// 		cli = NewClient(port)
// 		// exec.
// 		var args []string
// 		args = append(args, ext.Name)
// 		args = append(args, "-port")
// 		args = append(args, fmt.Sprintf("%d", port))
// 		proc, err := os.StartProcess(config.Config.Exts[prompt].Exec, args, &os.ProcAttr{Dir: ext.Dir, Files: []*os.File{os.Stdin, os.Stdout, os.Stderr}})
// 		if err != nil {
// 			log.Printf("StartProcess: %v\n", err)
// 			return &daemon.DisplayResult{Type: daemon.ResultType_ERROR}, nil
// 		}
// 		cli.Process = proc
// 		extStack[ext.Id] = cli
// 	}
// 	ctx = context.Background()
// 	res, err := cli.InputChange(ctx, in)
// 	//
// 	var c = 5
// 	for err != nil && c > 0 {
// 		log.Printf("err: %v\n", err)
// 		time.Sleep(time.Second)
// 		res, err = cli.InputChange(ctx, in)
// 		c--
// 	}
// 	if c == 0 {
// 		return &daemon.DisplayResult{Type: daemon.ResultType_ERROR}, nil
// 	}
// 	log.Printf("res: %+v\n", res)
// 	return &daemon.DisplayResult{Type: daemon.ResultType_OK, Id: ext.Id, Display: res}, nil
// }

// func (s *server) Submit(ctx context.Context, hint *defs.SubmitHint) (*daemon.SubmitResult, error) {
// 	log.Printf("Submit: %v", hint)
// 	ext, ok := extStack[hint.Id]
// 	if !ok {
// 		return &daemon.SubmitResult{Type: daemon.ResultType_ERROR}, nil
// 	}
// 	ctx = context.Background()
// 	res, err := ext.Submit(ctx, hint)
// 	if err != nil {
// 		log.Printf("err: %v\n", err)
// 		return &daemon.SubmitResult{Type: daemon.ResultType_ERROR}, nil
// 	}
// 	log.Printf("res: %+v\n", res)
// 	return &daemon.SubmitResult{Type: daemon.ResultType_OK, Id: hint.Id, Display: res}, nil
// }

// func run(port *int) {
// 	net.
// 	if err := Server.Serve(lis); err != nil {
// 		log.Fatalf("failed to serve: %v", err)
// 	}
// }

var (
	port = flag.Int("port", 50001, "The server port")
)

// var Server *grpc.Server
// var lis net.Listener

func init() {
	// log.Printf("server listening at %v", lis.Addr())
}

func main() {
	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf("127.0.0.1:%d", *port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	Server := grpc.NewServer()
	daemon.RegisterMainInteractServer(Server, &server{})
	log.Printf("server listening at %v", lis.Addr())
	if err := Server.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
