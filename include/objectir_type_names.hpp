#pragma once

#include "objectir_runtime.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace ObjectIR::TypeNames {

inline std::string Trim(std::string_view input) {
    size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
        ++start;
    }

    size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }

    return std::string(input.substr(start, end - start));
}

inline std::string ToLowerAscii(std::string_view input) {
    std::string out;
    out.reserve(input.size());
    for (char c : input) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

inline std::string NormalizeTypeName(std::string_view rawName) {
    auto trimmed = Trim(rawName);
    if (trimmed.empty()) return trimmed;

    const auto lower = ToLowerAscii(trimmed);

    // Fully-qualified primitive CLR names + common aliases.
    if (lower == "system.void" || lower == "void") return "void";
    if (lower == "system.string" || lower == "string") return "string";
    if (lower == "system.boolean" || lower == "bool" || lower == "boolean") return "bool";

    if (lower == "system.int32" || lower == "int32" || lower == "int") return "int32";
    if (lower == "system.int64" || lower == "int64" || lower == "long") return "int64";

    // Canonical float spellings are float32/float64.
    if (lower == "system.single" || lower == "single" || lower == "float" || lower == "float32") return "float32";
    if (lower == "system.double" || lower == "double" || lower == "float64") return "float64";

    if (lower == "system.byte" || lower == "byte" || lower == "uint8") return "uint8";

    if (lower == "system.object" || lower == "object") return "object";

    // Non-primitive: preserve original spelling (class names are case-sensitive).
    return trimmed;
}

inline std::vector<std::string> NormalizeTypeNames(const std::vector<std::string>& rawNames) {
    std::vector<std::string> normalized;
    normalized.reserve(rawNames.size());
    for (const auto& n : rawNames) {
        normalized.push_back(NormalizeTypeName(n));
    }
    return normalized;
}

inline std::string GetQualifiedClassName(const ClassRef& cls) {
    if (!cls) return {};

    // Treat Class::GetName() as potentially already-qualified.
    const std::string& rawName = cls->GetName();
    const auto dot = rawName.find_last_of('.');
    const std::string simpleName = (dot == std::string::npos) ? rawName : rawName.substr(dot + 1);

    const std::string& ns = cls->GetNamespace();
    if (ns.empty()) {
        // If name already contains dots, keep it; else just simple.
        return rawName;
    }
    return ns + "." + simpleName;
}

inline std::string CanonicalTypeName(const TypeReference& type) {
    if (type.IsPrimitive()) {
        switch (type.GetPrimitiveType()) {
            case PrimitiveType::Int32: return "int32";
            case PrimitiveType::Int64: return "int64";
            case PrimitiveType::Float32: return "float32";
            case PrimitiveType::Float64: return "float64";
            case PrimitiveType::Bool: return "bool";
            case PrimitiveType::Void: return "void";
            case PrimitiveType::String: return "string";
            case PrimitiveType::UInt8: return "uint8";
            case PrimitiveType::Object: return "object";
            default: return "object";
        }
    }

    // Class-backed object types.
    if (auto cls = type.GetClassType()) {
        return GetQualifiedClassName(cls);
    }

    return "object";
}

} // namespace ObjectIR::TypeNames
