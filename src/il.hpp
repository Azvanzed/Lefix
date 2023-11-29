#ifndef HPP_LI
#define HPP_LI

#include "tokenizer.hpp"
#include <variant>

using namespace std;

namespace engine {
    enum DataType {
        DATA_TYPE_I64,
        DATA_TYPE_I32,
        DATA_TYPE_I16,
        DATA_TYPE_I8,
        DATA_TYPE_U64,
        DATA_TYPE_U32,
        DATA_TYPE_U16,
        DATA_TYPE_U8,
        DATA_TYPE_STR,
        DATA_TYPE_BOOL
    };

    enum VarFlags {
        VAR_FLAGS_NONE = 0,
        VAR_FLAGS_ARG = (1 << 0),
    };

    const static unordered_map<string, DataType> DATA_TYPES = {
        { "i64", DATA_TYPE_I64 },
        { "i32", DATA_TYPE_I32 },
        { "i16", DATA_TYPE_I16 },
        { "i8", DATA_TYPE_I8 },
        { "u64", DATA_TYPE_U64 },
        { "u32", DATA_TYPE_U32 },
        { "u16", DATA_TYPE_U16 },
        { "u8", DATA_TYPE_U8 },
        { "str", DATA_TYPE_STR },
        { "bool", DATA_TYPE_BOOL }
    };

    const static unordered_map<DataType, uint8_t> DATA_TYPE_SIZES = {
        { DATA_TYPE_I64, 64 },
        { DATA_TYPE_I32, 32 },
        { DATA_TYPE_I16, 16 },
        { DATA_TYPE_I8, 8 },
        { DATA_TYPE_U64, 64 },
        { DATA_TYPE_U32, 32 },
        { DATA_TYPE_U16, 16 },
        { DATA_TYPE_U8, 8 },
        { DATA_TYPE_STR, 64 },
        { DATA_TYPE_BOOL, 8 }
    };

    struct DeclareVariable {
        const struct DeclareFunction* function;
        DataType type;
        size_t size;
        string name;
        string value;
        VarFlags flags;
    };

    struct DeclareFunction {
        string name;
        DataType ret_type;
        vector<DeclareVariable> args;
    };

    struct FunctionReturn {
        const DeclareFunction* function;
        const DeclareVariable* var;
    };

    struct EQSet {
        const DeclareVariable* left;
        const DeclareVariable* right;
    };

    struct FunctionCall {
        const DeclareFunction* function;
        const DeclareFunction* callee;
        const DeclareVariable* ret;
        vector<const DeclareVariable*> args;
    };
    
    enum InstructionType {
        IL_TYPE_UNKNOWN,
        IL_TYPE_DECLARE_VARIABLE,
        IL_TYPE_DECLARE_FUNCTION,
        IL_TYPE_RETURN,
        IL_TYPE_EQ_SET,
        IL_TYPE_FUNC_CALL
    };

    struct IL_Instruction {
        uint64_t id;
        InstructionType type;
        variant<DeclareVariable, DeclareFunction, FunctionReturn, EQSet, FunctionCall> data;

        IL_Instruction() {}
        ~IL_Instruction() {}
    };
    
    class IL {
        public:
            IL(const vector<Token>& tokens);
            ~IL();

            void analyze();
            void optimize();
            
            vector<IL_Instruction*> getILs() const;
            
            [[nodiscard]] static uint16_t getRandomId();
            [[nodiscard]] static uint64_t getImm(const string& value);
            [[nodiscard]] static DataType getImmType(const string& value);
            [[nodiscard]] static bool isDataType(const Token& token);
            [[nodiscard]] static const DeclareVariable* getArg(const DeclareFunction* function, const string& name);

        private:

            [[nodiscard]] const Token& Move(const Token& token, int64_t times) const;
            [[nodiscard]] IL_Instruction* CreateIL(InstructionType type, const auto& data) const;

            [[nodiscard]] pair<IL_Instruction*, size_t> AnalyzeDeclareFunction(const Token& token) const;
            [[nodiscard]] pair<IL_Instruction*, size_t> AnalyzeDeclareVariable(const DeclareFunction* function, const Token& token) const; 
            [[nodiscard]] pair<IL_Instruction*, size_t> AnalyzeReturn(const DeclareFunction* function, const Token& token) const;
            [[nodiscard]] pair<IL_Instruction*, size_t> AnalyzeOperator(const DeclareFunction* function, const Token& token) const;
            [[nodiscard]] pair<IL_Instruction*, size_t> AnalyzeCall(const DeclareFunction* function, const Token& token) const;

            [[nodiscard]] const DeclareFunction* FindFunction(const string& name) const;

            [[nodiscard]] const DeclareVariable* FindVariable(const DeclareFunction* function, const string& name) const;
            [[nodiscard]] DeclareVariable* MakeVariable(const DeclareFunction* function, const Token& token) const;

            vector<Token> m_tokens;
            vector<IL_Instruction*> m_ils;
    };
}

#endif