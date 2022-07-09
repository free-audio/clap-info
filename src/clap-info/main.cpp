#include <iostream>
#include <filesystem>

#include "clap-info-host.h"
#include "clap-scanner/scanner.h"
#include "clap/plugin-factory.h"

#include "info.h"

#include "CLI11/CLI11.hpp"

struct CLAPInfoJsonRoot
{
    Json::Value root;
    bool active{true};
    ~CLAPInfoJsonRoot()
    {
        if (active)
        {
            Json::StyledWriter writer;
            std::string out_string = writer.write(root);
            std::cout << out_string << std::endl;
        }
    }
};

int main(int argc, char **argv)
{
    CLI::App app("clap-info: CLAP command line validation tool");

    app.set_version_flag("--version", "0.0.0");
    std::string clap;
    app.add_option("-f,--file,file", clap, "CLAP plugin file location");

    bool create{true};
    app.add_option("--create", create, "Choose whether to create an instance of the plugin or just scan the entry.")->default_str("TRUE");

    uint32_t which_plugin{0};
    app.add_option( "--which", which_plugin, "Choose which plugin to create (if the CLAP has more than one)")->default_str("0");

    bool annExt{false};
    app.add_option("--announce-extensions", annExt, "Announce extensions queried by plugin.")->default_str("TRUE");

    bool descShow{true};
    app.add_option("--descriptions", descShow, "Show the descriptions of the plugins in this CLAP")->default_str("TRUE");

    bool paramShow{true};
    app.add_option( "--params", paramShow, "Print plugin parameters.")->default_str("TRUE");

    int paramVerbosity{2};
    app.add_option( "--param-verbosity", paramVerbosity, "How verbosely to display/query params (1-4)")->default_str("2");

    bool audioPorts{true};
    app.add_option( "--audio-ports", audioPorts, "Display the Audio Ports configuration")->default_str("TRUE");

    bool notePorts{true};
    app.add_option( "--note-ports", notePorts, "Display the Note Ports configuration")->default_str("TRUE");

    bool latency{true};
    app.add_option( "--latency", latency, "Display the latency configuration")->default_str("TRUE");

    bool gui{true};
    app.add_option( "--gui", notePorts, "Display the gui configuration")->default_str("TRUE");


    bool searchPath{false};
    app.add_flag( "--search-path", searchPath, "Show the CLAP plugin search paths then exit");

    bool showClaps{false};
    app.add_flag( "-l,--list-clap-files", showClaps, "Show all CLAP files in the search path then exit");

    bool showClapsWithDesc{false};
    app.add_flag( "-s,--scan-clap-files", showClapsWithDesc, "Show all descriptions in all CLAP files in the search path, then exit");

    CLI11_PARSE(app, argc, argv);

    CLAPInfoJsonRoot doc;

    if (searchPath)
    {
        doc.root["action"] = "display clap search paths";
        Json::Value res;
        auto sp = clap_scanner::validCLAPSearchPaths();
        for (const auto &q : sp)
            res.append(q.u8string());
        doc.root["result"] = res;

        return 0;
    }

    if (showClaps)
    {
        doc.root["action"] = "display paths for installed claps";
        Json::Value res;
        auto sp = clap_scanner::installedCLAPs();
        for (const auto &q : sp)
            res.append(q.u8string());
        doc.root["result"] = res;

        return 0;
    }


    if (showClapsWithDesc)
    {
        doc.root["action"] = "display descriptions for installed claps";
        Json::Value res;
        auto sp = clap_scanner::installedCLAPs();
        for (const auto &q : sp)
        {
            Json::Value entryJson;
            if (auto entry = clap_scanner::entryFromCLAPPath(q))
            {
                entryJson["path"] = q.u8string();
                entryJson["clap-version"] = std::to_string(entry->clap_version.major)
                                           + "." + std::to_string(entry->clap_version.minor) + "." +
                                           std::to_string(entry->clap_version.revision);
                entryJson["plugins"] = Json::Value();
                clap_scanner::foreachCLAPDescription(entry, [&entryJson](const clap_plugin_descriptor_t *desc) {
                    Json::Value thisPlugin;
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
                    entryJson["plugins"].append(thisPlugin);
                });
                res.append(entryJson);
            }
        }
        doc.root["result"] = res;
        return 0;
    }

    auto clapPath = std::filesystem::path(clap);
#if MAC
    if (!(std::filesystem::is_directory(clapPath) || std::filesystem::is_regular_file(clapPath)))
    {
        std::cerr << "Your file '" << clap << "' is neither a bundle nor a file" << std::endl;
        doc.active = false;
        return 2;
    }
#else
    if (!std::filesystem::is_regular_file(clapPath))
    {
        std::cerr << "Your file '" << clap << "' is not a regular file" << std::endl;
        doc.active = false;
        return 2;
    }
#endif


//    std::cout << "Loading clap        : " << clap << std::endl;
    auto entry = clap_scanner::entryFromCLAPPath(clapPath);

    if (!entry)
    {
        std::cerr << "   clap_entry returned a nullptr\n"
                  << "   either this plugin is not a CLAP or it has exported the incorrect symbol."
                  << std::endl;
        doc.active = false;
        return 3;
    }

    Json::Value &root = doc.root;
    root["file"] = clap;

    auto version = entry->clap_version;
    std::stringstream ss;
    ss << version.major << "." << version.minor << "." << version.revision;
    root["version"] = ss.str();

    entry->init(clap.c_str());

    auto fac = (clap_plugin_factory_t *)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    auto plugin_count = fac->get_plugin_count(fac);
    if (plugin_count <= 0)
    {
        std::cerr << "Plugin factory has no plugins" << std::endl;
        doc.active = 0;
        return 4;
    }

    root["plugin-count"] = plugin_count;

    if (descShow)
    {
        Json::Value factory;
        factory.resize(0);
        for (uint32_t pl = 0; pl < plugin_count; ++pl)
        {
            Json::Value pluginDescriptor;
            auto desc = fac->get_plugin_descriptor(fac, pl);

            pluginDescriptor["name"] = desc->name;
            pluginDescriptor["version"] = desc->version;
            pluginDescriptor["id"] = desc->id;
            pluginDescriptor["description"] = desc->description;

            auto f = desc->features;
            while (f[0])
            {
                pluginDescriptor["features"].append(f[0]);
                f++;
            }
            factory.append(pluginDescriptor);
        }
        root[CLAP_PLUGIN_FACTORY_ID] = factory;
    }

    if (!create)
    {
        return 0;
    }

    if (which_plugin < 0 || which_plugin >= plugin_count)
    {
        std::cerr << "Unable to create plugin " << which_plugin << " which must be between 0 and " << plugin_count - 1 << std::endl;
        doc.active = false;
        return 4;
    }

    auto desc = fac->get_plugin_descriptor(fac, which_plugin);


    // Now lets make an instance
    auto host = clap_info_host::createClapValHost();
    clap_info_host::getHostConfig()->announceQueriedExtensions = annExt;
    auto inst = fac->create_plugin(fac, host, desc->id);

    inst->init(inst);
    inst->activate(inst, 48000, 32, 4096);

    Json::Value extensions;

    if (paramShow)
    {
        extensions[CLAP_EXT_PARAMS] = clap_info_host::createParamsJson(inst);
    }

    if (audioPorts)
    {
        extensions[CLAP_EXT_AUDIO_PORTS] = clap_info_host::createAudioPortsJson(inst);
    }

    if (notePorts)
    {
        extensions[CLAP_EXT_NOTE_PORTS] = clap_info_host::createNotePortsJson(inst);
    }

    if (latency)
    {
        extensions[CLAP_EXT_LATENCY] = clap_info_host::createLatencyJson(inst);
    }

    if (latency)
    {
        extensions[CLAP_EXT_LATENCY] = clap_info_host::createLatencyJson(inst);
    }

    if (gui)
    {
        extensions[CLAP_EXT_GUI] = clap_info_host::createGuiJson(inst);
    }

    // Probably want to rework this structure
    root["id"] = desc->id;

    root["extensions"] = extensions;

    inst->deactivate(inst);
    inst->destroy(inst);

    entry->deinit();

    return 0;
}
