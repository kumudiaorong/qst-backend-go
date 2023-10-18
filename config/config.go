package config

import (
	"log"
	"os"

	"gopkg.in/yaml.v3"
)

type ExtAttr struct {
	// id of extension
	Id uint32 `yaml:"id"`
	// prompt of extension
	Name string `yaml:"prompt"`
	// dir of extension
	Dir string `yaml:"path"`
	// executable file of extension
	Exec string `yaml:"exec"`
	// run status of extension
	Addr string `yaml:"runstat"`
}

type FileAttr struct {
	// QstPath is the path of qst
	Exts map[string]*ExtAttr `yaml:"exts"`
}
type ExtService struct {
	*os.Process
}
type ExtStatus struct {
	Attr    FileAttr
	RunStat map[string]*ExtService //区分本地和远程
}

var (
	// decoder is the decoder of config file
	decoder *yaml.Decoder
	// Config is the config of qst
	Status ExtStatus
)

func init() {
	var config FileAttr
	var cfgPath string
	home := os.Getenv("HOME")
	if home == "" {
		log.Println("Can't Find Home Path")
	} else {
		cfgPath = home + "/.config/qst/config.yaml"
		ifs, err := os.Open(cfgPath)
		if err != nil {
			if os.IsNotExist(err) {
				err = os.MkdirAll(home+"/.config/qst", 0755)
				if err != nil {
					log.Printf("Can't Create Config Dir:%v\n", err)
				}
				ifs, err = os.Create(cfgPath)
				if err != nil {
					log.Printf("Can't Create Config File:%v\n", err)
				} else {
					defer ifs.Close()
				}
			} else {
				log.Printf("Can't Open Config File:%v\n", err)
			}
		} else {
			defer ifs.Close()
		}
		if ifs != nil {
			decoder = yaml.NewDecoder(ifs)
			decoder.Decode(&config)
		}
	}

	if Status.Attr.Exts == nil {
		Status.Attr.Exts = make(map[string]*ExtAttr)
		Status.Attr.Exts[""] = &ExtAttr{Id: 0, Name: "qst-ext-appsearcher", Dir: "/home/kmdr/pro/qst/qst-ext-appsearcher", Exec: "/home/kmdr/pro/qst/qst-ext-appsearcher-go/qst-ext-appsearcher-go"}
	}
	Status.RunStat = make(map[string]*ExtService)
}
