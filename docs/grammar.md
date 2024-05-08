$$
\begin{align}
    [\text{Prog}] &\to [\text{Stmt}]^*
    \\
    [\text{Stmt}] &\to 
    \begin {cases}
        \text{exit}([\text{Expr}]); \\
        \text{var}\space \text{identifier} = [\text{Expr}]; \\
        \text{if}\ ([\text{Expr}])\ [\text{Scope}] \\
        [\text{Scope}] \\
    \end {cases}
    \\
    [\text{Scope}] &\to 
    \begin{cases}
        \{[\text{Stmt}]^*\}
    \end{cases}
    \\
    [\text{Expr}] &\to 
    \begin {cases}
        [\text{Term}] \\
        [\text{BinExpr}]
    \end {cases}
    \\
    [\text{BinExpr}] &\to
    \begin {cases}
        [\text{Expr}] * [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] \div [\text{Expr}] & \text{prec} = 1 \\
        [\text{Expr}] + [\text{Expr}] & \text{prec} = 0 \\
        [\text{Expr}] - [\text{Expr}] & \text{prec} = 0 \\
    \end {cases}
    \\
    [\text{Term}] &\to 
    \begin {cases}
        \text{int-lit} \\
        \text{identifier} \\
        ([\text{Expr}])
    \end{cases}
\end{align}
$$