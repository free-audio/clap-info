//
// Created by Paul Walker on 7/9/22.
//

#include "info.h"

#include <clap/ext/latency.h>
#include <clap/ext/gui.h>

namespace clap_info_host
{
Json::Value createLatencyJson(const clap_plugin *inst)
{
    auto inst_latency =
        (clap_plugin_latency_t *)inst->get_extension(inst, CLAP_EXT_LATENCY);

    Json::Value res;
    if (inst_latency)
    {
        res["latency"] = inst_latency->get(inst);
    }
    return res;
}

Json::Value createGuiJson(const clap_plugin *inst)
{
    auto inst_gui =
        (clap_plugin_gui_t *)inst->get_extension(inst, CLAP_EXT_GUI);

    Json::Value res;
    if (inst_gui)
    {
        res["api_supported"] = Json::Value();
        for (const auto &api : { CLAP_WINDOW_API_COCOA, CLAP_WINDOW_API_WIN32, CLAP_WINDOW_API_X11,CLAP_WINDOW_API_WAYLAND})
        {
            if (inst_gui->is_api_supported(inst, api, false))
                res["api_supported"].append(api);
            if (inst_gui->is_api_supported(inst, api, true))
                res["api_supported"].append(std::string(api) + ".floating");
        }

        const char prefA[CLAP_NAME_SIZE]{};
        bool fl{false};
        if (inst_gui->get_preferred_api(inst, (const char**)&prefA, &fl))
        {
            res["preferred_api"] = std::string(prefA) + (fl ? ".floating" : "");
        }
    }
    return res;
}
}
