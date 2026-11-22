// build-time constants. must be defined by build.js via -D flags
// XOMMIT_VERSION: "1.0.6" (from package.json)
// XOMMIT_REPO: "abhraneeldhar7/xommit" (github api urls)
// XOMMIT_ASSET: "xommit.exe" (release asset name)

#pragma once

// Must be defined by build system (-D flags)
#ifndef XOMMIT_VERSION
#error "XOMMIT_VERSION must be defined by build system"
#endif

#ifndef XOMMIT_REPO
#error "XOMMIT_REPO must be defined by build system"
#endif

#ifndef XOMMIT_ASSET
#error "XOMMIT_ASSET must be defined by build system"
#endif
