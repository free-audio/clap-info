//
// Created by Paul Walker on 6/17/22.
//

#include <iostream>
#include "info.h"
#include "clap-info-host.h"

#include "json/json.h"

namespace clap_info_host
{
void showCLAPSearchpath()
{
    Json::Value root;
    root["action"] = "system clap file find";
    Json::Value path;
    auto sp = clap_info_host::validCLAPSearchPaths();
    for (auto q : sp)
        path.append(q.u8string());
    root["paths"] = path;

    Json::StyledWriter writer;
    auto s = writer.write(root);
    std::cout << s << std::endl;
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
        Json::Value root;
        root["action"] = "system clap file find";
        Json::Value clapsNode;
        for (auto p : claps)
            clapsNode.append(p.u8string());
        root["clap-files"] = clapsNode;
        Json::StyledWriter writer;
        std::string out_string = writer.write(root);
        std::cout << out_string << std::endl;
    }

    if (lev == FIND_DESCRIPTIONS)
    {
        Json::Value root;
        root["action"] = "system clap scan";

        Json::Value clapfiles;
        for (auto p : claps)
        {
            Json::Value thisClap;
            thisClap["path"] = p.u8string();
            auto entry = clap_info_host::entryFromClapPath(p);
            if (!entry)
            {
                thisClap["error"] = ".clap contains no clap_entry";
                continue;
            }

            thisClap["clap-version"] = std::to_string(entry->clap_version.major)
                + "." + std::to_string(entry->clap_version.minor) + "." +
                                         std::to_string(entry->clap_version.revision);
            if (!clap_version_is_compatible(entry->clap_version))
            {
                thisClap["error"] = "Incompatible clap version";
                continue;
            }

            entry->init(p.u8string().c_str());
            auto fac = (clap_plugin_factory_t *)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
            auto pc = fac->get_plugin_count(fac);
            thisClap["plugin-count"] = pc;

            Json::Value plugins;
            for (uint32_t pl = 0; pl < pc; ++pl)
            {
                Json::Value thisPlugin;
                auto desc = fac->get_plugin_descriptor(fac, pl);

                thisPlugin["name"] = desc->name;
                thisPlugin["version"] = desc->version;
                thisPlugin["id"] = desc->id;
                thisPlugin["description"] = desc->description;

                Json::Value features;

                auto f = desc->features;
                while (f[0])
                {
                    features.append(f[0]);
                    f++;
                }
                thisPlugin["features"] = features;
                plugins.append(thisPlugin);
            }
            thisClap["plugins"] = plugins;
            clapfiles.append(thisClap);
        }

        root["clap-files"] = clapfiles;
        Json::StyledWriter writer;
        std::string out_string = writer.write(root);
        std::cout << out_string << std::endl;

    }
}
}