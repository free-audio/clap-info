#include <iostream>
#include <filesystem>

#include "clap-info-host.h"

#include <clap/plugin-factory.h>
#include <clap/ext/params.h>
#include <clap/ext/audio-ports.h>
#include <clap/ext/note-ports.h>

#include <CLI11/CLI11.hpp>

int main(int argc, char **argv)
{
    CLI::App app("clap-info: CLAP command line validation tool");

    app.set_version_flag("--version", "0.0.0");
    std::string clap;
    app.add_option("-f,--file,file", clap, "CLAP plugin file location")->required(true);

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

    CLI11_PARSE(app, argc, argv);

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

    if (paramShow)
    {
        auto inst_param = (clap_plugin_params_t *)inst->get_extension(inst, CLAP_EXT_PARAMS);
        if (inst_param)
        {
            auto pc = inst_param->count(inst);
            std::cout << "Plugin has " << pc << " params " << std::endl;

            for (auto i = 0U; i < pc; ++i)
            {
                clap_param_info_t inf;
                inst_param->get_info(inst, i, &inf);

                double d;
                inst_param->get_value(inst, inf.id, &d);

                std::cout << "   " << i << " " << inf.module << " " << inf.name << " (id=0x"
                          << std::hex << inf.id << std::dec << ") val=" << d << std::endl;
            }
        }
        else
        {
            std::cout << "Parameters: Plugin does not implement '" << CLAP_EXT_PARAMS << "'" << std::endl;
        }
    }

    if (audioPorts)
    {
        auto inst_ports =
            (clap_plugin_audio_ports_t *)inst->get_extension(inst, CLAP_EXT_AUDIO_PORTS);
        int inPorts{0}, outPorts{0};
        if (inst_ports)
        {
            inPorts = inst_ports->count(inst, true);
            outPorts = inst_ports->count(inst, false);

            std::cout << "Audio Ports: Plugin has " << inPorts << " input and " << outPorts << " output ports."
                      << std::endl;

            // For now fail out if a port isn't stereo
            for (int i = 0; i < inPorts; ++i)
            {
                clap_audio_port_info_t inf;
                inst_ports->get(inst, i, true, &inf);
                std::cout << "    Input " << i << " : name=" << inf.name
                          << " channels=" << inf.channel_count << std::endl;
            }
            for (int i = 0; i < outPorts; ++i)
            {
                clap_audio_port_info_t inf;
                inst_ports->get(inst, i, false, &inf);
                std::cout << "    Output " << i << " : name=" << inf.name
                          << " channels=" << inf.channel_count << std::endl;
            }
        }
        else
        {
            std::cout << "Audio Ports: Plugin does not implement '" << CLAP_EXT_AUDIO_PORTS << "'" << std::endl;
        }
    }

    if (notePorts)
    {
        auto inst_ports =
            (clap_plugin_note_ports_t *)inst->get_extension(inst, CLAP_EXT_NOTE_PORTS);
        int inPorts{0}, outPorts{0};
        if (inst_ports)
        {
            inPorts = inst_ports->count(inst, true);
            outPorts = inst_ports->count(inst, false);

            std::cout << "Note Ports: Plugin has " << inPorts << " input and " << outPorts << " output ports."
                      << std::endl;

            auto dial = [](auto supported, auto pref) {
                std::ostringstream oss;
                std::string pre = "";
#define CHECKD(x) \
                if (supported & x) {\
                oss << pre << #x; \
                if (pref == x)     \
                oss << " (pref)";   \
                pre = ", ";  \
                }

                CHECKD(CLAP_NOTE_DIALECT_CLAP);
                CHECKD(CLAP_NOTE_DIALECT_MIDI);
                CHECKD(CLAP_NOTE_DIALECT_MIDI_MPE);
                CHECKD(CLAP_NOTE_DIALECT_MIDI2);

#undef CHECKD

                return oss.str();
            };
            for (int i = 0; i < inPorts; ++i)
            {
                clap_note_port_info_t inf;
                inst_ports->get(inst, i, true, &inf);
                std::cout << "    Input " << i << " : name=" << inf.name << " (" << inf.id << ") "
                          << " dialects=" << dial(inf.supported_dialects, inf.preferred_dialect) << std::endl;
            }
            for (int i = 0; i < outPorts; ++i)
            {
                clap_note_port_info_t inf;
                inst_ports->get(inst, i, false, &inf);
                std::cout << "    Output " << i << " : name=" << inf.name << " (" << inf.id << ") "
                          << " dialects=" << dial(inf.supported_dialects, inf.preferred_dialect) << std::endl;
            }
        }
        else
        {
            std::cout << "Note Ports: Plugin does not implement '" << CLAP_EXT_AUDIO_PORTS << "'" << std::endl;
        }
    }

    inst->deactivate(inst);
    inst->destroy(inst);

    entry->deinit();
}
