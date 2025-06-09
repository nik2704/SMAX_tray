#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "TextureLoader.h"
#include <windows.h>
#include <GL/gl.h>
#include <iostream>

// Load image data from resource into memory buffer
unsigned char* LoadImageFromResource(int resourceID, int& outWidth, int& outHeight, int& outChannels) {
    HMODULE hModule = GetModuleHandle(NULL);
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    if (!hResource) return nullptr;

    HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
    if (!hLoadedResource) return nullptr;

    DWORD size = SizeofResource(hModule, hResource);
    void* pResourceData = LockResource(hLoadedResource);
    if (!pResourceData) return nullptr;

    // Decode image from memory buffer with stb_image
    unsigned char* data = stbi_load_from_memory(
        (const stbi_uc*)pResourceData,
        size,
        &outWidth,
        &outHeight,
        &outChannels,
        4  // force 4 channels (RGBA)
    );
    return data;
}

// Then create OpenGL texture from this data:
ImTextureID LoadTextureFromResource(int resourceID) {
    int width, height, channels;
    unsigned char* data = LoadImageFromResource(resourceID, width, height, channels);
    if (!data) {
        std::cerr << "Failed to load image resource ID " << resourceID << std::endl;
        return (ImTextureID)0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    return (ImTextureID)(intptr_t)textureID;
}

ImTextureID LoadTextureFromFile(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image: " << filename << std::endl;
        return (ImTextureID)0;
    }

    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data
    );

    stbi_image_free(data);

    return static_cast<ImTextureID>(textureID);
}

