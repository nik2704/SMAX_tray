/**
 * @file Link.h
 * @brief Defines the Link structure representing a hyperlink or reference in SMAX context.
 */

#pragma once

#include <string>

namespace smax {

/**
 * @struct Link
 * @brief Represents a hyperlink or reference with identifying and descriptive attributes.
 */
struct Link {
    std::string id;    ///< Unique identifier for the link.
    std::string type;  ///< Type or category of the link (e.g., Article, ExternalArticle, ...s).
    std::string label; ///< Human-readable label or title for the link.
    std::string url;   ///< URL or address the link points to.
};

} // namespace smax
