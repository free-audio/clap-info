//
// Created by Paul Walker on 5/13/22.
//

#include "clap-info-host.h"
#include <iostream>

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#if WIN
#include <windows.h>
#include <shlobj_core.h>
#endif

#if LIN
#include <dlfcn.h>
#endif

namespace clap_info_host
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

void getSystemPaths(std::vector<std::filesystem::path> &);
#endif

#if WIN
clap_plugin_entry_t *entryFromClapPath(const std::filesystem::path &p)
{
    auto han = LoadLibrary((LPCSTR)(p.generic_string().c_str()));
    if (!han)
        return nullptr;
    auto phan = GetProcAddress(han, "clap_entry");
//    std::cout << "phan is " << phan << std::endl;
    return (clap_plugin_entry_t *)phan;
}
#endif


#if LIN
clap_plugin_entry_t *entryFromClapPath(const std::filesystem::path &p)
{
    void    *handle;
    int     *iptr;

    handle = dlopen(p.u8string().c_str(), RTLD_LOCAL | RTLD_LAZY);

    iptr = (int *)dlsym(handle, "clap_entry");

    return (clap_plugin_entry_t *)iptr;
}

#endif

std::vector<std::filesystem::path> validCLAPSearchPaths()
{
    std::vector<std::filesystem::path> res;

#if MAC
    getSystemPaths(res);
#endif

#if LIN
    res.emplace_back("/usr/lib/clap");
    res.emplace_back(std::filesystem::path(getenv("HOME")) / std::filesystem::path(".clap"));
#endif

#if WIN
    {
        // I think this should use SHGetKnownFilderLocation but I don't know windows well enough
        auto p = getenv("COMMONPROGRAMFILES");
        if (p)
        {
            res.emplace_back(std::filesystem::path{p} / "CLAP");
        }
        auto q = getenv("LOCALAPPDATA");
        if (q)
        {
            res.emplace_back(std::filesystem::path{q} / "Programs" / "Common" / "CLAP" );
        }

    }
#endif

    auto p = getenv( "CLAP_PATH" );

    if (p)
    {
#if WIN
        auto sep = ';';
#else
        auto sep = ':';
#endif
        auto cp = std::string(p);

        size_t pos;
        while((pos = cp.find(sep)) != std::string::npos)
        {
            auto item = cp.substr(0, pos);
            cp = cp.substr(pos+1);
            res.emplace_back(std::filesystem::path{item});
        }
        if (cp.size())
            res.emplace_back(std::filesystem::path{cp});
    }

    return res;
}


} // namespace clap_info_host
