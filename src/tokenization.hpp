#pragma once

#include <string>
#include <vector>
#include <optional>

enum class TokenType {
    exit,
    int_lit,
    semi,
    l_paren,
    r_paren,
    ident,
    var,
    eq,
    star,
    div,
    plus,
    sub
};

std::optional<int> bin_prec(TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::sub:
            return 0;
        case TokenType::star:
        case TokenType::div:
            return 1;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    std::optional<std::string> value{};
};

class Tokenizer {
public:
    inline explicit Tokenizer(std::string src) : m_src(std::move(src)) {

    }

    inline std::vector<Token> tokenize() {
        std::string buf;
        std::vector<Token> tokens;

        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") {
                    tokens.push_back({.type = TokenType::exit});
                    buf.clear();
                }
                else if (buf == "var") {
                    tokens.push_back({.type = TokenType::var});
                    buf.clear();
                }
                else {
                    tokens.push_back({.type = TokenType::ident, .value = buf});
                    buf.clear();
                }
            }
            else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({.type = TokenType::int_lit, .value = buf});
                buf.clear();
            }
            else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TokenType::l_paren});
            }
            else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = TokenType::r_paren});
            }
            else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::semi});
            }
            else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TokenType::eq});
            }
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({.type = TokenType::star});
            }
            else if (peek().value() == '/') {
                consume();
                tokens.push_back({.type = TokenType::div});
            }
            else if (peek().value() == '+') {
                consume();
                tokens.push_back({.type = TokenType::plus});
            }
            else if (peek().value() == '-') {
                consume();
                tokens.push_back({.type = TokenType::sub});
            }
            else if (std::isspace(peek().value())) {
                consume();
            }
            else {
                std::cerr << "Incorrect syntax" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:

    [[nodiscard]] inline std::optional<char> peek(int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        }
        else {
            return m_src.at(m_index + offset);
        }
    }

    inline char consume() {
        return m_src.at(m_index++);
    }

    const std::string m_src;
    size_t m_index = 0;
};