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

#include "clap-scanner/scanner.h"

namespace clap_scanner
{
std::vector<std::filesystem::path> installedCLAPs()
{
    auto sp = validCLAPSearchPaths();
    std::vector<std::filesystem::path> claps;
    for (const auto &p : sp)
    {
        try
        {
            for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(p))
            {
                if (dir_entry.path().extension().u8string() == ".clap")
                {
#if MAC
                    if (std::filesystem::is_directory(dir_entry.path()))
                    {
                        claps.emplace_back(dir_entry.path());
                    }
#else
                    if (!std::filesystem::is_directory(dir_entry.path()))
                    {
                        claps.emplace_back(dir_entry.path());
                    }
#endif
                }
            }
        }
        catch (const std::filesystem::filesystem_error &)
        {
        }
    }
    return claps;
}

bool foreachCLAPDescription(const clap_plugin_entry_t *entry,
                            std::function<void(const clap_plugin_descriptor_t *)> cb)
{
    auto fac = (clap_plugin_factory_t *)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    if (!fac)
    {
        return false;
    }
    auto plugin_count = fac->get_plugin_count(fac);
    if (plugin_count <= 0)
    {
        return false;
    }
    for (uint32_t pl = 0; pl < plugin_count; ++pl)
    {
        auto desc = fac->get_plugin_descriptor(fac, pl);
        if (!desc)
            return false;
        cb(desc);
    }
    return true;
}
} // namespace clap_scanner