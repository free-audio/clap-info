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
#include <iomanip>

#include "clap/ext/params.h"

#include "info.h"

namespace clap_info_host
{
Json::Value createParamsJson(const clap_plugin *inst)
{
    auto inst_param = (clap_plugin_params_t *)inst->get_extension(inst, CLAP_EXT_PARAMS);

    Json::Value pluginParams;
    if (inst_param)
    {
        auto pc = inst_param->count(inst);
        pluginParams["implemented"] = true;
        pluginParams["param-count"] = pc;

        Json::Value instParams;
        instParams.resize(0);
        for (auto i = 0U; i < pc; ++i)
        {
            clap_param_info_t inf;
            inst_param->get_info(inst, i, &inf);

            Json::Value instParam;

            std::stringstream ss;
            ss << "0x" << std::hex << inf.id;
            instParam["id"] = ss.str();

            instParam["module"] = inf.module;
            instParam["name"] = inf.name;

            Json::Value values;
            values["min"] = inf.min_value;
            values["max"] = inf.max_value;
            values["default"] = inf.default_value;
            double d;
            inst_param->get_value(inst, inf.id, &d);
            values["current"] = d;
            instParam["values"] = values;

            auto cp = [&inf, &instParam](auto x, auto y) {
                if (inf.flags & x)
                {
                    instParam["flags"].append(y);
                }
            };
            cp(CLAP_PARAM_IS_STEPPED, "stepped");
            cp(CLAP_PARAM_IS_PERIODIC, "periodic");
            cp(CLAP_PARAM_IS_HIDDEN, "hidden");
            cp(CLAP_PARAM_IS_READONLY, "readonly");
            cp(CLAP_PARAM_IS_BYPASS, "bypass");

            cp(CLAP_PARAM_IS_AUTOMATABLE, "auto");
            cp(CLAP_PARAM_IS_AUTOMATABLE_PER_NOTE_ID, "auto-noteid");
            cp(CLAP_PARAM_IS_AUTOMATABLE_PER_KEY, "auto-key");
            cp(CLAP_PARAM_IS_AUTOMATABLE_PER_CHANNEL, "auto-channel");
            cp(CLAP_PARAM_IS_AUTOMATABLE_PER_PORT, "auto-port");

            cp(CLAP_PARAM_IS_MODULATABLE, "mod");
            cp(CLAP_PARAM_IS_MODULATABLE_PER_NOTE_ID, "mod-noteid");
            cp(CLAP_PARAM_IS_MODULATABLE_PER_KEY, "mod-key");
            cp(CLAP_PARAM_IS_MODULATABLE_PER_CHANNEL, "mod-channel");
            cp(CLAP_PARAM_IS_MODULATABLE_PER_PORT, "mod-port");

            cp(CLAP_PARAM_REQUIRES_PROCESS, "requires-process");
            instParams.append(instParam);
        }
        pluginParams["param-info"] = instParams;
    }
    else
    {
        pluginParams["implemented"] = false;
    }
    return pluginParams;
}
} // namespace clap_info_host