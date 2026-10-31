#pragma once
#include <stddef.h>

namespace marauder {
// Arguments are already tokenized by the CLI. Null/empty arguments are allowed.
const char* studioUnsupportedReason(const char* command, const char* option = "", const char* value = "");
bool studioSettingSupported(const char* name);
}
