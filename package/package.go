package cmsg

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	"github.com/jurgen-kluft/ccode/denv"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

// GetPackage returns the package object of 'cmsg'
func GetPackage() *denv.Package {
	// Dependencies
	cunittestpkg := cunittest.GetPackage()
	cbasepkg := cbase.GetPackage()

	// The main (cmsg) package
	mainpkg := denv.NewPackage("cmsg")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(cbasepkg)

	// 'cmsg' library
	mainlib := denv.SetupDefaultCppLibProject("cmsg", "github.com\\jurgen-kluft\\cmsg")
	mainlib.Dependencies = append(mainlib.Dependencies, cbasepkg.GetMainLib())

	// 'cmsg' unittest project
	maintest := denv.SetupDefaultCppTestProject("cmsg_test", "github.com\\jurgen-kluft\\cmsg")
	maintest.Dependencies = append(maintest.Dependencies, cunittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, cbasepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
