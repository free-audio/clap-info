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

#ifndef CLAP_INFO_SCANNER_H
#define CLAP_INFO_SCANNER_H

#include <vector>
#include <filesystem>
#include <functional>

#include <clap/clap.h>

namespace clap_scanner
{
/*
 * Return a list of the valid CLAP search paths, per the spec
 * in entry.h, including parsing the appropriate environment.
 */
std::vector<std::filesystem::path> validCLAPSearchPaths();

/*
 * Traverse the valid CLAP search paths looking for OS specific
 * paths to .clap instances
 */
std::vector<std::filesystem::path> installedCLAPs();

/*
 * Given the path of a CLAP instance, undertake the OS specific code
 * to open the clap and resolve the clap_plugin_entry_t *, which can return
 * null in the case of an invalid CLAP file
 */
const clap_plugin_entry_t *entryFromCLAPPath(const std::filesystem::path &p);

/*
 * Given a clap_plugin_entry_t, visit each clap description available
 * inside the CLAP with a std::function
 */
bool foreachCLAPDescription(const clap_plugin_entry_t *, std::function<void(const clap_plugin_descriptor_t *)> );

/*
 * Given a path to a CLAP, visit each clap description available inside the
 * CLAP with a std::function
 */
inline bool foreachCLAPDescription(const std::filesystem::path &p, std::function<void(const clap_plugin_descriptor_t *)> cb)
{
    auto e = entryFromCLAPPath(p);
    if (e)
    {
        return foreachCLAPDescription(e, cb);
    }
    return false;
}


}

#endif // CLAP_INFO_SCANNER_H
