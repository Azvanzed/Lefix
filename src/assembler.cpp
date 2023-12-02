#include "assembler.hpp"
#include "io.hpp"
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include "assert.hpp"

using namespace std;

engine::Assembler::Assembler() {
}

engine::Assembler::Assembler(const vector<const IL_Instruction*>& ils) 
    : m_ils(move(ils)){
    m_routines.clear();
}

engine::Assembler::~Assembler() {
    for (const AsmRoutine* routine : m_routines) {
        for (const auto [var, local] : routine->stack) {
            delete local;
        }
     
        delete routine;
    }
}

void engine::Assembler::global(const string& name, const string& comment) {
    m_output += "global " + name + ";";

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::label(const string& name, const string& comment) {
    m_output += name + ":";

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::add(const string& dst, const string& src, const string& comment) {
    m_output += "\tadd " + dst += ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::sub(const string& dst, const string& src, const string& comment) {
    m_output += "\tsub " + dst += ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::insert(const string& code, const string& comment) {
    m_output += "\n";

    if (comment.empty() == false) {
        m_output += " ; " + comment + "\n";
    }

    istringstream iss(code);
    ostringstream oss;

    string line;
    while (getline(iss, line)) {
        oss << "\t" << line << "\n";
    }

    m_output += oss.str();

    if (comment.empty() == false) {
        m_output += " ; " + comment + "\n";
    }

    m_output += "\n";
}

void engine::Assembler::mov(const string& dst, const string& src, const string& comment) {
    m_output += "\tmov " + dst += ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::push(const string& src, const string& comment) {
    m_output += "\tpush " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::pop(const string& dst, const string& comment) {
    m_output += "\tpop " + dst;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::call(const string& dst, const string& comment) {
    m_output += "\tcall " + dst;
    
    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_ret(const string& comment) {
    m_output += "\tret";

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_int(const string& value, const string& comment) {
    m_output += "\tint " + value;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

string engine::Assembler::getMemSize(size_t size) {
    switch (size) {
        case 8: return "byte";
        case 16: return "word";
        case 32: return "dword";
        case 64: return "qword";
        default: CRASH("Unknown type size"); return "qword";
    }
}

string engine::Assembler::getGP0(size_t size) {
    switch (size) {
        case 8: return "al";
        case 16: return "ax";
        case 32: return "eax";
        case 64: return "rax";
        default: CRASH("Unknown type size"); return "rax";
    }
}

size_t engine::Assembler::AlignStack(size_t offset, size_t size) {
    size = min<size_t>(8, size);
    if (offset % size == 0) {
        return offset;
    }

    return offset + (size - (offset % size));
}

void engine::Assembler::translate() {
    for (const IL_Instruction* il : m_ils) {
        switch (il->type) {
            case IL_TYPE_DECLARE_FUNCTION: {
                const DeclareFunction* func = (const DeclareFunction*)&il->data;

                AsmRoutine* routine = new AsmRoutine;
                routine->name = func->name;
                routine->stack_size = 0;
                routine->stack.clear();
                routine->insns.clear();
                
                vector<const DeclareVariable*> vars;
                for (const DeclareVariable& arg : func->args) {
                    vars.push_back(&arg);
                }
                
                for (const IL_Instruction* il : m_ils) {
                    if (il->type == IL_TYPE_DECLARE_VARIABLE) {
                        const DeclareVariable* var = &get<DeclareVariable>(il->data);
                        if (var->function == func) {
                            vars.push_back(var);
                        }
                    }
                }

                for (const DeclareVariable* var : vars) {
                    AsmLocal* local = new AsmLocal;
                    local->size = var->size / 8;
                    local->type = var->value.empty() == false ? ASM_LOCAL_TYPE_IMMEDIATE : ASM_LOCAL_TYPE_NONE; 
                    local->offset = AlignStack(routine->stack_size, local->size);

                    switch (local->type) {
                        case ASM_LOCAL_TYPE_IMMEDIATE: {
                            switch (var->size)  {
                                case 8: local->imm.b8 = IL::getImm(var->value); break;
                                case 16: local->imm.b16 = IL::getImm(var->value); break;
                                case 32: local->imm.b32 = IL::getImm(var->value); break;
                                case 64: local->imm.b64 = IL::getImm(var->value); break;
                                default: CRASH("Unknown type size"); break;
                            }
                            
                        } break;
                        default: break;
                    }

                    routine->stack.emplace(var, local);

                    size_t diff = local->offset - routine->stack_size;
                    routine->stack_size += diff + local->size;
                }

                for (const IL_Instruction* il : m_ils) {
                    if (il->type != IL_TYPE_DECLARE_VARIABLE) {
                        if (*(DeclareFunction**)&il->data == func) {
                            routine->insns.push_back(il);
                        }
                    }
                }
                
                m_routines.push_back(routine);
                break;
            } break;
            default: break;
        };
    }
}

void engine::Assembler::optimize() {
    // Remove stack if no locals are used
    for (AsmRoutine* routine : m_routines) {
        if (routine->stack.empty()) {
            continue;
        }

        vector<const AsmLocal*> used_locals;
        for (const IL_Instruction* insn : routine->insns) {
            switch (insn->type) {
                case IL_TYPE_RETURN: {
                    const FunctionReturn* data = &get<FunctionReturn>(insn->data);
                    if (data->var != nullptr) {
                        used_locals.push_back(routine->stack.at(data->var));
                    }
                } break;
                case IL_TYPE_EQ_SET: {
                    const EQSet* data = &get<EQSet>(insn->data);
                    const DeclareVariable* left = data->left;
                    const DeclareVariable* right = data->right;
                    
                    if (!(left->flags & VAR_FLAGS_IMMEDIATE)) {
                        used_locals.push_back(routine->stack.at(left));
                    }

                    if (!(right->flags & VAR_FLAGS_IMMEDIATE)) {
                        used_locals.push_back(routine->stack.at(right));
                    }
                } break;
                case IL_TYPE_FUNC_CALL: {
                    const FunctionCall* data = &get<FunctionCall>(insn->data);
                    if (data->ret != nullptr) {
                        used_locals.push_back(routine->stack.at(data->ret));
                    }
                    
                    for (const DeclareVariable* arg : data->args) {
                        used_locals.push_back(routine->stack.at(arg));
                    }
                } break;
                default: break;
            }
        }

        if (used_locals.empty() == true) {
            routine->stack_size = false;
            routine->stack.clear();
        }
    }
}



void engine::Assembler::assemble() {
    for (const AsmRoutine* routine : m_routines) {
        // create a label for the function
        label(routine->name);

        // reserve stack for variables
        if (routine->stack_size > 0) {
            sub("rsp", to_string(routine->stack_size), "reserve locals");
        }

        // assemble instructions
        for (const IL_Instruction* insn : routine->insns) {
            switch (insn->type)
            {
            case IL_TYPE_INLINE_ASM: {
                InlineAsm* data = (InlineAsm*)&get<InlineAsm>(insn->data);


                for (size_t i = 0; i < data->code.size(); ++i) {
                    if (data->code.substr(i, 11) == "@stack_size") {
                        data->code.erase(i, 11);

                        string stub = to_string(routine->stack_size) + " ; @stack_size";
                        data->code.insert(i, stub);
                        i += stub.size();
                    } else if (data->code[i] == '@') {
                        size_t start = i + 1;
                        size_t end = start;
                        while (end < data->code.size() && (isalnum(data->code[end]) || data->code[end] == '_')) {
                            ++end;
                        }

                        for (const auto [var, local] : routine->stack) {
                            if (var->name == data->code.substr(start, end - start)) {
                                int64_t right_offset = local->offset;
                                if (var->flags & VAR_FLAGS_ARG) {
                                    right_offset += routine->stack_size + 8;
                                }

                                data->code.erase(i, end - start + 1);
                                data->code.insert(i, + "rsp+" + to_string(right_offset));
                                break;
                            }
                        }
                    }
                }

                insert(data->code, "Inlined assembly");
            } break;
            case IL_TYPE_EQ_SET: {
                const EQSet* data = &get<EQSet>(insn->data);
                
                const DeclareVariable* left = data->left;
                const DeclareVariable* right = data->right;
  
                const string& mem = getMemSize(right->size);
                const string& gp0 = getGP0(right->size);

                if (!(right->flags & VAR_FLAGS_IMMEDIATE)) {
                    const AsmLocal* right_stack = routine->stack.at(right);
                  
                    int64_t right_offset = right_stack->offset;
                    if (right->flags & VAR_FLAGS_ARG) {
                        right_offset += routine->stack_size + 8;
                    }

                    // read from var within the stack
                    mov(gp0, mem + " [rsp+" + to_string(right_offset) + "]", right->name);
                }
                else {
                    // write to gp0 reg imm value
                    mov(gp0, to_string(IL::getImm(right->value)), right->name);
                }

                // write to var within the stack
                const AsmLocal* left_stack = routine->stack.at(left);
            
                int64_t left_offset = left_stack->offset;
                 if (left->flags & VAR_FLAGS_ARG) {
                    left_offset += routine->stack_size + 8;
                }

                mov(mem + " [rsp+" + to_string(left_offset) + "]", gp0, left->name);
            } break;
            case IL_TYPE_FUNC_CALL: {
                const FunctionCall* data = &get<FunctionCall>(insn->data);
                
                size_t stack_size = 0;
                for (const DeclareVariable* arg : data->args) {
                    const string& mem = getMemSize(arg->size);
                    const string& gp0 = getGP0(arg->size);

                    // reserve space for the var
                    sub("rsp", to_string(arg->size / 8), "reserve " + arg->name);

                    if (!(arg->flags & VAR_FLAGS_IMMEDIATE)) {
                        const AsmLocal* arg_stack = routine->stack.at(arg);

                        int64_t arg_offset = arg_stack->offset;
                        if (arg->flags & VAR_FLAGS_ARG) {
                            arg_offset += routine->stack_size + 8;
                        }
                        arg_offset += arg->size / 8; // skip the reserved space

                        mov(gp0, mem + " [rsp+" + to_string(arg_offset) + "]", arg->name);
                        mov(mem + " [rsp+0]", gp0);
                    }
                    else {
                        mov(gp0, to_string(IL::getImm(arg->value)), arg->name);
                        mov(mem + " [rsp+0]", gp0);
                    }

                    stack_size += arg->size / 8;
                }

                call(data->callee->name);

                if (stack_size > 0) {
                    add("rsp", to_string(stack_size), "free args");
                }
                
                if (data->ret != nullptr) {
                    size_t ret_size = DATA_TYPE_SIZES.at(data->ret->type);
                    const string& mem = getMemSize(ret_size);
                    const string& gp0 = getGP0(ret_size);

                    const AsmLocal* ret_stack = routine->stack.at(data->ret);

                    int64_t ret_offset = ret_stack->offset;
                    if (data->ret->flags & VAR_FLAGS_ARG) {
                        ret_offset += routine->stack_size + 8;
                    }

                    // write to var within the stack from gp0 (result)
                    mov(mem + " [rsp+" + to_string(ret_offset) + "]", gp0, data->ret->name);
                }

            } break;
            case IL_TYPE_RETURN: {
                const FunctionReturn* data = &get<FunctionReturn>(insn->data);

                if (data->var != nullptr) {
                    const DeclareVariable* var = data->var;

                    const string& gp0 = getGP0(var->size);
                    const string& mem = getMemSize(var->size);

                    if (!(var->flags & VAR_FLAGS_IMMEDIATE)) {
                        const AsmLocal* var_stack = routine->stack.at(var);

                        // calculate stack offset
                        int64_t offset = var_stack->offset;
                        if (var->flags & VAR_FLAGS_ARG) {
                            offset += routine->stack_size + 8;
                        }

                        // read from var within the stack
                        mov(gp0, mem + " [rsp+" + to_string(offset) + "]", var->name);
                    }
                    else {
                        // write to gp0 reg imm value
                        mov(gp0, to_string(IL::getImm(var->value)), var->name);
                    }
                }

                if (routine->stack_size > 0) {
                    add("rsp", to_string(routine->stack_size), "free locals");
                } 

                _ret();
            } break;
            default: break;
            }
        }
    }

    global("_start", "for testing");
    label("_start");
    mov("rcx", "69"); 
    mov("rdx", "96"); 
    push("rdx", "SystemTable");
    push("rcx", "ImageHandle");
    call("efi_main");
    add("rsp", "16", "free args");
    mov("rbx", "rax", "exit code");
    mov("rax", "1", "sys_exit");
    _int("0x80");
}


void engine::Assembler::create(const string& filename) const {
    io::File file(filename);
    file.clear();
    file.write(m_output);
}