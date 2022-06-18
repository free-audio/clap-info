//
// Created by Paul Walker on 6/17/22.
//


#include <iostream>
#include <sstream>

#include <clap/ext/audio-ports.h>
#include <clap/ext/note-ports.h>


#include "info.h"


namespace clap_info_host
{
void showAudioPorts(const clap_plugin *inst)
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
void showNotePorts(const clap_plugin *inst)
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
}