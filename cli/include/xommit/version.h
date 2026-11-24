// build-time constants. must be defined by build.js via -D flags
// XOMMIT_VERSION: "1.0.9" (from package.json)
// XOMMIT_REPO: "abhraneeldhar7/xommit" (github api urls)
// XOMMIT_EXE: "xommit.exe" (installed binary filename)
// XOMMIT_ASSET: "xommit-windows-x64.exe" (github release asset name)
// XOMMIT_HOMEPAGE: "https://xommit.antk.in" (project homepage)

#pragma once

// Must be defined by build system (-D flags)
#ifndef XOMMIT_VERSION
#error "XOMMIT_VERSION must be defined by build system"
#endif

#ifndef XOMMIT_REPO
#error "XOMMIT_REPO must be defined by build system"
#endif

#ifndef XOMMIT_EXE
#error "XOMMIT_EXE must be defined by build system"
#endif

#ifndef XOMMIT_ASSET
#error "XOMMIT_ASSET must be defined by build system"
#endif

#ifndef XOMMIT_HOMEPAGE
#error "XOMMIT_HOMEPAGE must be defined by build system"
#endif
