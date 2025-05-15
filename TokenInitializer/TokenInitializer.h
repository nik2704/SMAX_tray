#pragma once

#include <windows.h>
#include <string>
#include "../Utils/SimpleIni.h"

namespace smax {

struct InputData {
    wchar_t* username;
    wchar_t* token;
};

class TokenInitializer {
public:
    static void initializeToken(const std::wstring& iniPath);

private:
    static std::pair<std::wstring, std::wstring> TokenInitializer::promptInput();
};

} // namespace smax
