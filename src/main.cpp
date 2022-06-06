#include <iostream>
#include <filesystem>

#include "clap-val-host.h"

#include <clap/plugin-factory.h>
#include <clap/ext/params.h>
#include <clap/ext/audio-ports.h>

#include <unordered_map>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "USAGE: " << argv[0] << " path-to-clap" << std::endl;
        exit(1);
    }
    std::string clap = argv[1];
    auto clapPath = std::filesystem::path(clap);

    if (!(std::filesystem::is_directory(clapPath) || std::filesystem::is_regular_file(clapPath)))
    {
        std::cout << "Your file '" << clap << "' is neither a bundle nor a file" << std::endl;
        exit(2);
    }

    std::cout << "Loading clap        : " << clap << std::endl;

    auto entry = clap_val_host::entryFromClapPath(clapPath);

    if (!entry)
    {
        std::cout << "Got a null entru " << std::endl;
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

    // FIXME - what about multiplugin? for now just grab 0
    auto desc = fac->get_plugin_descriptor(fac, 0);

    std::cout << "Plugin description: \n"
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

    // Now lets make an instance
    auto host = clap_val_host::createClapValHost();
    auto inst = fac->create_plugin(fac, host, desc->id);


    inst->init(inst);
    inst->activate(inst, 48000, 32, 4096);

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

            std::cout << i << " " << inf.module << " " << inf.name << " (id=0x" << std::hex
                      << inf.id << std::dec << ") val=" << d << std::endl;
        }
    }
    else
    {
        std::cout << "No Parameters Available" << std::endl;
    }

    auto inst_ports = (clap_plugin_audio_ports_t *)inst->get_extension(inst, CLAP_EXT_AUDIO_PORTS);
    int inPorts{0}, outPorts{0};
    if (inst_ports)
    {
        inPorts = inst_ports->count(inst, true);
        outPorts = inst_ports->count(inst, false);

        // For now fail out if a port isn't stereo
        for (int i = 0; i < inPorts; ++i)
        {
            clap_audio_port_info_t inf;
            inst_ports->get(inst, i, true, &inf);
            if (inf.channel_count != 2)
                std::cout << "ERROR - need channel count 2" << std::endl;
        }
        for (int i = 0; i < outPorts; ++i)
        {
            clap_audio_port_info_t inf;
            inst_ports->get(inst, i, false, &inf);
            if (inf.channel_count != 2)
                std::cout << "ERROR - need channel count 2" << std::endl;
        }
    }
    else
    {
        std::cout << "No ports extension" << std::endl;
    }

    inst->deactivate(inst);
    inst->destroy(inst);

    entry->deinit();
}
