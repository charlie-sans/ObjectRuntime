#pragma once

#include "instruction_executor.hpp"
#include "objectir_type_names.hpp"

#include <string>
#include <utility>
#include <vector>

namespace ObjectIR::PluginHelpers {

inline CallTarget MethodRef(std::string declaringType,
                            std::string name,
                            std::vector<std::string> parameterTypes,
                            std::string returnType = "void") {
    CallTarget target;
    target.declaringType = std::move(declaringType);
    target.name = std::move(name);
    target.returnType = TypeNames::NormalizeTypeName(returnType);
    target.parameterTypes = TypeNames::NormalizeTypeNames(parameterTypes);
    return target;
}

inline Instruction LdStr(std::string value) {
    Instruction instr;
    instr.opCode = OpCode::LdStr;
    instr.hasConstant = true;
    instr.constantRawValue = std::move(value);
    return instr;
}

inline Instruction LdI4(int32_t value) {
    Instruction instr;
    instr.opCode = OpCode::LdI4;
    instr.hasConstant = true;
    instr.operandInt = value;
    instr.hasOperandInt = true;
    instr.constantRawValue = std::to_string(value);
    instr.constantType = "int32";
    return instr;
}

inline Instruction Call(const CallTarget& target) {
    Instruction instr;
    instr.opCode = OpCode::Call;
    instr.callTarget = target;
    return instr;
}

inline Instruction CallVirt(const CallTarget& target) {
    Instruction instr;
    instr.opCode = OpCode::CallVirt;
    instr.callTarget = target;
    return instr;
}

inline Instruction Ret() {
    Instruction instr;
    instr.opCode = OpCode::Ret;
    return instr;
}

inline std::vector<Instruction> CompileJsonInstructions(const json& instrArray) {
    std::vector<Instruction> compiled;
    if (!instrArray.is_array()) return compiled;

    compiled.reserve(instrArray.size());
    for (const auto& node : instrArray) {
        compiled.push_back(InstructionExecutor::ParseJsonInstruction(node));
    }
    return compiled;
}

} // namespace ObjectIR::PluginHelpers
