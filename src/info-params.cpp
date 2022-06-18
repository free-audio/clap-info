//
// Created by Paul Walker on 6/17/22.
//

#include <iostream>
#include <iomanip>

#include <clap/ext/params.h>

#include "info.h"

namespace clap_info_host
{
void showParams(const clap_plugin *inst)
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


            std::cout  << std::setw(4) << i << " " << inf.module << " " << inf.name << " (id=0x"
                      << std::hex << inf.id << std::dec << ")\n    "
                      << " min/max/def=" << inf.min_value << "/"
                      << inf.max_value << "/" << inf.default_value;
            double d;
            inst_param->get_value(inst, inf.id, &d);
            std::cout << " val=" << d << " flags=";

            std::string pre = "";
            auto cp = [&inf, &pre](auto x, auto y) {
                    if (inf.flags & x)
                    {
                        std::cout << pre << y;
                        pre = ",";
                    }
                };
            cp(CLAP_PARAM_IS_STEPPED, "stepped" );
            cp(CLAP_PARAM_IS_PERIODIC, "periodic");
            cp (CLAP_PARAM_IS_HIDDEN, "hidden");
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
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "Parameters: Plugin does not implement '" << CLAP_EXT_PARAMS << "'" << std::endl;
    }
}
}