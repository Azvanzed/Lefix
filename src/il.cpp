#include "il.hpp"
#include <iostream>
#include <random>
#include "assert.hpp"

using namespace std;

engine::IL::IL(const vector<Token>& tokens) {
    m_tokens = move(tokens);
}

engine::IL::~IL() {
    for (IL_Instruction* insn : m_ils) {
        delete insn;
    }
}

void engine::IL::analyze() {
    DeclareFunction* function = nullptr;
    
    for (size_t i = 0; i < m_tokens.size();) {
        const Token& token = m_tokens.at(i);

        switch (token.type) {
            case TOKEN_TYPE_KEYWORD: {
                if (token.value == "fn") {
                    auto [il, size] = AnalyzeDeclareFunction(token);
                    function = (DeclareFunction*)&il->data;
                    m_ils.push_back(il);
                    i += size;
                    }
                else if (isDataType(token) == true) {
                    auto [il, size] = AnalyzeDeclareVariable(function, token);
                    m_ils.push_back(il);
                    i += size;
                }
                else if (token.value == "ret") {
                    auto [il, size] = AnalyzeReturn(function, token);
                    m_ils.push_back(il);
                    i += size;
                }
            } break;
            case TOKEN_TYPE_OPERATOR: {
                auto [il, size] = AnalyzeOperator(function, token);
                m_ils.push_back(il);
                i += size;
            } break;
            default: ++i; break;
        }
    }
}

void engine::IL::optimize() {
    // TODO
}

vector<engine::IL_Instruction*> engine::IL::getILs() const {
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
    ASSERT(index < m_tokens.size(), "Index out of bounds");
    
    return m_tokens.at(index);
}

uint16_t engine::IL::getRandomId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT16_MAX);
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
    IL_Instruction* il = CreateIL(IL_TYPE_DECLARE_FUNCTION, fn);

    DeclareVariable* ret = new DeclareVariable;
    ret->function = (const DeclareFunction*)&il->data;
    ret->type = DATA_TYPES.at(Move(token, 1).value);
    ret->name = "#";
    ret->value = "";
    ret->size = DATA_TYPE_SIZES.at(ret->type);
    ((DeclareFunction*)&il->data)->ret = ret;

    size_t size = 4;
    if (Move(token, size).type != TOKEN_TYPE_ARG_END) {
        while (true) {
            const Token& arg = Move(token, size);
            ASSERT(isDataType(arg) == true, "Expected data type in argument");
            ASSERT(Move(token, size + 1).type == TOKEN_TYPE_IDENTIFIER, "Expected identifier after data type in argument");

            DeclareVariable* arg_var = new DeclareVariable();
            arg_var->function =  nullptr;
            arg_var->type = DATA_TYPES.at(arg.value);
            arg_var->name = Move(token, size + 1).value;
            arg_var->value = "";
            arg_var->size = DATA_TYPE_SIZES.at(arg_var->type);
            ((DeclareFunction*)&il->data)->args.push_back(arg_var);

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
    return { CreateIL(IL_TYPE_DECLARE_VARIABLE, var), 2 };    
}

pair<engine::IL_Instruction*, size_t> engine::IL::AnalyzeReturn(const DeclareFunction* function, const Token& token) const {
    ASSERT(function != nullptr, "Expected function declaration before return keyword");

    const Token& src = Move(token, 1);


    FunctionReturn ret;
    ret.function = function;

    switch (src.type)
    {
    case TOKEN_TYPE_IDENTIFIER: ret.var = (const DeclareVariable*)&FindVariable(function, src.value); break;
    case TOKEN_TYPE_STRING:
    case TOKEN_TYPE_NUMBER: ret.var = MakeVariable(function, src); break;
    default:
        ASSERT(false, "Expected identifier after return keyword");
        break;
    }

    return { CreateIL(IL_TYPE_RETURN, ret), 2 };
}

uint64_t engine::IL::getImm(const string& value) {
    if (value.length() >= 3 && value[0] == '0' && value[1] == 'x') {
        return strtoull(value.c_str(), nullptr, 16);
    } 
    
    return strtoull(value.c_str(), nullptr, 10);
}

engine::DataType engine::IL::getImmType(const string& value) {
    uint64_t num = getImm(value);

    if (num <= UINT8_MAX) return DATA_TYPE_U8;
    else if (num <= UINT16_MAX) return DATA_TYPE_U16;
    else if (num <= UINT32_MAX) return DATA_TYPE_U32;
    return DATA_TYPE_U64;
}

[[nodiscard]] bool engine::IL::isDataType(const Token& token) {
    return token.type == TOKEN_TYPE_KEYWORD && DATA_TYPES.find(token.value) != DATA_TYPES.end();
}

pair<engine::IL_Instruction*, size_t> engine::IL::AnalyzeOperator(const DeclareFunction* function, const Token& token) const {
    ASSERT(Move(token, -1).type == TOKEN_TYPE_IDENTIFIER, "Expected identifier before '=' operator");

    if (token.value == "=" && Move(token, -1).type != TOKEN_TYPE_OPERATOR) {
        const Token& left = Move(token, -1);
        const Token& right = Move(token, 1);

        EQSet set;
        set.left = (const DeclareVariable*)&FindVariable(function, left.value);

        switch (right.type)
        {
            case TOKEN_TYPE_IDENTIFIER: {
                set.right = (const DeclareVariable*)&FindVariable(function, right.value);
                ASSERT(set.left->type == set.right->type, "Expected same data type");
            } break;
            case TOKEN_TYPE_STRING: {
                set.right = MakeVariable(function, right);
                ASSERT(set.left->type == DATA_TYPE_STR, "Expected number type");
            } break;
            case TOKEN_TYPE_NUMBER: {
                set.right = MakeVariable(function, right);
                ASSERT(set.left->type != DATA_TYPE_STR, "Expected string type");
            } break;
            default:
                ASSERT(false, "Unexpected token type");
                break;
        }
        
        return { CreateIL(IL_TYPE_EQ_SET, set), 2 };
    }

    return { nullptr, 1 };
}

const engine::DeclareVariable& engine::IL::FindVariable(const DeclareFunction* function, const string& name) const {
    ASSERT(function != nullptr, "Expected function declaration");
    
    for (const IL_Instruction* il : m_ils) {
        if (il->type == IL_TYPE_DECLARE_VARIABLE) {
            const DeclareVariable& var = get<DeclareVariable>(il->data);
            if (var.function == function && var.name == name) {
                return var;
            }
        }
    }

    ASSERT(false, "Variable not found");
}

engine::DeclareVariable* engine::IL::MakeVariable(const DeclareFunction* function, const Token& token) const {
    ASSERT(function != nullptr, "Expected function declaration");

    DeclareVariable* var = new DeclareVariable();
    var->function = function;
    
    if (isDataType(Move(token, -1)) == true) {
        var->type = DATA_TYPES.at(Move(token, -1).value);
        var->name = Move(token, 1).value;
    }
    else {
        var->name = "var_" + to_string(getRandomId());

        switch (token.type)
        {
        case TOKEN_TYPE_STRING: {
            var->type = DATA_TYPE_STR;
            var->value = token.value.substr(1, token.value.size() - 2);
        } break;
        case TOKEN_TYPE_NUMBER: {
            var->type = Move(token, -1).value == "ret" ? function->ret->type : getImmType(token.value);
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