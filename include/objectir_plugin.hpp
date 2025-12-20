#pragma once

namespace ObjectIR {
class VirtualMachine;

// A plugin shared library should export this symbol.
//
// extern "C" bool ObjectIR_PluginInit(ObjectIR::VirtualMachine* vm);
//
// Returning false indicates plugin initialization failure.
using PluginInitFn = bool (*)(VirtualMachine*);

// Optional shutdown hook.
//
// extern "C" void ObjectIR_PluginShutdown(ObjectIR::VirtualMachine* vm);
using PluginShutdownFn = void (*)(VirtualMachine*);

} // namespace ObjectIR
