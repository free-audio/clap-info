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
#include "clap-info-host.h"

namespace clap_info_host
{

static std::unique_ptr<HostConfig> static_host_config;
const void *get_extension(const struct clap_host *host, const char *eid)
{
    if (static_host_config->announceQueriedExtensions)
        std::cout << "    Plugin->Host : Requesting Extension " << eid << std::endl;
    return nullptr;
}

void request_restart(const struct clap_host *h) {}

void request_process(const struct clap_host *h) {}

void request_callback(const struct clap_host *h) {}

static clap_host clap_info_host_static{CLAP_VERSION_INIT,
                                       nullptr,
                                       "clap-info",
                                       "CLAP team",
                                       "https://github.com/free-audio/clap-info",
                                       "1.0.0",
                                       get_extension,
                                       request_restart,
                                       request_process,
                                       request_callback};

clap_host_t *createCLAPInfoHost()
{
    if (!static_host_config)
        static_host_config = std::make_unique<HostConfig>();
    return &clap_info_host_static;
}

HostConfig *getHostConfig()
{
    if (!static_host_config)
    {
        std::cout << "Please call createCLAPInfoHost before getHostConfig()";
        return nullptr;
    }
    return static_host_config.get();
}

} // namespace clap_info_host