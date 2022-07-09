#include <vector>
#include <filesystem>
#include <iostream>

#include <Foundation/Foundation.h>

namespace clap_scanner
{

void getSystemPaths(std::vector<std::filesystem::path> &res)
{
    auto *fileManager = [NSFileManager defaultManager];
    auto *userLibURLs = [fileManager URLsForDirectory:NSLibraryDirectory
                                           inDomains:NSUserDomainMask];
    auto *sysLibURLs = [fileManager URLsForDirectory:NSLibraryDirectory
                                           inDomains:NSLocalDomainMask];

    if (userLibURLs)
    {
        auto *u = [userLibURLs objectAtIndex:0];
        auto p = std::filesystem::path{[u fileSystemRepresentation]} / "Audio" / "Plug-Ins" / "CLAP";
        res.push_back(p);
    }

    if (sysLibURLs)
    {
        auto *u = [sysLibURLs objectAtIndex:0];
        auto p = std::filesystem::path{[u fileSystemRepresentation]} / "Audio" / "Plug-Ins" / "CLAP";
        res.push_back(p);
    }
}
}