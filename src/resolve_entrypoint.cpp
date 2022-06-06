//
// Created by Paul Walker on 5/13/22.
//

#include "clap-val-host.h"
#include <iostream>

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#if WIN
#include <windows.h>
#endif

#if LIN
#include <dlfcn.h>
#endif

namespace clap_val_host
{
#if MAC
clap_plugin_entry_t *entryFromClapPath(const std::filesystem::path &p)
{
    auto ps = p.u8string();
    auto cs = CFStringCreateWithBytes(kCFAllocatorDefault, (uint8_t *)ps.c_str(), ps.size(),
                                      kCFStringEncodingUTF8, false);
    auto bundleURL =
        CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cs, kCFURLPOSIXPathStyle, true);

    auto bundle = CFBundleCreate(kCFAllocatorDefault, bundleURL);

    auto db = CFBundleGetDataPointerForName(bundle, CFSTR("clap_entry"));

    CFRelease(bundle);
    CFRelease(bundleURL);
    CFRelease(cs);

    return (clap_plugin_entry_t *)db;
}
#endif

#if WIN
clap_plugin_entry_t *entryFromClapPath(const std::filesystem::path &p)
{
    auto han = LoadLibrary((LPCSTR)(p.generic_string().c_str()));
    if (!han)
        return nullptr;
    auto phan = GetProcAddress(han, "clap_entry");
    std::cout << "phan is " << phan << std::endl;
    return (clap_plugin_entry_t *)phan;
}
#endif


#if LIN
clap_plugin_entry_t *entryFromClapPath(const std::filesystem::path &p)
{
    std::cout << "WARNING - THIS CODE IS COMPLETELY UNTESTED" << std::endl;
    void    *handle;
    int     *iptr;

    handle = dlopen(p.u8string().c_str(), RTLD_LOCAL | RTLD_LAZY);

    iptr = (int *)dlsym(handle, "clap_entry");

    return (clap_plugin_entry_t *)iptr;
}

#endif


} // namespace clap_val_host
