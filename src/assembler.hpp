#ifndef HPP_ASSEMBLER
#define HPP_ASSEMBLER

#include "engine.hpp"

using namespace std;

namespace engine {
    class Assembler {
        public:
            Assembler();
            Assembler(const vector<IL_Instruction*>& ils);
            ~Assembler();

            void assemble();
            void label(const string& name);
            void add(const string& dst, const string& src);
            void sub(const string& dst, const string& src);
            void mov(const string& dst, const string& src);
            void push(const string& src);
            void pop(const string& dst);
            void call(const string& dst);
            void ret();
            void _int(const string& value);

            void create(const string& filename) const;

        private:
            string m_output;
            vector<IL_Instruction*> m_ils;
    };
}

#endif