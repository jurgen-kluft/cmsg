package cmsg

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	denv "github.com/jurgen-kluft/ccode/denv"
	crtti "github.com/jurgen-kluft/crtti/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

// GetPackage returns the package object of 'cmsg'
func GetPackage() *denv.Package {
	// Dependencies
	cunittestpkg := cunittest.GetPackage()
	cbasepkg := cbase.GetPackage()
	crttipkg := crtti.GetPackage()

	// The main (cmsg) package
	mainpkg := denv.NewPackage("cmsg")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(cbasepkg)
	mainpkg.AddPackage(crttipkg)

	// 'cmsg' library
	mainlib := denv.SetupDefaultCppLibProject("cmsg", "github.com\\jurgen-kluft\\cmsg")
	mainlib.Dependencies = append(mainlib.Dependencies, cbasepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, crttipkg.GetMainLib())

	// 'cmsg' unittest project
	maintest := denv.SetupDefaultCppTestProject("cmsg"+"_test", "github.com\\jurgen-kluft\\cmsg")
	maintest.Dependencies = append(maintest.Dependencies, cunittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, cbasepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, crttipkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
