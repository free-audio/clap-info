/*
 * CLAP-INFO
 *
 * https://github.com/free-audio/clap-info
 *
 * CLAP-INFO is Free and Open Source software, released under the MIT
 * License, a copy of which is included with this source in the file
 * "LICENSE.md"
 *
 * Copyright (c) 2022-2026 Various Authors, per the Git Transaction Log
 */

#include "info.h"
#include "clap/factory/preset-discovery.h"
#include <vector>
#include <string>
#include <filesystem>

namespace clap_info_host
{

struct ProviderContext
{
    Json::Value &json;
    bool debugInfo;

    struct Location
    {
        uint32_t flags;
        std::string name;
        uint32_t kind;
        std::string location;
    };
    std::vector<Location> locations;

    struct FileType
    {
        std::string name;
        std::string description;
        std::string file_extension;
    };
    std::vector<FileType> fileTypes;

    ProviderContext(Json::Value &j, bool d) : json(j), debugInfo(d) {}
};

static bool declare_filetype(const clap_preset_discovery_indexer_t *indexer,
                             const clap_preset_discovery_filetype_t *filetype)
{
    auto *ctx = static_cast<ProviderContext *>(indexer->indexer_data);
    ctx->fileTypes.push_back({filetype->name, filetype->description ? filetype->description : "",
                              filetype->file_extension ? filetype->file_extension : ""});
    return true;
}

static bool declare_location(const clap_preset_discovery_indexer_t *indexer,
                             const clap_preset_discovery_location_t *location)
{
    auto *ctx = static_cast<ProviderContext *>(indexer->indexer_data);
    ctx->locations.push_back({location->flags, location->name, location->kind,
                              location->location ? location->location : ""});
    return true;
}

static bool declare_soundpack(const clap_preset_discovery_indexer_t *indexer,
                              const clap_preset_discovery_soundpack_t *soundpack)
{
    return true;
}

static const void *get_indexer_extension(const clap_preset_discovery_indexer_t *indexer,
                                         const char *extension_id)
{
    return nullptr;
}

struct ReceiverContext
{
    Json::Value &presets;
    Json::Value currentPreset;

    ReceiverContext(Json::Value &p) : presets(p) {}

    void finalize_preset()
    {
        if (!currentPreset.isNull())
        {
            presets.append(currentPreset);
            currentPreset = Json::Value();
        }
    }
};

static void on_error(const struct clap_preset_discovery_metadata_receiver *receiver,
                     int32_t os_error, const char *error_message)
{
}

static bool begin_preset(const struct clap_preset_discovery_metadata_receiver *receiver,
                         const char *name, const char *load_key)
{
    auto *ctx = static_cast<ReceiverContext *>(receiver->receiver_data);
    ctx->finalize_preset();
    ctx->currentPreset["name"] = name ? name : "";
    ctx->currentPreset["load_key"] = load_key ? load_key : "";
    return true;
}

static void add_plugin_id(const struct clap_preset_discovery_metadata_receiver *receiver,
                          const clap_universal_plugin_id_t *plugin_id)
{
    auto *ctx = static_cast<ReceiverContext *>(receiver->receiver_data);
    Json::Value pid;
    pid["abi"] = plugin_id->abi;
    pid["id"] = plugin_id->id;
    ctx->currentPreset["plugin_ids"].append(pid);
}

static void set_soundpack_id(const struct clap_preset_discovery_metadata_receiver *receiver,
                             const char *soundpack_id)
{
    auto *ctx = static_cast<ReceiverContext *>(receiver->receiver_data);
    ctx->currentPreset["soundpack_id"] = soundpack_id;
}

static void set_flags(const struct clap_preset_discovery_metadata_receiver *receiver,
                      uint32_t flags)
{
    auto *ctx = static_cast<ReceiverContext *>(receiver->receiver_data);
    ctx->currentPreset["flags"] = flags;
}

static void add_creator(const struct clap_preset_discovery_metadata_receiver *receiver,
                        const char *creator)
{
    auto *ctx = static_cast<ReceiverContext *>(receiver->receiver_data);
    ctx->currentPreset["creators"].append(creator);
}

static void set_description(const struct clap_preset_discovery_metadata_receiver *receiver,
                            const char *description)
{
    auto *ctx = static_cast<ReceiverContext *>(receiver->receiver_data);
    ctx->currentPreset["description"] = description;
}

static void set_timestamps(const struct clap_preset_discovery_metadata_receiver *receiver,
                           clap_timestamp creation_time, clap_timestamp modification_time)
{
    auto *ctx = static_cast<ReceiverContext *>(receiver->receiver_data);
    ctx->currentPreset["creation_time"] = (Json::Int64)creation_time;
    ctx->currentPreset["modification_time"] = (Json::Int64)modification_time;
}

static void add_feature(const struct clap_preset_discovery_metadata_receiver *receiver,
                        const char *feature)
{
    auto *ctx = static_cast<ReceiverContext *>(receiver->receiver_data);
    ctx->currentPreset["features"].append(feature);
}

static void add_extra_info(const struct clap_preset_discovery_metadata_receiver *receiver,
                           const char *key, const char *value)
{
    auto *ctx = static_cast<ReceiverContext *>(receiver->receiver_data);
    ctx->currentPreset["extra_info"][key] = value;
}

void crawl(const clap_preset_discovery_provider_t *provider, uint32_t kind,
           const std::string &location, Json::Value &out)
{
    clap_preset_discovery_metadata_receiver_t rec;
    ReceiverContext recCtx(out["presets"]);
    rec.receiver_data = &recCtx;
    rec.on_error = on_error;
    rec.begin_preset = begin_preset;
    rec.add_plugin_id = add_plugin_id;
    rec.set_soundpack_id = set_soundpack_id;
    rec.set_flags = set_flags;
    rec.add_creator = add_creator;
    rec.set_description = set_description;
    rec.set_timestamps = set_timestamps;
    rec.add_feature = add_feature;
    rec.add_extra_info = add_extra_info;

    provider->get_metadata(provider, kind, location.empty() ? nullptr : location.c_str(), &rec);
    recCtx.finalize_preset();
}

void scanProvider(const clap_preset_discovery_factory_t *fac,
                  const clap_preset_discovery_provider_descriptor_t *desc,
                  Json::Value &providersJson, bool debugInfo)
{
    ProviderContext ctx(providersJson, debugInfo);
    clap_preset_discovery_indexer_t indexer;
    indexer.clap_version = CLAP_VERSION;
    indexer.name = "clap-info";
    indexer.vendor = "clap-info";
    indexer.url = "https://github.com/free-audio/clap-info";
    indexer.version = "1.0.0";
    indexer.indexer_data = &ctx;
    indexer.declare_filetype = declare_filetype;
    indexer.declare_location = declare_location;
    indexer.declare_soundpack = declare_soundpack;
    indexer.get_extension = get_indexer_extension;

    auto *provider = fac->create(fac, &indexer, desc->id);
    if (!provider)
        return;

    if (provider->init(provider))
    {
        Json::Value thisProvider;
        thisProvider["name"] = desc->name;
        thisProvider["id"] = desc->id;

        for (const auto &loc : ctx.locations)
        {
            Json::Value l;
            l["name"] = loc.name;
            l["kind"] = loc.kind;
            l["location"] = loc.location;
            l["flags"] = loc.flags;

            if (loc.kind == CLAP_PRESET_DISCOVERY_LOCATION_PLUGIN)
            {
                crawl(provider, loc.kind, "", l);
            }
            else if (loc.kind == CLAP_PRESET_DISCOVERY_LOCATION_FILE)
            {
                if (std::filesystem::is_directory(loc.location))
                {
                    for (auto const &dir_entry :
                         std::filesystem::recursive_directory_iterator(loc.location))
                    {
                        if (dir_entry.is_regular_file())
                        {
                            auto ext = dir_entry.path().extension().string();
                            if (!ext.empty() && ext[0] == '.')
                                ext = ext.substr(1);

                            bool match = ctx.fileTypes.empty();
                            for (const auto &ft : ctx.fileTypes)
                            {
                                if (ft.file_extension == ext || ft.file_extension == "")
                                {
                                    match = true;
                                    break;
                                }
                            }

                            if (match)
                            {
                                crawl(provider, loc.kind, dir_entry.path().string(), l);
                            }
                        }
                    }
                }
                else if (std::filesystem::is_regular_file(loc.location))
                {
                    crawl(provider, loc.kind, loc.location, l);
                }
            }
            thisProvider["locations"].append(l);
        }
        providersJson.append(thisProvider);
    }
    provider->destroy(provider);
}

Json::Value presetDiscovery(const clap_plugin_entry_t *entry)
{
    auto res = Json::Value();
    auto fac =
        (clap_preset_discovery_factory_t *)entry->get_factory(CLAP_PRESET_DISCOVERY_FACTORY_ID);
    res["implements-preset-discovery"] = (fac != nullptr);
    if (!fac)
        return res;

    uint32_t count = fac->count(fac);
    for (uint32_t i = 0; i < count; ++i)
    {
        auto *desc = fac->get_descriptor(fac, i);
        scanProvider(fac, desc, res["providers"], false);
    }

    return res;
}
} // namespace clap_info_host