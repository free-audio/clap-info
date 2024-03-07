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

#include <iostream>
#include <filesystem>

#include "clap/all.h"

#include "clap-info-host.h"
#include "clap-scanner/scanner.h"
#include "clap/factory/plugin-factory.h"

#include "info.h"

#include "CLI11/CLI11.hpp"

struct CLAPInfoJsonRoot
{
    Json::Value root;
    bool active{true};
    std::string outFile;
    ~CLAPInfoJsonRoot()
    {
        if (active)
        {
            Json::StyledWriter writer;
            std::string out_string = writer.write(root);
            if (outFile.empty())
            {
                std::cout << out_string << std::endl;
            }
            else
            {
                auto ofs = std::ofstream(outFile);
                if (ofs.is_open())
                {
                    ofs << out_string;
                    ofs.close();
                }
                else
                {
                    std::cout << "Unable to open output file '" << outFile << "' for writing";
                }
            }
        }
    }
};

int main(int argc, char **argv)
{
    CLI::App app("clap-info: CLAP command line validation tool");

    app.set_version_flag("--version", "0.9.0");
    std::string clap;
    app.add_option("-f,--file,file", clap, "CLAP plugin file location");

    bool showClaps{false};
    app.add_flag("-l,--list-clap-files", showClaps,
                 "Show all CLAP files in the search path then exit");

    bool showClapsWithDesc{false};
    app.add_flag("-s,--scan-clap-files", showClapsWithDesc,
                 "Show all descriptions in all CLAP files in the search path, then exit");

    std::string outFile{};
    app.add_option("-o,--output", outFile, "Redirect JSON to an output file rather than stdout");

    bool create{true};
    app.add_option("--create", create,
                   "Choose whether to create an instance of the plugin or just scan the entry.")
        ->default_str("TRUE");

    int32_t which_plugin{-1};
    app.add_option(
           "--which", which_plugin,
           "Choose which plugin to create (if the CLAP has more than one). If you set to -1 "
           "we will traverse all plugins.")
        ->default_str("-1");

    bool annExt{false};
    app.add_option("--announce-extensions", annExt, "Announce extensions queried by plugin.")
        ->default_str("FALSE");

    bool descShow{true};
    app.add_option("--descriptions", descShow, "Show the descriptions of the plugins in this CLAP")
        ->default_str("TRUE");

    bool paramShow{true};
    app.add_option("--params", paramShow, "Print plugin parameters.")->default_str("TRUE");

    /*int paramVerbosity{2};
    app.add_option("--param-verbosity", paramVerbosity,
                   "How verbosely to display/query params (1-4)")
        ->default_str("2");
        */

    bool audioPorts{true};
    app.add_option("--audio-ports", audioPorts, "Display the Audio Ports configuration")
        ->default_str("TRUE");

    bool notePorts{true};
    app.add_option("--note-ports", notePorts, "Display the Note Ports configuration")
        ->default_str("TRUE");

    bool otherExt{true};
    app.add_option("--other-ext", otherExt, "Display brief information about all other extensions")
        ->default_str("TRUE");

    bool searchPath{false};
    app.add_flag("--search-path", searchPath, "Show the CLAP plugin search paths then exit");

    bool brief{false};
    app.add_flag("--brief", brief, "Output brief infomation only");

    CLI11_PARSE(app, argc, argv);

    CLAPInfoJsonRoot doc;
    doc.outFile = outFile;

    // If brief param is set, override other params to output concise information only
    if (brief)
    {
        annExt = false;
        audioPorts = false;
        notePorts = false;
        paramShow = false;
        otherExt = false;
    }

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
                entry->init(q.u8string().c_str());
                entryJson["path"] = q.u8string();
                entryJson["clap-version"] = std::to_string(entry->clap_version.major) + "." +
                                            std::to_string(entry->clap_version.minor) + "." +
                                            std::to_string(entry->clap_version.revision);
                entryJson["plugins"] = Json::Value();
                clap_scanner::foreachCLAPDescription(
                    entry, [&entryJson](const clap_plugin_descriptor_t *desc) {
                        Json::Value thisPlugin;
                        thisPlugin["name"] = desc->name;
                        if (desc->version)
                            thisPlugin["version"] = desc->version;
                        thisPlugin["id"] = desc->id;
                        if (desc->vendor)
                            thisPlugin["vendor"] = desc->vendor;
                        if (desc->description)
                            thisPlugin["description"] = desc->description;

                        Json::Value features;

                        auto f = desc->features;
                        int idx = 0;
                        while (f[0])
                        {
                            bool nullWithinSize{false};
                            for (int i = 0; i < CLAP_NAME_SIZE; ++i)
                            {
                                if (f[0][i] == 0)
                                {
                                    nullWithinSize = true;
                                }
                            }

                            if (!nullWithinSize)
                            {
                                std::cerr
                                    << "Feature element at index " << idx
                                    << " lacked null within CLAP_NAME_SIZE."
                                    << "This means either a feature at this index overflowed or "
                                       "you didn't null terminate your "
                                    << "features array" << std::endl;
                                break;
                            }
                            features.append(f[0]);
                            f++;
                            idx++;
                        }
                        thisPlugin["features"] = features;
                        entryJson["plugins"].append(thisPlugin);
                    });
                res.append(entryJson);
                entry->deinit();
            }
        }
        doc.root["result"] = res;
        return 0;
    }

    if (clap == "")
    {
        std::cout << app.help() << std::endl;
        doc.active = false;
        return 1;
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
    root["clap-version"] = ss.str();

    entry->init(clap.c_str());

    auto fac = (clap_plugin_factory_t *)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    auto plugin_count = fac->get_plugin_count(fac);
    if (plugin_count <= 0)
    {
        std::cerr << "Plugin factory has no plugins" << std::endl;
        doc.active = 0;
        return 4;
    }

    if (descShow && !create)
    {
        // Only loop if we don't create
        root["plugin-count"] = plugin_count;
        Json::Value factory;
        factory.resize(0);
        for (uint32_t pl = 0; pl < plugin_count; ++pl)
        {
            Json::Value pluginDescriptor;
            auto desc = fac->get_plugin_descriptor(fac, pl);

            pluginDescriptor["name"] = desc->name;
            if (desc->version)
                pluginDescriptor["version"] = desc->version;
            pluginDescriptor["id"] = desc->id;
            if (desc->vendor)
                pluginDescriptor["vendor"] = desc->vendor;
            if (desc->description)
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

    if (which_plugin != -1 && (which_plugin < 0 || which_plugin >= (int)plugin_count))
    {
        std::cerr << "Unable to create plugin " << which_plugin << " which must be between 0 and "
                  << plugin_count - 1 << " or be a -1 sentinel" << std::endl;
        doc.active = false;
        return 4;
    }

    int startPlugin = (which_plugin < 0 ? 0 : which_plugin);
    int endPlugin = (which_plugin < 0 ? plugin_count : which_plugin + 1);

    root["plugin_count"] = plugin_count;
    root["plugins"] = Json::Value();

    for (int plug = startPlugin; plug < endPlugin; ++plug)
    {
        auto thisPluginJson = Json::Value();
        auto desc = fac->get_plugin_descriptor(fac, plug);

        if (descShow)
        {
            // Only loop if we don't create
            Json::Value pluginDescriptor;

            pluginDescriptor["name"] = desc->name;
            if (desc->version)
                pluginDescriptor["version"] = desc->version;
            pluginDescriptor["id"] = desc->id;
            if (desc->vendor)
                pluginDescriptor["vendor"] = desc->vendor;
            if (desc->description)
                pluginDescriptor["description"] = desc->description;

            auto f = desc->features;
            while (f[0])
            {
                pluginDescriptor["features"].append(f[0]);
                f++;
            }
            thisPluginJson["descriptor"] = pluginDescriptor;
            thisPluginJson["plugin-index"] = plug;
        }

        // Now lets make an instance
        auto host = clap_info_host::createCLAPInfoHost();
        clap_info_host::getHostConfig()->announceQueriedExtensions = annExt;
        auto inst = fac->create_plugin(fac, host, desc->id);
        if (!inst)
        {
            std::cerr << "Unable to create plugin; inst is null" << std::endl;
            doc.active = false;
            return 5;
        }

        bool result = inst->init(inst);
        if (!result)
        {
            std::cerr << "Unable to init plugin" << std::endl;
            doc.active = false;
            return 6;
        }
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

        if (otherExt)
        {
            extensions[CLAP_EXT_LATENCY] = clap_info_host::createLatencyJson(inst);
            extensions[CLAP_EXT_TAIL] = clap_info_host::createTailJson(inst);
            extensions[CLAP_EXT_GUI] = clap_info_host::createGuiJson(inst);
            extensions[CLAP_EXT_STATE] = clap_info_host::createStateJson(inst);
            extensions[CLAP_EXT_NOTE_NAME] = clap_info_host::createNoteNameJson(inst);
            extensions[CLAP_EXT_AUDIO_PORTS_CONFIG] =
                clap_info_host::createAudioPortsConfigJson(inst);

            // Some 'is implemented' only ones. This is the
            // entire 1.2.0 list generated with
            // grep -r CLAP_EXT libs/clap/include | grep static | awk '{print $5}' | sed -e
            // 's/\[\]/,/' and then remove the ones handled above by hand
            for (auto ext : {
                     CLAP_EXT_AMBISONIC,
                     CLAP_EXT_AMBISONIC_COMPAT,
                     CLAP_EXT_AUDIO_PORTS_ACTIVATION,
                     CLAP_EXT_AUDIO_PORTS_ACTIVATION_COMPAT,
                     CLAP_EXT_AUDIO_PORTS_CONFIG_INFO,
                     CLAP_EXT_AUDIO_PORTS_CONFIG_INFO_COMPAT,
                     CLAP_EXT_CONFIGURABLE_AUDIO_PORTS,
                     CLAP_EXT_CONFIGURABLE_AUDIO_PORTS_COMPAT,
                     CLAP_EXT_CONTEXT_MENU,
                     CLAP_EXT_CONTEXT_MENU_COMPAT,
                     CLAP_EXT_EVENT_REGISTRY,
                     CLAP_EXT_EXTENSIBLE_AUDIO_PORTS,
                     CLAP_EXT_LOG,
                     CLAP_EXT_PARAM_INDICATION,
                     CLAP_EXT_PARAM_INDICATION_COMPAT,
                     CLAP_EXT_POSIX_FD_SUPPORT,
                     CLAP_EXT_PRESET_LOAD,
                     CLAP_EXT_PRESET_LOAD_COMPAT,
                     CLAP_EXT_REMOTE_CONTROLS,
                     CLAP_EXT_REMOTE_CONTROLS_COMPAT,
                     CLAP_EXT_RENDER,
                     CLAP_EXT_RESOURCE_DIRECTORY,
                     CLAP_EXT_STATE_CONTEXT,
                     CLAP_EXT_SURROUND,
                     CLAP_EXT_SURROUND_COMPAT,
                     CLAP_EXT_THREAD_CHECK,
                     CLAP_EXT_THREAD_POOL,
                     CLAP_EXT_TIMER_SUPPORT,
                     CLAP_EXT_TRACK_INFO,
                     CLAP_EXT_TRACK_INFO_COMPAT,
                     CLAP_EXT_TRANSPORT_CONTROL,
                     CLAP_EXT_TRIGGERS,
                     CLAP_EXT_TUNING,
                     CLAP_EXT_VOICE_INFO,

                 })
            {
                auto exf = inst->get_extension(inst, ext);
                Json::Value r;
                r["implemented"] = exf ? true : false;
                extensions[ext] = r;
            }
        }

        thisPluginJson["extensions"] = extensions;
        root["plugins"].append(thisPluginJson);

        inst->deactivate(inst);
        inst->destroy(inst);
    }

    entry->deinit();

    return 0;
}
