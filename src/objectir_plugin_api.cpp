#include "objectir_plugin_api.h"

#include "instruction_executor.hpp"
#include "objectir_type_names.hpp"
#include "objectir_runtime.hpp"

#include <algorithm>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
thread_local std::string g_lastError;

void SetLastError(const std::string& msg) { g_lastError = msg; }
void ClearLastError() { g_lastError.clear(); }

char* CopyToCString(const std::string& source) {
    char* buffer = new (std::nothrow) char[source.size() + 1];
    if (!buffer) return nullptr;
    std::memcpy(buffer, source.c_str(), source.size() + 1);
    return buffer;
}

ObjectIR::ClassRef FindClass(ObjectIR::VirtualMachine* vm, const std::string& className) {
    if (!vm) throw std::runtime_error("vm is null");
    if (className.empty()) throw std::runtime_error("className is empty");

    if (vm->HasClass(className)) {
        return vm->GetClass(className);
    }

    // Fallback: match by suffix, e.g. "HelloWorld.Program" when asked for "Program".
    auto names = vm->GetAllClassNames();
    auto it = std::find_if(names.begin(), names.end(), [&](const std::string& n) {
        if (n == className) return true;
        if (n.size() >= className.size() + 1 && n.rfind("." + className) == (n.size() - (className.size() + 1))) {
            return true;
        }
        return false;
    });

    if (it != names.end()) {
        return vm->GetClass(*it);
    }

    throw std::runtime_error("Class not found: " + className);
}

std::vector<ObjectIR::MethodRef> CollectMethodsByName(ObjectIR::ClassRef cls, const std::string& name) {
    std::vector<ObjectIR::MethodRef> matches;
    for (auto current = cls; current; current = current->GetBaseClass()) {
        for (const auto& method : current->GetAllMethods()) {
            if (method && method->GetName() == name) {
                matches.push_back(method);
            }
        }
    }
    return matches;
}

bool ParameterTypeMatches(const std::string& requestedType, const ObjectIR::TypeReference& parameterType) {
    const auto requestedNorm = ObjectIR::TypeNames::NormalizeTypeName(requestedType);
    const auto paramCanon = ObjectIR::TypeNames::CanonicalTypeName(parameterType);
    if (requestedNorm == paramCanon) return true;

    if (requestedNorm.find('.') == std::string::npos) {
        const auto dot = paramCanon.find_last_of('.');
        const auto simple = (dot == std::string::npos) ? paramCanon : paramCanon.substr(dot + 1);
        return requestedNorm == simple;
    }
    return false;
}

ObjectIR::MethodRef FindMethodBySignatureOrThrow(ObjectIR::ClassRef cls,
                                                const std::string& name,
                                                const std::vector<std::string>& parameterTypes) {
    auto candidates = CollectMethodsByName(cls, name);
    if (candidates.empty()) {
        throw std::runtime_error("Method not found: " + name);
    }

    const auto requested = ObjectIR::TypeNames::NormalizeTypeNames(parameterTypes);
    std::vector<ObjectIR::MethodRef> exact;
    for (const auto& m : candidates) {
        if (!m) continue;
        const auto& params = m->GetParameters();
        if (params.size() != requested.size()) continue;
        bool ok = true;
        for (size_t i = 0; i < params.size(); ++i) {
            if (!ParameterTypeMatches(requested[i], params[i].second)) {
                ok = false;
                break;
            }
        }
        if (ok) {
            exact.push_back(m);
        }
    }

    if (exact.size() == 1) return exact[0];
    if (exact.empty()) {
        throw std::runtime_error("No matching overload for method: " + name);
    }
    throw std::runtime_error("Ambiguous overload for method: " + name);
}

ObjectIR::MethodRef FindMethodUniqueNameOrThrow(ObjectIR::ClassRef cls, const std::string& name) {
    auto candidates = CollectMethodsByName(cls, name);
    if (candidates.empty()) {
        throw std::runtime_error("Method not found: " + name);
    }
    if (candidates.size() != 1) {
        throw std::runtime_error("Ambiguous overload for method: " + name + ". Use signature-qualified API.");
    }
    return candidates[0];
}
} // namespace

extern "C" {

const char* ObjectIR_PluginLastError(void) {
    return g_lastError.c_str();
}

void ObjectIR_PluginFreeString(char* str) {
    delete[] str;
}

uint32_t ObjectIR_RuntimeGetPluginAbiVersionPacked(void) {
    return OBJECTIR_PLUGIN_ABI_VERSION_PACKED;
}

int32_t ObjectIR_PluginGetAllClassNamesJson(ObjectIR_VirtualMachine* vm, char** outJson) {
    if (!vm || !outJson) {
        SetLastError("Invalid arguments to ObjectIR_PluginGetAllClassNamesJson");
        return 0;
    }

    try {
        ClearLastError();
        auto* vmCpp = reinterpret_cast<ObjectIR::VirtualMachine*>(vm);
        auto names = vmCpp->GetAllClassNames();
        ObjectIR::json j = ObjectIR::json::array();
        for (const auto& n : names) j.push_back(n);

        auto s = j.dump();
        *outJson = CopyToCString(s);
        if (!*outJson) {
            SetLastError("Allocation failure");
            return 0;
        }
        return 1;
    } catch (const std::exception& ex) {
        SetLastError(ex.what());
    } catch (...) {
        SetLastError("Unknown error in ObjectIR_PluginGetAllClassNamesJson");
    }
    return 0;
}

int32_t ObjectIR_PluginGetClassMetadataJson(
    ObjectIR_VirtualMachine* vm,
    const char* className,
    int32_t includeInstructions,
    char** outJson
) {
    if (!vm || !className || !outJson) {
        SetLastError("Invalid arguments to ObjectIR_PluginGetClassMetadataJson");
        return 0;
    }

    try {
        ClearLastError();
        auto* vmCpp = reinterpret_cast<ObjectIR::VirtualMachine*>(vm);
        auto cls = FindClass(vmCpp, className);
        auto j = vmCpp->ExportClassMetadata(ObjectIR::TypeNames::GetQualifiedClassName(cls), includeInstructions != 0);
        auto s = j.dump();
        *outJson = CopyToCString(s);
        if (!*outJson) {
            SetLastError("Allocation failure");
            return 0;
        }
        return 1;
    } catch (const std::exception& ex) {
        SetLastError(ex.what());
    } catch (...) {
        SetLastError("Unknown error in ObjectIR_PluginGetClassMetadataJson");
    }
    return 0;
}

int32_t ObjectIR_PluginReplaceMethodInstructionsJson(
    ObjectIR_VirtualMachine* vm,
    const char* className,
    const char* methodName,
    const char* instructionsJsonArray
) {
    if (!vm || !className || !methodName || !instructionsJsonArray) {
        SetLastError("Invalid arguments to ObjectIR_PluginReplaceMethodInstructionsJson");
        return 0;
    }

    try {
        ClearLastError();
        auto* vmCpp = reinterpret_cast<ObjectIR::VirtualMachine*>(vm);
        auto cls = FindClass(vmCpp, className);

        auto m = FindMethodUniqueNameOrThrow(cls, methodName);

        ObjectIR::json j = ObjectIR::json::parse(instructionsJsonArray);
        if (!j.is_array()) {
            throw std::runtime_error("instructionsJsonArray must be a JSON array");
        }

        std::vector<ObjectIR::Instruction> compiled;
        compiled.reserve(j.size());
        for (const auto& node : j) {
            compiled.push_back(ObjectIR::InstructionExecutor::ParseJsonInstruction(node));
        }

        m->SetInstructions(std::move(compiled));
        return 1;
    } catch (const std::exception& ex) {
        SetLastError(ex.what());
    } catch (...) {
        SetLastError("Unknown error in ObjectIR_PluginReplaceMethodInstructionsJson");
    }

    return 0;
}

int32_t ObjectIR_PluginReplaceMethodInstructionsJsonBySignature(
    ObjectIR_VirtualMachine* vm,
    const char* className,
    const char* methodName,
    const char* parameterTypesJsonArray,
    const char* /*returnType*/,
    const char* instructionsJsonArray
) {
    if (!vm || !className || !methodName || !parameterTypesJsonArray || !instructionsJsonArray) {
        SetLastError("Invalid arguments to ObjectIR_PluginReplaceMethodInstructionsJsonBySignature");
        return 0;
    }

    try {
        ClearLastError();
        auto* vmCpp = reinterpret_cast<ObjectIR::VirtualMachine*>(vm);
        auto cls = FindClass(vmCpp, className);

        ObjectIR::json paramJson = ObjectIR::json::parse(parameterTypesJsonArray);
        if (!paramJson.is_array()) {
            throw std::runtime_error("parameterTypesJsonArray must be a JSON array");
        }

        std::vector<std::string> parameterTypes;
        parameterTypes.reserve(paramJson.size());
        for (const auto& node : paramJson) {
            if (!node.is_string()) {
                throw std::runtime_error("parameterTypesJsonArray elements must be strings");
            }
            parameterTypes.push_back(node.get<std::string>());
        }

        auto m = FindMethodBySignatureOrThrow(cls, methodName, parameterTypes);

        ObjectIR::json j = ObjectIR::json::parse(instructionsJsonArray);
        if (!j.is_array()) {
            throw std::runtime_error("instructionsJsonArray must be a JSON array");
        }

        std::vector<ObjectIR::Instruction> compiled;
        compiled.reserve(j.size());
        for (const auto& node : j) {
            compiled.push_back(ObjectIR::InstructionExecutor::ParseJsonInstruction(node));
        }

        m->SetInstructions(std::move(compiled));
        return 1;
    } catch (const std::exception& ex) {
        SetLastError(ex.what());
    } catch (...) {
        SetLastError("Unknown error in ObjectIR_PluginReplaceMethodInstructionsJsonBySignature");
    }

    return 0;
}

} // extern "C"
