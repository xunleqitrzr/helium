#pragma once

#include <vector>
#include <optional>
#include <variant>

#include "./tokenization.hpp"

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr;

struct BinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct BinExprMulti {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct BinExpr {
    std::variant<BinExprAdd*, BinExprMulti*> var;
};

struct NodeExpr {
    std::variant<NodeExprIntLit*, NodeExprIdent*, BinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmtVar {
    Token ident;
    NodeExpr expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit, NodeStmtVar> var;
};

struct NodeProg {
    std::vector<NodeStmt> stmts;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)){

    }

    std::optional<NodeExpr> parse_expr() {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            return NodeExpr{.var = NodeExprIntLit {.int_lit = consume()}};
        }
        else if (peek().has_value() && peek().value().type == TokenType::ident) {
            return NodeExpr {.var = NodeExprIdent {.ident = consume()}};
        } else {
            return {};
        }
    }

    std::optional<NodeStmt> parse_stmt() {
        if ((peek().value().type == TokenType::exit) && (peek(1).has_value()) && (peek(1).value().type == TokenType::l_paren)) {
            consume();
            consume();
            NodeStmtExit stmt_exit;
            if (auto node_expr = parse_expr()) {
                stmt_exit =  {.expr = node_expr.value()};
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if ((peek().has_value()) && (peek().value().type == TokenType::r_paren)) {
                consume();
            } else {
                std::cerr << "Closing parentheses `)` expected" << std::endl;
                exit(EXIT_FAILURE);
            }
            if ((peek().has_value()) && (peek().value().type == TokenType::semi)) {
                consume();
            } else {
                std::cerr << "Semicolon `;` expected" << std::endl;
                exit(EXIT_FAILURE);
            }
            return NodeStmt {.var = stmt_exit};
        } else if ((peek().has_value() && peek().value().type == TokenType::var) && // var
                (peek(1).has_value() && peek(1).value().type == TokenType::ident) && // var 'abc'
                (peek(2).has_value() && peek(2).value().type == TokenType::eq)) { // var 'abc =
            consume();
            auto stmt_let = NodeStmtVar {.ident = consume()};
            consume();
            if (auto expr = parse_expr()) {
                stmt_let.expr = expr.value();
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            } else {
                std::cerr << "Semicolon `;` expected" << std::endl;
                exit(EXIT_FAILURE);
            }
            return NodeStmt {.var = stmt_let};
        } else {
            return {};
        }
    }

    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                std::cerr << "Invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:

    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index + offset);
        }
    }

    inline Token consume() {
        return m_tokens.at(m_index++);
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;
};