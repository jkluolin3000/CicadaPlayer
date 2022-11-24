//
// Created by moqi on 2018/4/27.
//

#ifndef FRAMEWORK_HLSMANAGER_H
#define FRAMEWORK_HLSMANAGER_H

#include "PlaylistManager.h"
#include "HLSStream.h"
#include <queue>

typedef uint8_t *(*ParseM3u8KeyFunc)(const char* url, void *userData);
namespace Cicada{
    struct HLSExtension {
        ParseM3u8KeyFunc pParseM3U8Key;
        void *userData;
    };
}

extern Cicada::HLSExtension *hlsExtension;

#endif //FRAMEWORK_HLSMANAGER_H
