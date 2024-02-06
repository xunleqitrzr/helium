#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>

enum class TokenType {
    _return,
    int_lit,
    semi
};

struct Token {
    TokenType type;
    std::optional<std::string> value {};
};

std::vector<Token> tokenize(const std::string& str) {
    std::vector<Token> tokens;

    std::string buf;

    for (int i = 0; i < str.length(); i++) {
        char c = str.at(i);
        if (std::isalpha(c)) {
            buf.push_back(c);
            i++;
            while (std::isalnum(str.at(i))) {
                buf.push_back(str.at(i));
                i++;
            }
            i--;

            if (buf == "return") {
                tokens.push_back({.type = TokenType::_return});
                buf.clear();
                continue;
            } else {
                std::cerr << "Incorrect syntax" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (std::isdigit(c)) {
            buf.push_back(c);
            i++;
            while (std::isdigit(str.at(i))) {
                buf.push_back(str.at(i));
                i++;
            }
            i--;
            tokens.push_back({.type = TokenType::int_lit, .value = buf});
            buf.clear();
        }
        else if (c == ';') {
            tokens.push_back({.type = TokenType::semi});
            buf.clear();
        }
        else if (std::isspace(c)) {
            continue;
        } else {
            std::cerr << "Incorrect syntax" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return tokens;
}

std::string tokens_to_asm(const std::vector<Token>& tokens) {
    std::stringstream output;
    output << "global _start\n_start:\n";
    for (int i = 0; i < tokens.size(); i++) {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::_return) {
            if ((i + 1 < tokens.size()) && (tokens.at(i + 1).type == TokenType::int_lit)) {
                if ((i + 2 < tokens.size()) && (tokens.at(i + 2).type == TokenType::semi)) {
                    output << "    mov rax, 60\n";
                    output << "    mox rdi, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }

    return output.str();
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Incorrect usage" << std::endl;
        std::cerr << "Correct usage:\t./helium <file.he>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::fstream input(argv[1], std::ios::in);
        std::stringstream content_stream;
        content_stream << input.rdbuf();
        contents = content_stream.str();
    }

    std::vector<Token> tokens = tokenize(contents);
    {
        std::fstream file("../out.asm", std::ios::out);
        file << tokens_to_asm(tokens);
    }

    return EXIT_SUCCESS;
}