#pragma once

#include <windows.h>
#include <string>
#include "../Utils/SimpleIni.h"

namespace smax {

class TokenInitializer {
public:
    static void initializeToken(const std::wstring& iniPath);

private:
    static std::wstring promptInput(const std::wstring& message, const std::wstring& title);
};

} // namespace smax
