#pragma once

#include "imgui.h"

unsigned char* LoadImageFromResource(int resourceID, int& outWidth, int& outHeight, int& outChannels);
ImTextureID LoadTextureFromResource(int resourceID);
ImTextureID LoadTextureFromFile(const char* filename);
