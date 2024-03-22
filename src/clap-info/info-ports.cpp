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

#include "clap/ext/audio-ports.h"
#include "clap/ext/note-ports.h"

#include "info.h"

#include "json/json.h"

namespace clap_info_host
{

Json::Value createAudioPortsJson(const clap_plugin *inst)
{
    auto inst_ports = (clap_plugin_audio_ports_t *)inst->get_extension(inst, CLAP_EXT_AUDIO_PORTS);
    int inPorts{0}, outPorts{0};

    Json::Value audioPorts;
    if (inst_ports)
    {
        audioPorts["implemented"] = true;
        inPorts = inst_ports->count(inst, true);
        audioPorts["input-port-count"] = inPorts;
        outPorts = inst_ports->count(inst, false);
        audioPorts["output-port-count"] = outPorts;

        auto makeFlags = [](const auto &p) {
            Json::Value flag;
            flag["value"] = p;
            Json::Value desc;

#define ADDF(x)                                                                                    \
    if (p & x)                                                                                     \
    {                                                                                              \
        desc.append(#x);                                                                           \
    }
            ADDF(CLAP_AUDIO_PORT_IS_MAIN);
            ADDF(CLAP_AUDIO_PORT_SUPPORTS_64BITS);
            ADDF(CLAP_AUDIO_PORT_PREFERS_64BITS);
            ADDF(CLAP_AUDIO_PORT_REQUIRES_COMMON_SAMPLE_SIZE);
#undef ADDF

            if (desc.size() != 0)
            {
                flag["fields"] = desc;
            }
            return flag;
        };

        Json::Value inputPorts;
        inputPorts.resize(0);
        for (int i = 0; i < inPorts; ++i)
        {
            clap_audio_port_info_t inf;
            inst_ports->get(inst, i, true, &inf);
            Json::Value inputPort;
            inputPort["name"] = inf.name;
            inputPort["id"] = inf.id;
            inputPort["channel-count"] = inf.channel_count;
            if (inf.port_type)
            {
                inputPort["port-type"] = inf.port_type;
            }
            inputPort["flags"] = makeFlags(inf.flags);

            if (inf.in_place_pair != CLAP_INVALID_ID)
            {
                inputPort["in-place-pair"] = inf.in_place_pair;
            }
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
            outputPort["id"] = inf.id;
            outputPort["channel-count"] = inf.channel_count;
            if (inf.port_type)
            {
                outputPort["port-type"] = inf.port_type;
            }

            outputPort["flags"] = makeFlags(inf.flags);

            if (inf.in_place_pair != CLAP_INVALID_ID)
            {
                outputPort["in-place-pair"] = inf.in_place_pair;
            }
            outputPorts.append(outputPort);
        }
        audioPorts["output-ports"] = outputPorts;
    }
    else
    {
        audioPorts["implemented"] = false;
    }
    return audioPorts;
}

Json::Value createNotePortsJson(const clap_plugin *inst)
{
    auto inst_ports = (clap_plugin_note_ports_t *)inst->get_extension(inst, CLAP_EXT_NOTE_PORTS);
    int inPorts{0}, outPorts{0};

    Json::Value notePorts;
    if (inst_ports)
    {
        notePorts["implemented"] = true;
        inPorts = inst_ports->count(inst, true);
        notePorts["input-port-count"] = inPorts;
        outPorts = inst_ports->count(inst, false);
        notePorts["output-port-count"] = outPorts;

        auto dial = [](Json::Value &inputPort, auto supported, auto pref) {

#define CHECKD(x)                                                                                  \
    if (supported & x)                                                                             \
    {                                                                                              \
        inputPort["dialects"].append(#x);                                                          \
        if (pref == x)                                                                             \
            inputPort["preferred"] = #x;                                                           \
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
    else
    {
        notePorts["implmeented"] = false;
    }
    return notePorts;
}
} // namespace clap_info_host
