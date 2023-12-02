#include "il.hpp"
#include <iostream>
#include <random>
#include "assert.hpp"
#include <stdexcept>
#include <charconv>
#include <limits>
#include <regex>

using namespace std;

engine::IL::IL(const vector<Token>& tokens) {
    m_tokens = move(tokens);
}

engine::IL::~IL() {
    for (const IL_Instruction* insn : m_ils) {
        delete insn;
    }
}

void engine::IL::analyze() {
    DeclareFunction* function = nullptr;
    
    for (size_t i = 0; i < m_tokens.size(); ++i) {
        const Token& token = m_tokens.at(i);

        switch (token.type) {
            case TOKEN_TYPE_KEYWORD: {
                if (token.value == "fn") {
                    auto [il, size] = AnalyzeDeclareFunction(token);
                    function = (DeclareFunction*)&il->data;
                    m_ils.push_back(il);
                    i += size - 1;
                }
                else if (isDataType(token) == true) {
                    auto [il, size] = AnalyzeDeclareVariable(function, token);
                    m_ils.push_back(il);
                    i += size - 1;
                }
                else if (token.value == "ret") {
                    auto [ils, size] = AnalyzeReturn(function, token);
                    for (const IL_Instruction* il : ils) {
                        m_ils.push_back(il);
                    }

                    i += size - 1;
                }
            } break;
            case TOKEN_TYPE_IDENTIFIER: {
                if (Move(token, 1).type == TOKEN_TYPE_ARG_START) {
                    ASSERT(FindFunction(token.value) != NULL, "Function '%s' not found", token.value.data());
                    
                    auto [il, size] = AnalyzeCall(function, token);
                    m_ils.push_back(il);
                    i += size - 1;
                }
            } break;
            case TOKEN_TYPE_OPERATOR: {
                auto [il, size] = AnalyzeOperator(function, token);
                m_ils.push_back(il);
                i += size - 1;
            } break;
            case TOKEN_TYPE_MACRO: {
                auto [il, size] = AnalyzeMacro(function, token);
                m_ils.push_back(il);
                i += size - 1;
            } break;
            default: break;
        }
    }
}

void engine::IL::optimize() {
    // TODO
}

const vector<const engine::IL_Instruction*>& engine::IL::getILs() const {
    return move(m_ils);
}

const engine::Token& engine::IL::Move(const Token& token, int64_t times) const {
    int64_t index = -1;

    for (size_t i = 0; i < m_tokens.size(); ++i) {
        if (m_tokens[i].id == token.id) {
            index = i;
            break;
        }
    }

    index += times;
    ASSERT(index < (int64_t)m_tokens.size(), "Index out of bounds");
    
    return m_tokens.at(index);
}

uint16_t engine::IL::getRandomId() {
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dis(10000, 99999);
    return dis(gen);
}

engine::IL_Instruction* engine::IL::CreateIL(InstructionType type, const auto& data) const {
    IL_Instruction* il = new IL_Instruction();
    il->id = getRandomId();
    il->type = type;
    il->data = data;
    return il;
}

pair<engine::IL_Instruction*, size_t> engine::IL::AnalyzeDeclareFunction(const Token& token) const {
    ASSERT(isDataType(Move(token, 1)) == true, "Expected data type after 'fn' keyword");
    ASSERT(Move(token, 2).type == TOKEN_TYPE_IDENTIFIER, "Expected identifier after data type");
    ASSERT(Move(token, 3).type == TOKEN_TYPE_ARG_START, "Expected '(' after function identifier");

    DeclareFunction fn;
    fn.name = Move(token, 2).value;
    fn.args.clear();
    fn.ret_type = DATA_TYPES.at(Move(token, 1).value);

    IL_Instruction* il = CreateIL(IL_TYPE_DECLARE_FUNCTION, fn);

    size_t size = 4;
    if (Move(token, size).type != TOKEN_TYPE_ARG_END) {
        while (true) {
            const Token& arg = Move(token, size);
            ASSERT(isDataType(arg) == true, "Expected data type in argument");
            ASSERT(Move(token, size + 1).type == TOKEN_TYPE_IDENTIFIER, "Expected identifier after data type in argument");

            auto [il_arg, il_size] = AnalyzeDeclareVariable((DeclareFunction*)&il->data, arg);

            DeclareVariable* var = &get<DeclareVariable>(il_arg->data);
            var->flags |= VAR_FLAGS_ARG;

            get<DeclareFunction>(il->data).args.push_back(*var);
            delete il_arg;

            size += 3;
            if (Move(token, size - 1).type != TOKEN_TYPE_NEW_ARG) {
                break;
            }
        }
    }

    return { il, size };
}

pair<engine::IL_Instruction*, size_t> engine::IL::AnalyzeDeclareVariable(const DeclareFunction* function, const Token& token) const {
    ASSERT(function != nullptr, "Expected function declaration before variable declaration");
    ASSERT(Move(token, 1).type == TOKEN_TYPE_IDENTIFIER, "Expected identifier after data type");

    DeclareVariable var;
    var.function = function;
    var.type = DATA_TYPES.at(token.value);
    var.size = DATA_TYPE_SIZES.at(var.type);
    var.name = Move(token, 1).value;
    var.flags = VAR_FLAGS_NONE;
    var.value = "";
    return { CreateIL(IL_TYPE_DECLARE_VARIABLE, var), 2 };    
}

pair<vector<engine::IL_Instruction*>, size_t> engine::IL::AnalyzeReturn(const DeclareFunction* function, const Token& token) const {
    ASSERT(function != nullptr, "Expected function declaration before return keyword");

    const Token& src = Move(token, 1);

    FunctionReturn ret;
    ret.function = function;

    switch (src.type)
    {
    case TOKEN_TYPE_IDENTIFIER: {
        if (FindFunction(src.value) != nullptr) {
            vector<IL_Instruction*> ils;

            auto [il_call, il_size] = AnalyzeCall(function, src);

            DeclareVariable ret_var;
            ret_var.flags = VAR_FLAGS_NONE;
            ret_var.function = function;
            ret_var.name = "ret_" + to_string(getRandomId());
            ret_var.type = get<FunctionCall>(il_call->data).callee->ret_type;
            ret_var.size = DATA_TYPE_SIZES.at(ret_var.type);
            ret_var.value = "";
            
            IL_Instruction* il_var = CreateIL(IL_TYPE_DECLARE_VARIABLE, ret_var);
            get<FunctionCall>(il_call->data).ret = (const DeclareVariable*)&il_var->data;

            ret.var = (const DeclareVariable*)&il_var->data;
            IL_Instruction* il_ret = CreateIL(IL_TYPE_RETURN, ret);

            ils.push_back(il_var);
            ils.push_back(il_call);
            ils.push_back(il_ret);
            
            return { ils, il_size + 1 };
        }

        ret.var = FindVariable(function, src.value);
        
        if (ret.function->ret_type == DATA_TYPE_STR) {
            ASSERT(ret.var->type == DATA_TYPE_STR, "Expected string type at return statement of '%s'", function->name.data());
        } else {
            ASSERT(ret.var->type != DATA_TYPE_STR, "Expected number type at return statement of '%s'", function->name.data());
        }

        ASSERT(DATA_TYPE_SIZES.at(function->ret_type) >= ret.var->size, "Integer overflow at return statement of '%s'", function->name.data());
    } break;
    case TOKEN_TYPE_STRING: {
        ASSERT(function->ret_type == DATA_TYPE_STR, "Expected string type at return statement of '%s'", function->name.data());

        ret.var = MakeVariable(function, src);
        ASSERT(DATA_TYPE_SIZES.at(function->ret_type) >= ret.var->size, "Integer overflow at return statement of '%s'", function->name.data());
    } break;
    case TOKEN_TYPE_NUMBER: {
        ASSERT(function->ret_type != DATA_TYPE_STR, "Expected number type at return statement of '%s'", function->name.data());

        ret.var = MakeVariable(function, src);
        ASSERT(DATA_TYPE_SIZES.at(function->ret_type) >= ret.var->size, "Integer overflow at return statement of '%s'", function->name.data());
    } break;
    default: CRASH("Expected identifier after return keyword"); break;
    }

    return { { CreateIL(IL_TYPE_RETURN, ret) }, 2 };
}

uint64_t engine::IL::getImm(const string& value) {
    uint64_t num = 0;

    bool isHex = value.length() >= 3 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X');
    if (isHex) {
        auto [ptr, ec] = from_chars(value.data() + 2, value.data() + value.size(), num, 16);
        ASSERT(ec != errc::result_out_of_range, "Integer overflow");
    } 
    else {
        int64_t numSigned = stoll(value);
        ASSERT(numSigned >= 0, "Negative number is not valid for u64");

        num = (uint64_t)numSigned;
    }

    return num;
}

engine::DataType engine::IL::getImmType(const string& value) {
    uint64_t num = getImm(value);

    if (num <= UINT8_MAX) return DATA_TYPE_U8;
    else if (num <= UINT16_MAX) return DATA_TYPE_U16;
    else if (num <= UINT32_MAX) return DATA_TYPE_U32;
    else if (num <= UINT64_MAX) return DATA_TYPE_U64;

    CRASH("Unknown integer type");
}

bool engine::IL::isDataType(const Token& token) {
    return token.type == TOKEN_TYPE_KEYWORD && DATA_TYPES.find(token.value) != DATA_TYPES.end();
}


const engine::DeclareVariable* engine::IL::getArg(const DeclareFunction* function, const string& name) {
    for (const DeclareVariable& arg : function->args) {
        if (arg.name == name) {
            return &arg;
        }
    }

    return nullptr;
}

pair<engine::IL_Instruction*, size_t> engine::IL::AnalyzeOperator(const DeclareFunction* function, const Token& token) const {
    ASSERT(Move(token, -1).type == TOKEN_TYPE_IDENTIFIER, "Expected identifier before '=' operator");

    if (token.value == "=" && Move(token, -1).type != TOKEN_TYPE_OPERATOR) {
        const Token& left = Move(token, -1);
        const Token& right = Move(token, 1);

        EQSet set;
        set.function = function;
        set.left = FindVariable(function, left.value);
        
        switch (right.type)
        {
            case TOKEN_TYPE_IDENTIFIER: {
                if (const DeclareFunction* callee = FindFunction(right.value)) {
                    if (set.left->type == DATA_TYPE_STR) {
                        ASSERT(callee->ret_type == DATA_TYPE_STR, "Expected string type");
                    } else {
                        ASSERT(callee->ret_type != DATA_TYPE_STR, "Expected number type");
                    }

                    ASSERT(set.left->size >= DATA_TYPE_SIZES.at(callee->ret_type), "Integer overflow at '%s' < '%s' within '%s'", left.value.data(), right.value.data(), function->name.data());

                    auto [il_call, il_size] = AnalyzeCall(function, right);
                    get<FunctionCall>(il_call->data).ret = set.left;
                    
                    return { il_call, il_size };
                }
                else {
                    set.right = FindVariable(function, right.value);
                
                    if (set.left->type == DATA_TYPE_STR) {
                        ASSERT(set.right->type == DATA_TYPE_STR, "Expected string type");
                    } else {
                        ASSERT(set.right->type != DATA_TYPE_STR, "Expected number type");
                    }

                    ASSERT(set.left->size >= set.right->size, "Integer overflow at '%s' < '%s' within '%s'", left.value.data(), right.value.data(), function->name.data());
                }
            } break;
            case TOKEN_TYPE_STRING: {
                set.right = MakeVariable(function, right);
                ASSERT(set.left->type == DATA_TYPE_STR, "Expected number type");
            } break;
            case TOKEN_TYPE_NUMBER: {
                set.right = MakeVariable(function, right);
                ASSERT(set.left->type != DATA_TYPE_STR, "Expected string type");
                ASSERT(set.left->size >= set.right->size, "Integer overflow at '%s' < '%s' within '%s'", left.value.data(), right.value.data(), function->name.data());
            } break;
            default:
                CRASH("Unexpected token type");
                break;
        }
        
        return { CreateIL(IL_TYPE_EQ_SET, set), 2 };
    }

    return { nullptr, 1 };
}

pair<engine::IL_Instruction*, size_t> engine::IL::AnalyzeCall(const DeclareFunction* function, const Token& token) const {
    ASSERT(function != nullptr, "Expected function declaration before call");
    ASSERT(token.type == TOKEN_TYPE_IDENTIFIER, "Expected identifier before call");
    ASSERT(Move(token, 1).type == TOKEN_TYPE_ARG_START, "Expected '(' after call");

    const DeclareFunction* callee = FindFunction(token.value);
    ASSERT(callee != nullptr, "Function '%s' not found", token.value.data());

    size_t size = 2;

    vector<const DeclareVariable*> args;
    if (Move(token, size).type != TOKEN_TYPE_ARG_END) {
        while (true) {
            const Token& arg = Move(token, size);
            ASSERT(arg.type == TOKEN_TYPE_IDENTIFIER || arg.type == TOKEN_TYPE_NUMBER || arg.type == TOKEN_TYPE_STRING, "Expected identifier in argument");
            
            if (arg.type == TOKEN_TYPE_IDENTIFIER) {
                args.push_back(FindVariable(function, arg.value));
            }
            else {
                args.push_back(MakeVariable(function, arg));
            }

            size += 2;
            if (Move(token, size - 1).type != TOKEN_TYPE_NEW_ARG) {
                break;
            }
        }
    }

    if (args.size() < callee->args.size()) {
        CRASH("Too few arguments at call '%s' within '%s'", callee->name.data(), function->name.data());
    }
    else if (args.size() > callee->args.size()){ 
        CRASH("Too many arguments at call '%s' within '%s'", callee->name.data(), function->name.data());
    }
   
    for (size_t i = 0; i < args.size(); ++i) {
        const DeclareVariable* left = args.at(i);
        const DeclareVariable* right = &callee->args.at(i);

        if (left->type == DATA_TYPE_STR) {
            ASSERT(right->type == DATA_TYPE_STR, "Expected number type");
        } else {
            ASSERT(right->type != DATA_TYPE_STR, "Expected string type");
        }

        ASSERT(right->size >= left->size, "Integer overflow at call '%s' within '%s'", callee->name.data(), function->name.data());
    }

    FunctionCall call;
    call.function = function;
    call.callee = callee;
    call.args = move(args);
    return { CreateIL(IL_TYPE_FUNC_CALL, call), size };
}

pair<engine::IL_Instruction*, size_t> engine::IL::AnalyzeMacro(const DeclareFunction* function, const Token& token) const {
    ASSERT(function != nullptr, "Expected function declaration before macro");
    ASSERT(Move(token, 1).type == TOKEN_TYPE_IDENTIFIER, "Expected identifier after macro");


    if (Move(token, 1).value == "asm") {
        ASSERT(Move(token, 2).type == TOKEN_TYPE_ARG_START, "Expected '(' after asm macro");
        ASSERT(Move(token, 3).type == TOKEN_TYPE_STRING, "Expected string after asm macro");
        ASSERT(Move(token, 4).type == TOKEN_TYPE_ARG_END, "Expected ')' after asm macro");
    
        string code = Move(token, 3).value.substr(1, Move(token, 3).value.size() - 2);
        code = regex_replace(code, regex("\n\\s*"), "\n");
        code = regex_replace(code, regex("^\\s+|\\s+$"), "");

        InlineAsm inline_asm;
        inline_asm.function = function;
        inline_asm.code = code;
        
        return { CreateIL(IL_TYPE_INLINE_ASM, inline_asm), 6 };
    }

    CRASH("Unknown macro");
    return { nullptr, 2 };
}

const engine::DeclareFunction* engine::IL::FindFunction(const string& name) const {
    for (const IL_Instruction* il : m_ils) {
        if (il->type == IL_TYPE_DECLARE_FUNCTION) {
            const DeclareFunction& fn = get<DeclareFunction>(il->data);
            if (fn.name == name) {
                return (const DeclareFunction*)&fn;
            }
        }
    }

    return nullptr;
}

const engine::DeclareVariable* engine::IL::FindVariable(const DeclareFunction* function, const string& name) const {
    ASSERT(function != nullptr, "Expected function declaration");
    
    for (const IL_Instruction* il : m_ils) {
        if (il->type == IL_TYPE_DECLARE_VARIABLE) {
            const DeclareVariable& var = get<DeclareVariable>(il->data);
            if (var.function == function && var.name == name) {
                return (const DeclareVariable*)&var;
            }
        }
    }
    
    for (const DeclareVariable& arg : function->args) {
        if (arg.name == name) {
            return &arg;
        }
    }

    CRASH("Variable '%s' not found within '%s'", name.data(), function->name.data());
    return nullptr;
}

engine::DeclareVariable* engine::IL::MakeVariable(const DeclareFunction* function, const Token& token) const {
    ASSERT(function != nullptr, "Expected function declaration");
    ASSERT(token.type == TOKEN_TYPE_STRING || token.type == TOKEN_TYPE_NUMBER, "Expected string or number token");

    DeclareVariable* var = new DeclareVariable();
    var->function = function;
    var->flags = VAR_FLAGS_NONE;
    
    if (isDataType(Move(token, -1)) == true) {
        var->type = DATA_TYPES.at(Move(token, -1).value);
        var->name = Move(token, 1).value;
    }
    else {
        var->flags |= VAR_FLAGS_IMMEDIATE;
        var->name = "var_" + to_string(getRandomId());

        switch (token.type)
        {
        case TOKEN_TYPE_STRING: {
            var->type = DATA_TYPE_STR;
;
            var->value = token.value.substr(1, token.value.size() - 2);
        } break;
        case TOKEN_TYPE_NUMBER: {
            var->type = getImmType(token.value);
            var->value = token.value;
        } break;
        default:
            CRASH("Unexpected token type");
            break;
        }
    }

    var->size = DATA_TYPE_SIZES.at(var->type);
    return var;
}
