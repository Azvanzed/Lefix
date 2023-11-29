#include "assembler.hpp"
#include "io.hpp"
#include <fstream>
#include <unordered_map>
#include "assert.hpp"

using namespace std;

engine::Assembler::Assembler() {
}

engine::Assembler::Assembler(const vector<IL_Instruction*>& ils) 
    : m_ils(move(ils)){
}

engine::Assembler::~Assembler() {
}

void engine::Assembler::label(const string& name) {
    m_output += name + ":\n";
}

void engine::Assembler::add(const string& dst, const string& src) {
    m_output += "\tadd " + dst + ", " + src + "\n";
}

void engine::Assembler::sub(const string& dst, const string& src) {
    m_output += "\tsub " + dst + ", " + src + "\n";
}

void engine::Assembler::mov(const string& dst, const string& src) {
    m_output += "\tmov " + dst + ", " + src + "\n";
}

void engine::Assembler::push(const string& src) {
    m_output += "\tpush " + src + "\n";
}

void engine::Assembler::pop(const string& dst) {
    m_output += "\tpop " + dst + "\n";
}

void engine::Assembler::call(const string& dst) {
    m_output += "\tcall " + dst + "\n";
}

void engine::Assembler::ret() {
    m_output += "\tret\n";
}

void engine::Assembler::_int(const string& value) {
    m_output += "\tint " + value + "\n";
}

void engine::Assembler::assemble() {
    DeclareFunction* function = nullptr;
    vector<pair<const DeclareVariable*, size_t>> stack;

    const auto getStackSize = [&]() -> size_t {
        size_t size = 0;
        for (const auto& [var, offset] : stack) {
            size += DATA_TYPE_SIZES.at(var->type);
        }
      
        return size / 8;
    };

    const auto getLocalsSize = [&]() -> size_t {
        size_t size = 0;

        for (const IL_Instruction* il : m_ils) {
            const DeclareVariable* var = (DeclareVariable*)&il->data;
                
            if (var->function == function && il->type == IL_TYPE_DECLARE_VARIABLE) {
                size += var->size / 8;
            }
        }
        
        return size;
    };

    const auto getArgsSize = [](const DeclareFunction* function) -> size_t {
        size_t size = 0;

        for (const DeclareVariable& arg : function->args) {
            size += arg.size / 8;
        }
        
        return size;
    };

    const auto getStackOffset = [&](const DeclareVariable* var) -> int64_t {
        if (var->flags & VAR_FLAGS_ARG) {
            size_t arg_offset = 0;
            for (const DeclareVariable& arg : function->args) {
                if (arg.name == var->name) {
                    return getLocalsSize() + arg_offset + 8;
                }

                arg_offset += arg.size / 8;
            }
        }

        int64_t offset = 0;
        for (const auto& [v, o] : stack) {
            if (v == var) {
                return offset;
            }

            offset += v->size / 8;
        }

        CRASH("variable not found");
        return 0;
    };

    m_output += "global _start\n\n";
    
    for (const IL_Instruction* il : m_ils) {
        ASSERT(il != nullptr, "il is null");

        switch (il->type) {
            case IL_TYPE_DECLARE_FUNCTION: {
                function = (DeclareFunction*)&il->data;
                
                label(function->name);
                
                stack.clear();

                if (function->name == "_start") {
                    // entry arguments for uefi
                    mov("[rsp+8]", "rcx");
                    mov("[rsp+16]", "rdx");
                }

                size_t locals_size = getLocalsSize();
                if (locals_size > 0) {
                    sub("rsp", to_string(locals_size));
                }
            } break;
            case IL_TYPE_DECLARE_VARIABLE: {
                const DeclareVariable* data = (DeclareVariable*)&il->data;
                stack.emplace_back(data, getStackSize());
            } break;
            case IL_TYPE_EQ_SET: {
                const EQSet* data = (EQSet*)&il->data;
            
                if (data->right->value.empty() == false) {
                    mov("rax", to_string(IL::getImm(data->right->value)));
                    mov("[rsp+" + to_string(getStackOffset(data->left)) + "]", "rax");
                }
                else {
                    mov("rax", "[rsp+" + to_string(getStackOffset(data->right)) + "]");
                    mov("[rsp+" + to_string(getStackOffset(data->left)) + "]", "rax");
                }
            } break;
            case IL_TYPE_FUNC_CALL: {
                const FunctionCall* data = (FunctionCall*)&il->data;
                
                if (data->args.empty() == false) {
                    sub("rsp", to_string(getArgsSize(data->callee)));

                    size_t arg_offset = 0;
                    for (size_t i = 0; i < data->args.size(); i++) {
                        const DeclareVariable* arg = data->args[i];
                        const DeclareVariable* callee_arg = &data->callee->args[i];

                        if (arg->value.empty() == false) {
                            mov("rax", to_string(IL::getImm(arg->value)));
                            mov("[rsp+" + to_string(arg_offset) + "]", "rax");
                        }
                        else {
                            mov("rax", "[rsp+" + to_string(getStackOffset(arg)) + "]");
                            mov("[rsp+" + to_string(arg_offset) + "]", "rax");
                        }

                        arg_offset += callee_arg->size / 8; 
                    }
                }

                call(data->callee->name);
            } break;
            case IL_TYPE_RETURN: {
                const FunctionReturn* data = (FunctionReturn*)&il->data;

                if (data->var->value.empty() == false) {
                    mov("rax", to_string(IL::getImm(data->var->value)));
                }
                else {
                    mov("rax", "[rsp+" + to_string(getStackOffset(data->var)) + "]");
                }

                size_t stack_size = getStackSize();
                if (stack_size > 0) {
                    add("rsp", to_string(stack_size));
                }

                if (function->name == "_start") { // exit process syscall for testing
                    mov("rbx", "rax");
                    mov("rax", "1");
                    _int("80h");
                }
                else {
                    ret();
                }

                m_output += "\n";
            } break;
            default: break;
        }
    }
}

void engine::Assembler::create(const string& filename) const {
    io::File file(filename);
    file.clear();
    file.write(m_output);
}