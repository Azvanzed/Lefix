#ifndef HPP_TOKENIZER
#define HPP_TOKENIZER

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

namespace engine {
    enum TokenType {
        TOKEN_TYPE_UNKNOWN,
        TOKEN_TYPE_KEYWORD,
        TOKEN_TYPE_IDENTIFIER,
        TOKEN_TYPE_STRING,
        TOKEN_TYPE_NUMBER,
        TOKEN_TYPE_EOF,
        TOKEN_TYPE_OPERATOR,
        TOKEN_TYPE_MACRO,
        TOKEN_TYPE_ARG_START,
        TOKEN_TYPE_ARG_END,
        TOKEN_TYPE_NEW_ARG,
        TOKEN_TYPE_SCOPE_START,
        TOKEN_TYPE_SCOPE_END,
    };

    struct Token {
        uint64_t id;
        TokenType type;
        string value;
    };

    const static unordered_map<string, TokenType> KEYWORDS = {
        { "ret", TOKEN_TYPE_KEYWORD },
        { "fn", TOKEN_TYPE_KEYWORD },
        { "i64", TOKEN_TYPE_KEYWORD },
        { "i32", TOKEN_TYPE_KEYWORD },
        { "i16", TOKEN_TYPE_KEYWORD },
        { "i8", TOKEN_TYPE_KEYWORD },
        { "u64", TOKEN_TYPE_KEYWORD },
        { "u32", TOKEN_TYPE_KEYWORD },
        { "u16", TOKEN_TYPE_KEYWORD },
        { "u8", TOKEN_TYPE_KEYWORD },
        { "str", TOKEN_TYPE_KEYWORD },
        { "bool", TOKEN_TYPE_KEYWORD }
    };

    class Tokenizer {
        public:
            Tokenizer(const string& code);
            ~Tokenizer();

            void cleanup();
            void tokenize();

            [[nodiscard]] const vector<Token>& getTokens() const;
            
        private:    
            
            [[nodiscard]] static uint16_t getRandomId();

            
            vector<Token> m_tokens;
            string m_code;
    };
}

#endif