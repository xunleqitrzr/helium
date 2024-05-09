#pragma once

#include <vector>
#include <optional>
#include <variant>

#include "./arena.hpp"
#include "./tokenization.hpp"
#include "./color.hpp"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeTermParen {
    NodeExpr* expr;
};

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMulti {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMulti*, NodeBinExprDiv*, NodeBinExprSub*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtVar {
    Token ident;
    NodeExpr* expr{};
};

struct NodeStmt;

struct NodeScope {
    std::vector<NodeStmt*> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif {
    NodeExpr* expr{};
    NodeScope* scope{};
    std::optional<NodeIfPred*> pred;
};
struct NodeIfPredElse {
    NodeScope* scope;
};

struct NodeIfPred {
    std::variant<NodeIfPredElif*,NodeIfPredElse*> var;
};

struct NodeStmtIf {
    NodeExpr* expr{};
    NodeScope* scope{};
    std::optional<NodeIfPred*> pred;
};

struct NodeStmtAssign {
    Token ident;
    NodeExpr* expr{};
};

struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtVar*, NodeScope*, NodeStmtIf*, NodeStmtAssign*> var;
};

struct NodeProg {
    std::vector<NodeStmt*> stmts;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens)
            : m_tokens(std::move(tokens)),
              m_allocator(1024 * 1024 * 4) { // 4 megebytes allocated

    }

    void error_expected(const std::string& msg) const {
        std::cerr << RED <<"[Parse Error] " << YELLOW "Expected " << CYAN << msg << YELLOW << " on line " << CYAN << peek(-1).value().line << RESET << std::endl;
        exit(EXIT_FAILURE);
    }

    std::optional<NodeTerm*> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.emplace<NodeTermIntLit>(int_lit.value());
            auto term = m_allocator.emplace<NodeTerm>(term_int_lit);
            return term;
        }
        if (auto ident = try_consume(TokenType::ident)) {
            auto expr_ident = m_allocator.emplace<NodeTermIdent>(ident.value());
            auto term = m_allocator.emplace<NodeTerm>(expr_ident);
            return term;
        }
        if (const auto open_paren = try_consume(TokenType::l_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                error_expected("';'");
            }
            try_consume_err(TokenType::r_paren);
            auto term_paren = m_allocator.emplace<NodeTermParen>(expr.value());
            auto term = m_allocator.emplace<NodeTerm>(term_paren);
            return term;
        }
        return {};
    }

    std::optional<NodeExpr*> parse_expr(const int min_prec = 0) {
        std::optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }
        auto expr_lhs = m_allocator.emplace<NodeExpr>(term_lhs.value());

        while (true) {
            std::optional<Token> curr_tok = peek();
            std::optional<int> prec;
            if (curr_tok.has_value()) {
                prec = bin_prec(curr_tok->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            }
            else {
                break;
            }

            const auto [type, line, value] = consume();
            const int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);
            if (!expr_rhs.has_value()) {
                error_expected("legal expression");
                exit(EXIT_FAILURE);
            }
            auto expr = m_allocator.emplace<NodeBinExpr>();
            auto expr_lhs2 = m_allocator.emplace<NodeExpr>();
            if (type == TokenType::plus) {
                expr_lhs2->var = expr_lhs->var;
                auto add = m_allocator.emplace<NodeBinExprAdd>(expr_lhs2, expr_rhs.value());
                expr->var = add;
            }
            else if (type == TokenType::star) {
                expr_lhs2->var = expr_lhs->var;
                auto multi = m_allocator.emplace<NodeBinExprMulti>(expr_lhs2, expr_rhs.value());
                expr->var = multi;
            }
            else if (type == TokenType::minus) {
                expr_lhs2->var = expr_lhs->var;
                auto sub = m_allocator.emplace<NodeBinExprSub>(expr_lhs2, expr_rhs.value());
                expr->var = sub;
            }
            else if (type == TokenType::fslash) {
                expr_lhs2->var = expr_lhs->var;
                auto div = m_allocator.emplace<NodeBinExprDiv>(expr_lhs2, expr_rhs.value());
                expr->var = div;
            }
            else {
                assert(false); // unreachable
            }
            expr_lhs->var = expr;
        }
        return expr_lhs;
    }

    std::optional<NodeScope*> parse_scope() {
        if (!try_consume(TokenType::l_curly).has_value()) {
            return {};
        }
        auto scope = m_allocator.emplace<NodeScope>();
        while (auto stmt = parse_stmt()) {
            scope->stmts.push_back(stmt.value());
        }
        try_consume_err(TokenType::r_curly);
        return scope;
    }

    std::optional<NodeIfPred*> parse_if_pred() {
        if (try_consume(TokenType::elif)) {
            try_consume_err(TokenType::l_paren);
            const auto elif = m_allocator.alloc<NodeIfPredElif>();
            if (const auto expr = parse_expr()) {
                elif->expr = expr.value();
            } else {
                error_expected("expression in PAREN (expr)");
            }
            try_consume_err(TokenType::r_paren);
            if (const auto scope = parse_scope()) {
                elif->scope = scope.value();
            } else {
                error_expected("SCOPE {scope}");
            }
            elif->pred = parse_if_pred();
            auto pred = m_allocator.emplace<NodeIfPred>(elif);
            return pred;
        }
        if (try_consume(TokenType::else_)) {
            auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (const auto scope = parse_scope()) {
                else_->scope = scope.value();
            } else {
                error_expected("SCOPE {scope}");
            }
            auto pred = m_allocator.emplace<NodeIfPred>(else_);
            return pred;
        }
        return {};
    }

    std::optional<NodeStmt*> parse_stmt() {
        if ( (peek().has_value()) && (peek().value().type == TokenType::exit) && (peek(1).has_value()) &&
            (peek(1).value().type == TokenType::l_paren)) {
            consume();
            consume();
            auto stmt_exit = m_allocator.emplace<NodeStmtExit>();
            if (const auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            }
            else {
                error_expected("valid expression");
            }
            try_consume_err(TokenType::r_paren);
            try_consume_err(TokenType::semi);

            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        }
        if ((peek().has_value() && peek().value().type == TokenType::var) && // var
            (peek(1).has_value() && peek(1).value().type == TokenType::ident) && // var 'abc'
            (peek(2).has_value() && peek(2).value().type == TokenType::eq)) { // var 'abc =
            consume();
            auto stmt_var = m_allocator.emplace<NodeStmtVar>();
            stmt_var->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                stmt_var->expr = expr.value();
            }
            else {
                error_expected("valid expression");
            }
            try_consume_err(TokenType::semi);

            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_var;
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::ident && peek(1).has_value() &&
            peek(1).value().type == TokenType::eq) {
            const auto assign = m_allocator.alloc<NodeStmtAssign>();
            assign->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                assign->expr = expr.value();
            } else {
                error_expected("expression");
            }
            try_consume_err(TokenType::semi);
            auto stmt = m_allocator.emplace<NodeStmt>(assign);
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::l_curly) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.emplace<NodeStmt>(scope.value());
                return stmt;
            }
            error_expected("valid SCOPE {scope}");
        }
        if (auto if_ = try_consume(TokenType::if_)) {
            try_consume_err(TokenType::l_paren);
            auto stmt_if = m_allocator.emplace<NodeStmtIf>();
            if (const auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            } else {
                error_expected("valid expression");
            }
            try_consume_err(TokenType::r_paren);
            if (const auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            } else {
                error_expected("valid SCOPE {scope}");
            }
            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.emplace<NodeStmt>(stmt_if);
            return stmt;
        }
        return {};
    }

    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            }
            else {
                error_expected("valid statement");
            }
        }
        return prog;
    }

private:

    [[nodiscard]] std::optional<Token> peek(const int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        return m_tokens.at(m_index + offset);
    }

    Token consume() {
        return m_tokens.at(m_index++);
    }

    Token try_consume_err(const TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        error_expected(to_string(type));
        return {};
    }

    std::optional<Token> try_consume(const TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        return {};
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    ArenaAllocator m_allocator;
};