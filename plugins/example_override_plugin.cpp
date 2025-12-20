#include "objectir_runtime.hpp"
#include "instruction_executor.hpp"
#include "objectir_plugin_helpers.hpp"
#include "objectir_plugin_api.h"

#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

using ObjectIR::InstructionExecutor;
using ObjectIR::VirtualMachine;

extern "C" int32_t ObjectIR_PluginGetInfo(ObjectIR_PluginInfoV1* outInfo) {
    if (!outInfo) return 0;

    // Allow load on any v1.x runtime (minor-compatible).
    // If you ever make a breaking change to the C-friendly plugin ABI, bump
    // OBJECTIR_PLUGIN_ABI_MAJOR and narrow this range.
    outInfo->structSize = sizeof(ObjectIR_PluginInfoV1);
    outInfo->abiMinPacked = OBJECTIR_PLUGIN_ABI_PACKED(1u, 0u);
    outInfo->abiMaxPacked = OBJECTIR_PLUGIN_ABI_PACKED(1u, 0xFFFFu);
    outInfo->pluginName = "objectir_example_override_plugin";
    outInfo->pluginVersion = "1.0.0";
    return 1;
}

static ObjectIR::ClassRef FindProgramClass(VirtualMachine* vm) {
    if (!vm) throw std::runtime_error("vm is null");

    // Prefer exact match.
    if (vm->HasClass("Program")) {
        return vm->GetClass("Program");
    }

    // Otherwise scan for something ending in ".Program".
    auto names = vm->GetAllClassNames();
    auto it = std::find_if(names.begin(), names.end(), [](const std::string& n) {
        return n == "Program" || (n.size() >= 8 && n.rfind(".Program") == (n.size() - 8));
    });
    if (it != names.end()) {
        return vm->GetClass(*it);
    }

    throw std::runtime_error("Could not find Program class in VM");
}

extern "C" bool ObjectIR_PluginInit(VirtualMachine* vm) {
    try {
        auto program = FindProgramClass(vm);
        auto main = program->LookupMethod("Main");
        if (!main || !main->IsStatic()) {
            throw std::runtime_error("Program.Main not found or not static");
        }

        using namespace ObjectIR::PluginHelpers;

        const auto writeLine = MethodRef(
            "System.Console",
            "WriteLine",
            {"string"},
            "void"
        );

        std::vector<ObjectIR::Instruction> compiled;
        compiled.reserve(3);
        compiled.push_back(LdStr("Hello from native plugin!"));
        compiled.push_back(Call(writeLine));
        compiled.push_back(Ret());

        main->SetInstructions(std::move(compiled));
        return true;
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "[objectir_example_override_plugin] init failed: %s\n", ex.what());
        return false;
    } catch (...) {
        std::fprintf(stderr, "[objectir_example_override_plugin] init failed: unknown exception\n");
        return false;
    }
}

extern "C" void ObjectIR_PluginShutdown(VirtualMachine* /*vm*/) {
    // No-op.
}
