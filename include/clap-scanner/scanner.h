//
// Created by Paul Walker on 7/9/22.
//

#ifndef CLAP_INFO_SCANNER_H
#define CLAP_INFO_SCANNER_H

#include <vector>
#include <filesystem>
#include <functional>

#include <clap/clap.h>

namespace clap_scanner
{
std::vector<std::filesystem::path> validCLAPSearchPaths();
std::vector<std::filesystem::path> installedCLAPs();

const clap_plugin_entry_t *entryFromCLAPPath(const std::filesystem::path &p);

bool foreachCLAPDescription(const clap_plugin_entry_t *, std::function<void(const clap_plugin_descriptor_t *)> );
inline bool foreachCLAPDescription(const std::filesystem::path &p, std::function<void(const clap_plugin_descriptor_t *)> cb)
{
    auto e = entryFromCLAPPath(p);
    if (e)
    {
        return foreachCLAPDescription(e, cb);
    }
    return false;
}


}

#endif // CLAP_INFO_SCANNER_H
