package main

import (
	ccode "github.com/jurgen-kluft/ccode"
	cpkg "github.com/jurgen-kluft/cmsg/package"
)

func main() {
	if ccode.Init() {
		ccode.GenerateFiles()
		ccode.Generate(cpkg.GetPackage())
	}
}
