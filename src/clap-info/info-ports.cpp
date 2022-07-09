//
// Created by Paul Walker on 6/17/22.
//


#include <iostream>
#include <sstream>

#include "clap/ext/audio-ports.h"
#include "clap/ext/note-ports.h"

#include "info.h"

#include "json/json.h"

namespace clap_info_host
{

Json::Value createAudioPortsJson(const clap_plugin *inst)
{
    auto inst_ports =
        (clap_plugin_audio_ports_t *)inst->get_extension(inst, CLAP_EXT_AUDIO_PORTS);
    int inPorts{0}, outPorts{0};

    Json::Value audioPorts;
    if (inst_ports)
    {
        inPorts = inst_ports->count(inst, true);
        audioPorts["input-port-count"] = inPorts;
        outPorts = inst_ports->count(inst, false);
        audioPorts["output-port-count"] = outPorts;

        Json::Value inputPorts;
        inputPorts.resize(0);
        for (int i = 0; i < inPorts; ++i)
        {
            clap_audio_port_info_t inf;
            inst_ports->get(inst, i, true, &inf);
            Json::Value inputPort;
            inputPort["name"] = inf.name;
            inputPort["channel-count"] = inf.channel_count;
            inputPorts.append(inputPort);

        }
        audioPorts["input-ports"] = inputPorts;

        Json::Value outputPorts;
        outputPorts.resize(0);
        for (int i = 0; i < outPorts; ++i)
        {
            clap_audio_port_info_t inf;
            inst_ports->get(inst, i, false, &inf);
            Json::Value outputPort;
            outputPort["name"] = inf.name;
            outputPort["channel-count"] = inf.channel_count;
            outputPorts.append(outputPort);
        }
        audioPorts["output-ports"] = outputPorts;
    }
    return audioPorts;
}

Json::Value createNotePortsJson(const clap_plugin *inst)
{
    auto inst_ports =
        (clap_plugin_note_ports_t *)inst->get_extension(inst, CLAP_EXT_NOTE_PORTS);
    int inPorts{0}, outPorts{0};

    Json::Value notePorts;
    if (inst_ports)
    {
        inPorts = inst_ports->count(inst, true);
        notePorts["input-port-count"] = inPorts;
        outPorts = inst_ports->count(inst, false);
        notePorts["output-port-count"] = outPorts;

        auto dial = [](Json::Value& inputPort, auto supported, auto pref) {

#define CHECKD(x) \
                if (supported & x) {\
                  inputPort["dialects"].append(#x); \
                  if (pref == x)     \
                    inputPort["preferred"] = #x;   \
                }

            CHECKD(CLAP_NOTE_DIALECT_CLAP);
            CHECKD(CLAP_NOTE_DIALECT_MIDI);
            CHECKD(CLAP_NOTE_DIALECT_MIDI_MPE);
            CHECKD(CLAP_NOTE_DIALECT_MIDI2);

#undef CHECKD

        };
        Json::Value inputPorts;
        inputPorts.resize(0);
        for (int i = 0; i < inPorts; ++i)
        {
            clap_note_port_info_t inf;
            inst_ports->get(inst, i, true, &inf);
            Json::Value inputPort;
            inputPort["id"] = inf.id;
            inputPort["name"] = inf.name;
            dial(inputPort, inf.supported_dialects, inf.preferred_dialect);
            inputPorts.append(inputPort);
        }
        notePorts["input-ports"] = inputPorts;

        Json::Value outputPorts;
        outputPorts.resize(0);
        for (int i = 0; i < outPorts; ++i)
        {
            clap_note_port_info_t inf;
            inst_ports->get(inst, i, false, &inf);
            Json::Value outputPort;
            outputPort["id"] = inf.id;
            outputPort["name"] = inf.name;
            dial(outputPort, inf.supported_dialects, inf.preferred_dialect);
            outputPorts.append(outputPort);
        }
        notePorts["output-ports"] = outputPorts;
    }
    return notePorts;
}
}