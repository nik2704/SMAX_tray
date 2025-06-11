/**
 * @file ImageLoader.h
 * @brief Functions for loading images and textures for ImGui from resources or files.
 */

#pragma once

#include "imgui.h"

/**
 * @brief Loads an image from a resource embedded in the executable.
 * 
 * @param resourceID The identifier of the resource containing the image data.
 * @param[out] outWidth Receives the width of the loaded image.
 * @param[out] outHeight Receives the height of the loaded image.
 * @param[out] outChannels Receives the number of color channels in the image.
 * @return Pointer to the loaded image data as an array of unsigned chars. Must be freed by the caller.
 */
unsigned char* LoadImageFromResource(int resourceID, int& outWidth, int& outHeight, int& outChannels);

/**
 * @brief Loads a texture usable by ImGui from an image resource embedded in the executable.
 * 
 * @param resourceID The identifier of the resource containing the image data.
 * @return ImTextureID handle to the loaded texture.
 */
ImTextureID LoadTextureFromResource(int resourceID);

/**
 * @brief Loads a texture usable by ImGui from an image file on disk.
 * 
 * @param filename Path to the image file.
 * @return ImTextureID handle to the loaded texture.
 */
ImTextureID LoadTextureFromFile(const char* filename);
