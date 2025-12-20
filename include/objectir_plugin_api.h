#pragma once

// C-friendly plugin API for ObjectIR.
//
// This header is intended for plugins written in C or C++.
// Plugins still export the entrypoints:
//   bool ObjectIR_PluginInit(ObjectIR_VirtualMachine* vm);
//   void ObjectIR_PluginShutdown(ObjectIR_VirtualMachine* vm); // optional
//
// The `vm` pointer is an opaque handle to the running VM instance.
//
// Design goals:
// - Keep ABI stable and C compatible
// - Use JSON strings for "rich" data (class lists, instruction blocks, etc.)
// - Avoid exposing C++ STL types or templates

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
  #if defined(OBJECTIR_RUNTIME_STATIC)
    #define OBJECTIR_PLUGIN_API
  #else
    #if defined(objectir_runtime_EXPORTS)
      #define OBJECTIR_PLUGIN_API __declspec(dllexport)
    #else
      #define OBJECTIR_PLUGIN_API __declspec(dllimport)
    #endif
  #endif
#elif defined(__GNUC__)
  #define OBJECTIR_PLUGIN_API __attribute__((visibility("default")))
#else
  #define OBJECTIR_PLUGIN_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Opaque VM type (actually a C++ ObjectIR::VirtualMachine instance).
typedef struct ObjectIR_VirtualMachine ObjectIR_VirtualMachine;

// ---------------------------------------------------------------------------
// ABI versioning
// ---------------------------------------------------------------------------
//
// This is the ABI for the functions declared in this header.
// - Major bumps may break existing plugins.
// - Minor bumps add new optional functions / struct fields.
//
// Plugins that want forward/backward compatibility can optionally export:
//   int32_t ObjectIR_PluginGetInfo(struct ObjectIR_PluginInfoV1* outInfo);
// The runtime loader will call it (if present) and validate the ABI range.

#define OBJECTIR_PLUGIN_ABI_MAJOR 1u
#define OBJECTIR_PLUGIN_ABI_MINOR 0u
#define OBJECTIR_PLUGIN_ABI_PATCH 0u
#define OBJECTIR_PLUGIN_ABI_PACKED(major, minor) ((((uint32_t)(major)) << 16u) | ((uint32_t)(minor) & 0xFFFFu))
#define OBJECTIR_PLUGIN_ABI_VERSION_PACKED OBJECTIR_PLUGIN_ABI_PACKED(OBJECTIR_PLUGIN_ABI_MAJOR, OBJECTIR_PLUGIN_ABI_MINOR)

typedef struct ObjectIR_PluginInfoV1 {
  uint32_t structSize;
  uint32_t abiMinPacked;
  uint32_t abiMaxPacked;
  const char* pluginName;
  const char* pluginVersion;
} ObjectIR_PluginInfoV1;


// Error handling (thread-local).
OBJECTIR_PLUGIN_API const char* ObjectIR_PluginLastError(void);

// Memory management for strings returned by this API.
OBJECTIR_PLUGIN_API void ObjectIR_PluginFreeString(char* str);

// Returns OBJECTIR_PLUGIN_ABI_VERSION_PACKED for the runtime you linked against.
OBJECTIR_PLUGIN_API uint32_t ObjectIR_RuntimeGetPluginAbiVersionPacked(void);

// Introspection helpers.
// Returns 1 on success and writes a newly allocated JSON string to *outJson.
// JSON shape: ["ClassName", "Namespace.ClassName", ...]
OBJECTIR_PLUGIN_API int32_t ObjectIR_PluginGetAllClassNamesJson(ObjectIR_VirtualMachine* vm, char** outJson);

// Returns metadata JSON for a class.
// - className can be simple or qualified.
// - includeInstructions: 0 or 1.
OBJECTIR_PLUGIN_API int32_t ObjectIR_PluginGetClassMetadataJson(
  ObjectIR_VirtualMachine* vm,
  const char* className,
  int32_t includeInstructions,
  char** outJson
);

// Patching helpers.
// Replaces a method's instruction list by parsing a JSON array of instruction nodes.
// - className can be either "Program" or "Namespace.Program"; lookup falls back to scanning.
// - methodName is the simple method name (e.g., "Main").
// - instructionsJsonArray must be a JSON array compatible with InstructionExecutor::ParseJsonInstruction.
// Returns 1 on success.
OBJECTIR_PLUGIN_API int32_t ObjectIR_PluginReplaceMethodInstructionsJson(
    ObjectIR_VirtualMachine* vm,
    const char* className,
    const char* methodName,
    const char* instructionsJsonArray
);

// Signature-qualified patching helper (recommended for overloaded methods).
// - parameterTypesJsonArray must be a JSON array of type strings.
// - returnType may be NULL; it is currently informational.
OBJECTIR_PLUGIN_API int32_t ObjectIR_PluginReplaceMethodInstructionsJsonBySignature(
  ObjectIR_VirtualMachine* vm,
  const char* className,
  const char* methodName,
  const char* parameterTypesJsonArray,
  const char* returnType,
  const char* instructionsJsonArray
);

#ifdef __cplusplus
} // extern "C"
#endif
