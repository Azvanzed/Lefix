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

void engine::Assembler::_add(const string& dst, const string& src, const string& comment) {
    m_output += "\tadd " + dst + ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_sub(const string& dst, const string& src, const string& comment) {
    m_output += "\tsub " + dst + ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_div(const string& src, const string& comment) {
    m_output += "\tdiv " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_mul(const string& src, const string& comment) {
    m_output += "\tmul " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}


void engine::Assembler::insert(const string& code, const string& comment) {
    m_output += "\n";

    if (comment.empty() == false) {
        m_output += "\t; " + comment + "\n";
    }

    istringstream iss(code);
    ostringstream oss;

    string line;
    while (getline(iss, line)) {
        oss << "\t" << line << "\n";
    }

    m_output += oss.str();

    if (comment.empty() == false) {
        m_output += "\t; " + comment + "\n";
    }

    m_output += "\n";
}

void engine::Assembler::_mov(const string& dst, const string& src, const string& comment) {
    m_output += "\tmov " + dst + ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_push(const string& src, const string& comment) {
    m_output += "\tpush " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_pop(const string& dst, const string& comment) {
    m_output += "\tpop " + dst;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_call(const string& dst, const string& comment) {
    m_output += "\tcall " + dst;
    
    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_xor(const string& dst, const string& src, const string& comment) {
    m_output += "\txor " + dst + ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_or(const string& dst, const string& src, const string& comment) {
    m_output += "\tor " + dst + ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_not(const string& src, const string& comment) {
    m_output += "\tnot " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_and(const string& dst, const string& src, const string& comment) {
    m_output += "\tand " + dst + ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_shr(const string& dst, const string& src, const string& comment) {
    m_output += "\tshr " + dst + ", " + src;

    if (comment.empty() == false) {
        m_output += " ; " + comment;
    }

    m_output += "\n";
}

void engine::Assembler::_shl(const string& dst, const string& src, const string& comment) {
    m_output += "\tshl " + dst + ", " + src;

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
            _sub("rsp", to_string(routine->stack_size), "reserve locals");
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
  
                const string& left_mem = getMemSize(left->size);
                const string& right_mem = getMemSize(right->size);
                const string& left_gp0 = getGP0(left->size);
                const string& right_gp0 = getGP0(right->size);

                if (data->type != SET_TYPE_SHIFTL && data->type != SET_TYPE_SHIFTR) {
                    if (!(right->flags & VAR_FLAGS_IMMEDIATE)) {
                        const AsmLocal* right_stack = routine->stack.at(right);

                        int64_t right_offset = right_stack->offset;
                        if (right->flags & VAR_FLAGS_ARG) {
                            right_offset += routine->stack_size + 8;
                        }

                        // read from var within the stack
                        _mov(right_gp0, right_mem + " [rsp+" + to_string(right_offset) + "]", right->name);
                    }
                    else {
                        // write to gp0 reg imm value
                        _mov(right_gp0, to_string(IL::getImm(right->value)), right->name);
                    }
                }

                // write to var within the stack
                const AsmLocal* left_stack = routine->stack.at(left);
            
                int64_t left_offset = left_stack->offset;
                 if (left->flags & VAR_FLAGS_ARG) {
                    left_offset += routine->stack_size + 8;
                }
                
                switch (data->type) {
                    case SET_TYPE_DIRECT: {
                        _mov(left_mem + " [rsp+" + to_string(left_offset) + "]", left_gp0, left->name);
                    } break;
                    case SET_TYPE_ADD: {
                        _add(left_mem + " [rsp+" + to_string(left_offset) + "]", left_gp0, left->name);
                    } break;
                    case SET_TYPE_SUB: {
                        _sub(left_mem + " [rsp+" + to_string(left_offset) + "]", left_gp0, left->name);
                    } break;
                    case SET_TYPE_XOR: {
                        _xor(left_mem + " [rsp+" + to_string(left_offset) + "]", left_gp0, left->name);
                    } break;
                    case SET_TYPE_OR: {
                        _or(left_mem + " [rsp+" + to_string(left_offset) + "]", left_gp0, left->name);
                    } break;
                    case SET_TYPE_NOT: {
                        _not(left_mem + " [rsp+" + to_string(left_offset) + "]", left->name);
                    } break;
                    case SET_TYPE_AND: {
                        _and(left_mem + " [rsp+" + to_string(left_offset) + "]", left_gp0, left->name);
                    } break;
                    case SET_TYPE_SHIFTL: {
                        _shl(left_mem + " [rsp+" + to_string(left_offset) + "]", to_string(IL::getImm(right->value)), left->name);
                    } break;
                    case SET_TYPE_SHIFTR: {
                        _shr(left_mem + " [rsp+" + to_string(left_offset) + "]", to_string(IL::getImm(right->value)), left->name);
                    } break;
                    case SET_TYPE_MUL: {   
                        _mov("rbx", "rax");
                        _mov(left_gp0, left_mem + " [rsp+" + to_string(left_offset) + "]", left->name);
                        _mul("rbx", right->name);   
                        _mov(left_mem + " [rsp+" + to_string(left_offset) + "]", "rax", left->name); 
                    } break;
                    case SET_TYPE_DIV: {
                        _mov("rbx", "rax");
                        _xor("rdx", "rdx");
                        _mov(left_gp0, left_mem + " [rsp+" + to_string(left_offset) + "]", left->name);
                        _div("rbx", right->name);   
                        _mov(left_mem + " [rsp+" + to_string(left_offset) + "]", "rax", left->name);
                    } break;
                    case SET_TYPE_REM: {
                        _mov("rbx", "rax");
                        _xor("rdx", "rdx");
                        _mov(left_gp0, left_mem + " [rsp+" + to_string(left_offset) + "]", left->name);
                        _div("rbx", right->name);   
                        _mov(left_mem + " [rsp+" + to_string(left_offset) + "]", "rdx", left->name);
                    } break;
                    default: break;
                }
            } break;
            case IL_TYPE_FUNC_CALL: {
                const FunctionCall* data = &get<FunctionCall>(insn->data);
                
                size_t stack_size = 0;
                for (const DeclareVariable* arg : data->args) {
                    const string& mem = getMemSize(arg->size);
                    const string& gp0 = getGP0(arg->size);

                    // reserve space for the var
                    _sub("rsp", to_string(arg->size / 8), "reserve " + arg->name);

                    if (!(arg->flags & VAR_FLAGS_IMMEDIATE)) {
                        const AsmLocal* arg_stack = routine->stack.at(arg);

                        int64_t arg_offset = arg_stack->offset;
                        if (arg->flags & VAR_FLAGS_ARG) {
                            arg_offset += routine->stack_size + 8;
                        }
                        arg_offset += arg->size / 8; // skip the reserved space

                        _mov(gp0, mem + " [rsp+" + to_string(arg_offset) + "]", arg->name);
                        _mov(mem + " [rsp+0]", gp0);
                    }
                    else {
                        _mov(gp0, to_string(IL::getImm(arg->value)), arg->name);
                        _mov(mem + " [rsp+0]", gp0);
                    }

                    stack_size += arg->size / 8;
                }

                _call(data->callee->name);

                if (stack_size > 0) {
                    _add("rsp", to_string(stack_size), "free args");
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
                    _mov(mem + " [rsp+" + to_string(ret_offset) + "]", gp0, data->ret->name);
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
                        _mov(gp0, mem + " [rsp+" + to_string(offset) + "]", var->name);
                    }
                    else {
                        // write to gp0 reg imm value
                        _mov(gp0, to_string(IL::getImm(var->value)), var->name);
                    }
                }

                if (routine->stack_size > 0) {
                    _add("rsp", to_string(routine->stack_size), "free locals");
                } 

                _ret();
            } break;
            default: break;
            }
        }
    }

    global("_start", "for testing");
    label("_start");
    _mov("rcx", "69"); 
    _mov("rdx", "96"); 
    _push("rdx", "SystemTable");
    _push("rcx", "ImageHandle");
    _call("efi_main");
    _add("rsp", "16", "free args");
    _mov("rbx", "rax", "exit code");
    _mov("rax", "1", "sys_exit");
    _int("0x80");
}


void engine::Assembler::create(const string& filename) const {
    io::File file(filename);
    file.clear();
    file.write(m_output);
}