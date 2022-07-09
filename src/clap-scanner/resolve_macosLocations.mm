/*
 * CLAP-INFO
 *
 * https://github.com/free-audio/clap-info
 *
 * CLAP-INFO is Free and Open Source software, released under the MIT
 * License, a copy of which is included with this source in the file
 * "LICENSE.md"
 *
 * Copyright (c) 2022 Various Authors, per the Git Transaction Log
 */

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
        auto p =
            std::filesystem::path{[u fileSystemRepresentation]} / "Audio" / "Plug-Ins" / "CLAP";
        res.push_back(p);
    }

    if (sysLibURLs)
    {
        auto *u = [sysLibURLs objectAtIndex:0];
        auto p =
            std::filesystem::path{[u fileSystemRepresentation]} / "Audio" / "Plug-Ins" / "CLAP";
        res.push_back(p);
    }
}
}