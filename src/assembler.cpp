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
            if (il->type == IL_TYPE_DECLARE_VARIABLE) {
                const DeclareVariable* var = (DeclareVariable*)&il->data;
                size += var->size / 8;
            }
        }
        
        return size;
    };

    const auto getStackOffset = [&](const DeclareVariable* var) -> size_t {
        size_t offset = 0;
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
        switch (il->type) {
            case IL_TYPE_DECLARE_FUNCTION: {
                function = (DeclareFunction*)&il->data;
                
                label(function->name == "efi_main" ? "_start" : function->name);
                
                stack.clear();
                sub("rsp", to_string(getLocalsSize()));
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
                    printf("right: %s\n", data->right->name.data());
                    printf("left: %s\n", data->left->name.data());

                    mov("rax", "[rsp+" + to_string(getStackOffset(data->right)) + "]");
                    mov("[rsp+" + to_string(getStackOffset(data->left)) + "]", "rax");
                }
            } break;
            case IL_TYPE_RETURN: {
                const FunctionReturn* data = (FunctionReturn*)&il->data;

                mov("rax", "[rsp+" + to_string(getStackOffset(data->var)) + "]");

                size_t stack_size = getStackSize();
                if (stack_size > 0) {
                    add("rsp", to_string(stack_size));
                }

                if (function->name == "efi_main") {
                    mov("rbx", "rax");
                    mov("rax", "1");
                    _int("80h");
                }
                else {
                    ret();
                }

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