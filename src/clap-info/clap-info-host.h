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

#ifndef CLAP_INFO_HOST_CLAP_INFO_HOST_H
#define CLAP_INFO_HOST_CLAP_INFO_HOST_H

#include <filesystem>
#include <cassert>
#include <unordered_map>
#include <list>
#include <cstring>

#include "clap/entry.h"
#include "clap/host.h"
#include "clap/events.h"

#include "clap/ext/params.h"
#include "clap/ext/audio-ports.h"
#include <vector>

namespace clap_info_host
{
struct HostConfig
{
  public:
    bool announceQueriedExtensions{true};
};

clap_host_t *createCLAPInfoHost();
HostConfig *getHostConfig();

} // namespace clap_info_host

#endif // CLAP_INFO_HOST_CLAP_INFO_HOST_H
