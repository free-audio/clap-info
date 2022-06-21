#include <iostream>
#include <filesystem>

#include "clap-info-host.h"

#include <clap/plugin-factory.h>


#include <info.h>

#include <CLI11/CLI11.hpp>

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

    bool annExt{true};
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

    bool searchPath{false};
    app.add_flag( "--search-path", searchPath, "Show the CLAP plugin search paths then exit");

    bool showClaps{false};
    app.add_flag( "-l,--list-clap-files", showClaps, "Show all CLAP files in the search path then exit");

    bool showClapsWithDesc{false};
    app.add_flag( "-s,--scan-clap-files", showClapsWithDesc, "Show all descriptions in all CLAP files in the search path, then exit");

    CLI11_PARSE(app, argc, argv);

    if (searchPath)
    {
        clap_info_host::showCLAPSearchpath();
        exit(0);
    }

    if (showClaps)
    {
        clap_info_host::recurseAndListCLAPSearchpath(clap_info_host::FIND_FILES);
        exit(0);
    }


    if (showClapsWithDesc)
    {
        clap_info_host::recurseAndListCLAPSearchpath(clap_info_host::FIND_DESCRIPTIONS);
        exit(0);
    }

    auto clapPath = std::filesystem::path(clap);
#if MAC
    if (!(std::filesystem::is_directory(clapPath) || std::filesystem::is_regular_file(clapPath)))
    {
        std::cout << "Your file '" << clap << "' is neither a bundle nor a file" << std::endl;
        exit(2);
    }
#else
    if (!std::filesystem::is_regular_file(clapPath))
    {
        std::cout << "Your file '" << clap << "' is not a regular file" << std::endl;
        exit(2);
    }
#endif


    std::cout << "Loading clap        : " << clap << std::endl;
    auto entry = clap_info_host::entryFromClapPath(clapPath);

    if (!entry)
    {
        std::cout << "   clap_entry returned a nullptr\n"
                  << "   either this plugin is not a CLAP or it has exported the incorrect symbol."
                  << std::endl;
        exit(3);
    }

    auto version = entry->clap_version;
    std::cout << "Clap Version        : " << version.major << "." << version.minor << "."
              << version.revision << std::endl;

    entry->init(clap.c_str());

    auto fac = (clap_plugin_factory_t *)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    auto plugin_count = fac->get_plugin_count(fac);
    if (plugin_count <= 0)
    {
        std::cout << "Plugin factory has no plugins" << std::endl;
        exit(4);
    }
    std::cout << "Plugin Count        : " << plugin_count << std::endl;

    if (descShow)
    {
        for (uint32_t pl = 0; pl < plugin_count; ++pl)
        {
            auto desc = fac->get_plugin_descriptor(fac, pl);

            std::cout << "Plugin " << pl << " description: \n"
                      << "   name     : " << desc->name << "\n"
                      << "   version  : " << desc->version << "\n"
                      << "   id       : " << desc->id << "\n"
                      << "   desc     : " << desc->description << "\n"
                      << "   features : ";
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

    if (!create)
        exit(0);

    if (which_plugin < 0 || which_plugin >= plugin_count)
    {
        std::cout << "Unable to create plugin " << which_plugin << " which must be between 0 and " << plugin_count - 1 << std::endl;
        exit(4);
    }

    auto desc = fac->get_plugin_descriptor(fac, which_plugin);
    std::cout << "Creating Plugin " << std::setw(3) << which_plugin << " : " << desc->name << " (" << desc->id << ")" << std::endl;


    // Now lets make an instance
    auto host = clap_info_host::createClapValHost();
    clap_info_host::getHostConfig()->announceQueriedExtensions = annExt;
    auto inst = fac->create_plugin(fac, host, desc->id);

    inst->init(inst);
    inst->activate(inst, 48000, 32, 4096);

    Json::Value root;
    Json::Value extensions;

    if (paramShow)
    {
        clap_info_host::showParams(inst);
    }

    if (audioPorts)
    {
        extensions[CLAP_EXT_AUDIO_PORTS] = clap_info_host::showAudioPorts(inst);
    }

    if (notePorts)
    {
        extensions[CLAP_EXT_NOTE_PORTS] = clap_info_host::showNotePorts(inst);
    }

    root["extensions"] = extensions;

    Json::StyledWriter writer;
    std::string out_string = writer.write(root);
    std::cout << out_string << std::endl;

    inst->deactivate(inst);
    inst->destroy(inst);

    entry->deinit();
}
