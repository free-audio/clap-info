//
// Created by Paul Walker on 6/17/22.
//

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

}
#endif // CLAP_INFO_INFO_TYPES_H
