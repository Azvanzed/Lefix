#include "tokenizer.hpp"
#include "assert.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>
#include <random>
#include <immintrin.h>

using namespace std;

engine::Tokenizer::Tokenizer(const string& code) 
                            : m_code(move(code)) {
    m_tokens.clear();
}

engine::Tokenizer::~Tokenizer() {

}

void engine::Tokenizer::cleanup() {
    // remove comments. eg: # hello world
    m_code = regex_replace(m_code, regex("#.*"), "");
}

[[nodiscard]] uint16_t engine::Tokenizer::getRandomId() {
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dis(10000, 99999);
    return dis(gen);
}

void engine::Tokenizer::tokenize() {
    const static string operators = "=}+-*/%^&|~!<>";

    auto addToken = [&](size_t* i, TokenType type, size_t length = 1) {
        string value = m_code.substr(*i, length);
        uint64_t id = getRandomId();

        m_tokens.emplace_back(id, type, value);
        *i += length;
    };

    for (size_t i = 0; i < m_code.size();) {
        char c = m_code[i];

        if (isspace(c)) {
            ++i;
        } else if (c == '(' || c == ')') {
            addToken(&i, c == '(' ? TOKEN_TYPE_ARG_START : TOKEN_TYPE_ARG_END);
        } else if (c == ',') {
            addToken(&i, TOKEN_TYPE_NEW_ARG);
        } else if (c == '{' || c == '}') {
            addToken(&i, c == '{' ? TOKEN_TYPE_SCOPE_START : TOKEN_TYPE_SCOPE_END);
        } else if (c == '"') {
            size_t j = m_code.find('"', i + 1);
            ASSERT(j != string::npos, "Expected closing '\"'");
            addToken(&i, TOKEN_TYPE_STRING, j - i + 1);
        } else if (c == '0' && (i + 1 < m_code.size()) && m_code[i + 1] == 'x') {
            size_t j = i + 2;
            while (j < m_code.size() && isxdigit(m_code[j])) ++j;
            addToken(&i, TOKEN_TYPE_NUMBER, j - i);
        } else if (isdigit(c) && !(i > 0 && m_code[i - 1] == '0' && m_code[i] == 'x')) {
            size_t j = i;
            while (j < m_code.size() && isdigit(m_code[j])) ++j;
            addToken(&i, TOKEN_TYPE_NUMBER, j - i);
        } else if (isalpha(c) || c == '_') {
            size_t j = i;
            while (j < m_code.size() && (isalnum(m_code[j]) || m_code[j] == '_')) ++j;
            
            string value = m_code.substr(i, j - i);
            TokenType type = (KEYWORDS.find(value) != KEYWORDS.end()) ? KEYWORDS.at(value) : TOKEN_TYPE_IDENTIFIER;
            addToken(&i, type, j - i);
        } else if (operators.find(c) != string::npos) {
            addToken(&i, TOKEN_TYPE_OPERATOR);
        } else {
            size_t j = i;
            while (j < m_code.size() && !isspace(m_code[j]) && !isalnum(m_code[j]) && m_code[j] != '_' && m_code[j] != '"') ++j;
            addToken(&i, TOKEN_TYPE_UNKNOWN, j - i);
        }
    }
}

const vector<engine::Token>& engine::Tokenizer::getTokens() const {
    return move(m_tokens);
}