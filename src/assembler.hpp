#ifndef HPP_ASSEMBLER
#define HPP_ASSEMBLER

#include "engine.hpp"
#include "il.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <map>

using namespace std;

namespace engine {
    enum AsmLocalType {
        ASM_LOCAL_TYPE_NONE = 0,
        ASM_LOCAL_TYPE_IMMEDIATE
    };

    struct AsmLocal {
        AsmLocalType type;
        int64_t offset;
        uint8_t size;
        
        union {
            uint8_t b8;
            uint16_t b16;
            uint32_t b32;
            uint64_t b64;
        }imm;
    };

    struct AsmRoutine {
        string name;
        size_t stack_size;
        unordered_map<const DeclareVariable*, const AsmLocal*> stack; 
        vector<const IL_Instruction*> insns; 
    };
    
    class Assembler {
        public:

            Assembler();
            Assembler(const vector<const IL_Instruction*>& ils);
            ~Assembler();

            void translate();
            void optimize();
            void assemble();
            void create(const string& filename) const;
            
        private:
            void global(const string& name, const string& comment = "");
            void label(const string& name, const string& comment = "");
            void add(const string& dst, const string& src, const string& comment = "");
            void sub(const string& dst, const string& src, const string& comment = "");
            void _xor(const string& dst, const string& src, const string& comment = "");
            void shr(const string& dst, const string& src, const string& comment = "");
            void shl(const string& dst, const string& src, const string& comment = "");
            void div(const string& src, const string& comment = "");
            void mul(const string& src, const string& comment = "");
            void insert(const string& code, const string& comment = "");
            void mov(const string& dst, const string& src, const string& comment = "");
            void push(const string& src, const string& comment = "");
            void pop(const string& dst, const string& comment = "");
            void call(const string& dst, const string& comment = "");
            void _ret(const string& comment = "");
            void _int(const string& value, const string& comment = "");

            [[nodiscard]] static string getMemSize(size_t size);
            [[nodiscard]] static string getGP0(size_t size);
            [[nodiscard]] static size_t AlignStack(size_t offset, size_t size);

        private:
            string m_output;
            vector<const IL_Instruction*> m_ils;
            vector<AsmRoutine*> m_routines;
    };
}

#endif