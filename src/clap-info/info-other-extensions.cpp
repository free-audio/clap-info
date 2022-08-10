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

#include "info.h"

#include <clap/ext/latency.h>
#include <clap/ext/gui.h>
#include <clap/ext/state.h>

namespace clap_info_host
{

Json::Value unimpl(const Json::Value &v)
{
    if (v.type() == Json::nullValue)
    {
        Json::Value r;
        r["implemented"] = false;
        return r;
    }
    return v;
}

Json::Value createLatencyJson(const clap_plugin *inst)
{
    auto inst_latency = (clap_plugin_latency_t *)inst->get_extension(inst, CLAP_EXT_LATENCY);

    Json::Value res;
    if (inst_latency)
    {
        res["implemented"] = true;
        res["latency"] = inst_latency->get(inst);
    }
    return unimpl(res);
}

Json::Value createTailJson(const clap_plugin *inst)
{
    auto inst_tail = (clap_plugin_tail_t *)inst->get_extension(inst, CLAP_EXT_TAIL);

    Json::Value res;
    if (inst_tail)
    {
        res["implemented"] = true;
        res["tail"] = inst_tail->get(inst);
    }
    return unimpl(res);
}

Json::Value createGuiJson(const clap_plugin *inst)
{
    auto inst_gui = (clap_plugin_gui_t *)inst->get_extension(inst, CLAP_EXT_GUI);

    Json::Value res;
    if (inst_gui)
    {
        res["implemented"] = true;
        res["api_supported"] = Json::Value();
        for (const auto &api : {CLAP_WINDOW_API_COCOA, CLAP_WINDOW_API_WIN32, CLAP_WINDOW_API_X11,
                                CLAP_WINDOW_API_WAYLAND})
        {
            if (inst_gui->is_api_supported(inst, api, false))
                res["api_supported"].append(api);
            if (inst_gui->is_api_supported(inst, api, true))
                res["api_supported"].append(std::string(api) + ".floating");
        }

        const char *prefA{nullptr};
        bool fl{false};
        if (inst_gui->get_preferred_api(inst, &prefA, &fl) && prefA)
        {
            res["preferred_api"] = Json::Value();
            res["preferred_api"]["api"] = prefA;
            res["preferred_api"]["floating"] = fl;
        }
    }
    return unimpl(res);
}

Json::Value createStateJson(const clap_plugin *inst)
{
    auto inst_state = (clap_plugin_state_t *)inst->get_extension(inst, CLAP_EXT_STATE);
    Json::Value res;
    if (inst_state)
    {
        res["implemented"] = true;

        clap_ostream_t os;
        uint64_t written{0};

        os.ctx = &written;
        os.write = [](const struct clap_ostream *stream, const void *buffer,
                      uint64_t size) -> int64_t {
            uint64_t *c = static_cast<uint64_t *>(stream->ctx);
            *(c) += size;
            return size;
        };
        inst_state->save(inst, &os);
        res["bytes-written"] = written;
    }
    return unimpl(res);
}

Json::Value createNoteNameJson(const clap_plugin *inst)
{
    auto inst_notename =
        (clap_plugin_note_name /*_t*/ *)inst->get_extension(inst, CLAP_EXT_NOTE_NAME);

    Json::Value res;
    if (inst_notename)
    {
        res["implemented"] = true;
        auto ct = inst_notename->count(inst);
        res["count"] = ct;
        if (ct > 0)
        {
            Json::Value names;
            for (uint32_t i = 0; i < inst_notename->count(inst); ++i)
            {
                clap_note_name_t nn;
                inst_notename->get(inst, i, &nn);
                Json::Value jn;
                jn["name"] = nn.name;
                jn["port"] = nn.port;
                jn["key"] = nn.key;
                jn["channel"] = nn.channel;
                names.append(jn);
            }
            res["note_names"] = names;
        }
    }
    return unimpl(res);
}

Json::Value createAudioPortsConfigJson(const clap_plugin *inst)
{
    auto inst_apc =
        (clap_plugin_audio_ports_config_t *)inst->get_extension(inst, CLAP_EXT_AUDIO_PORTS_CONFIG);

    Json::Value res;
    if (inst_apc)
    {
        res["implemented"] = true;
        auto ct = inst_apc->count(inst);
        res["count"] = ct;
        if (ct > 0)
        {
            Json::Value cfgs;
            for (uint32_t i = 0; i < inst_apc->count(inst); ++i)
            {
                clap_audio_ports_config apc;
                inst_apc->get(inst, i, &apc);
                Json::Value jn;
                jn["id"] = apc.id;
                jn["name"] = apc.name;
                jn["input-port-count"] = apc.input_port_count;
                jn["output-port-count"] = apc.output_port_count;

                jn["has-main-input"] = apc.has_main_input;
                jn["main-input-channel-count"] = apc.main_input_channel_count;
                jn["main-input-port_type"] = apc.main_input_port_type;

                jn["has-main-output"] = apc.has_main_output;
                jn["main-output-channel-count"] = apc.main_output_channel_count;
                jn["main-output-port_type"] = apc.main_output_port_type;
                cfgs.append(jn);
            }
            res["configs"] = cfgs;
        }
    }
    return unimpl(res);
}
} // namespace clap_info_host
