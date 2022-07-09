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

#ifndef CLAP_INFO_INFO_TYPES_H
#define CLAP_INFO_INFO_TYPES_H

#include "clap/clap.h"

#include "json/json.h"

namespace clap_info_host
{
Json::Value createParamsJson(const clap_plugin *inst);
Json::Value createAudioPortsJson(const clap_plugin *inst);
Json::Value createNotePortsJson(const clap_plugin *inst);
Json::Value createLatencyJson(const clap_plugin *inst);
Json::Value createTailJson(const clap_plugin *inst);
Json::Value createGuiJson(const clap_plugin *inst);
Json::Value createStateJson(const clap_plugin *inst);
Json::Value createNoteNameJson(const clap_plugin *inst);
Json::Value createAudioPortsConfigJson(const clap_plugin *inst);

} // namespace clap_info_host
#endif // CLAP_INFO_INFO_TYPES_H
