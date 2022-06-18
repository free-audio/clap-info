//
// Created by Paul Walker on 6/17/22.
//

#ifndef CLAP_INFO_INFO_TYPES_H
#define CLAP_INFO_INFO_TYPES_H

#include <clap/clap.h>

namespace clap_info_host
{

void showCLAPSearchpath();
enum ScanLevel {
    FIND_FILES,
    FIND_DESCRIPTIONS
};
void recurseAndListCLAPSearchpath(ScanLevel l);


void showParams(const clap_plugin *);
void showAudioPorts(const clap_plugin *);
void showNotePorts(const clap_plugin *);
}
#endif // CLAP_INFO_INFO_TYPES_H
