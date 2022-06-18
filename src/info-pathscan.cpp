//
// Created by Paul Walker on 6/17/22.
//

#include <iostream>
#include "info.h"
#include "clap-info-host.h"

namespace clap_info_host
{
void showCLAPSearchpath()
{
    std::cout << "CLAP Search Path:" << std::endl;
    auto sp = clap_info_host::validCLAPSearchPaths();
    for (auto q : sp)
        std::cout << "    " << q.u8string() << std::endl;
}

void recurseAndListCLAPSearchpath(ScanLevel lev)
{
    auto sp = clap_info_host::validCLAPSearchPaths();
    std::vector<std::filesystem::path> claps;
    for (auto p : sp)
    {
        try
        {
            for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(p))
            {
                if (dir_entry.path().extension().u8string() == ".clap")
                {
                    claps.emplace_back(dir_entry.path());
                }
            }
        }
        catch (const std::filesystem::filesystem_error &)
        {

        }
    }

    if (lev == FIND_FILES)
    {
        std::cout << "All CLAP files in path\n";
        for (auto p : claps)
            std::cout << "  " << p.u8string() << std::endl;
    }

    if (lev == FIND_DESCRIPTIONS)
    {
        std::cout << "System CLAP Scan\n";
        for (auto p : claps)
        {
            auto entry = clap_info_host::entryFromClapPath(p);
            if (!entry)
            {
                std::cout << p.u8string() << "   **ERROR: NO CLAP_ENTRY**" << std::endl;
                continue;
            }
            if (!clap_version_is_compatible(entry->clap_version))
            {
                std::cout << p.u8string() << "   **ERROR: INCOMPATIBLE VERSION** "
                          << entry->clap_version.major << "."
                          << entry->clap_version.minor << "."
                          << entry->clap_version.revision
                          << std::endl;
                continue;
            }

            entry->init(p.u8string().c_str());
            auto fac = (clap_plugin_factory_t *)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
            auto pc = fac->get_plugin_count(fac);
            std::cout << p.u8string() << "  : Contains " << pc << " plugin" << (pc > 1 ? "s" : "") << std::endl;
            for (uint32_t pl = 0; pl < pc; ++pl)
            {
                auto desc = fac->get_plugin_descriptor(fac, pl);

                std::cout << "  Plugin " << pl << " description: \n"
                          << "    name     : " << desc->name << "\n"
                          << "    version  : " << desc->version << "\n"
                          << "    id       : " << desc->id << "\n"
                          << "    desc     : " << desc->description << "\n"
                          << "    features : ";
                auto f = desc->features;
                auto pre = std::string();
                while (f[0])
                {
                    std::cout << pre << f[0];
                    pre = ", ";
                    f++;
                }
                std::cout << std::endl;
            }
        }

    }
}
}