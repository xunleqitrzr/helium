#pragma once

#include <vector>
#include <optional>
#include <variant>

#include "./arena.hpp"
#include "./tokenization.hpp"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

/*struct NodeBinExprMulti {
    NodeExpr *lhs;
    NodeExpr *rhs;
};*/

struct NodeBinExpr {
    // std::variant<NodeBinExprAdd *, NodeBinExprMulti *> var;
    NodeBinExprAdd *add;
};

struct NodeTerm {
    std::variant<NodeTermIntLit *, NodeTermIdent *> var;
};

struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtVar {
    Token ident;
    NodeExpr *expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtVar *> var;
};

struct NodeProg {
    std::vector<NodeStmt *> stmts;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
            : m_tokens(std::move(tokens)),
              m_allocator(1024 * 1024 * 4) { // 4 megebytes allocated

    }

    std::optional<NodeTerm *> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        } else if (auto ident = try_consume(TokenType::ident)) {
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        } else {
            return {};
        }
    }

    std::optional<NodeExpr *> parse_expr() {
        if (auto term = parse_term()) {
            if (try_consume(TokenType::plus).has_value()) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                auto lhs_expr = m_allocator.alloc<NodeExpr>();
                lhs_expr->var = term.value();
                bin_expr_add->lhs = lhs_expr;
                if (auto rhs = parse_expr()) {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->add = bin_expr_add;
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = bin_expr;
                    return expr;
                } else {
                    std::cerr << "Right hand side of operator `+` expected" << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                auto expr = m_allocator.alloc<NodeExpr>();
                expr->var = term.value();
                return expr;
            }
        } else {
            return {};
        }
    }

    std::optional<NodeStmt *> parse_stmt() {
        if ((peek().value().type == TokenType::exit) && (peek(1).has_value()) &&
            (peek(1).value().type == TokenType::l_paren)) {
            consume();
            consume();
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::r_paren, "Closing parentheses `)` expected");
            try_consume(TokenType::semi, "Semicolon `;` expected");

            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        } else if ((peek().has_value() && peek().value().type == TokenType::var) && // var
                   (peek(1).has_value() && peek(1).value().type == TokenType::ident) && // var 'abc'
                   (peek(2).has_value() && peek(2).value().type == TokenType::eq)) { // var 'abc =
            consume();
            auto stmt_var = m_allocator.alloc<NodeStmtVar>();
            stmt_var->ident = consume();
            consume();
            if (auto expr = parse_expr()) {
                stmt_var->expr = expr.value();
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semi, "Semicolon `;` expected");

            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_var;
            return stmt;
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

    inline Token try_consume(TokenType type, const std::string &err_msg) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    inline std::optional<Token> try_consume(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            return {};
        }
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    ArenaAllocator m_allocator;
};