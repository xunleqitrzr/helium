$$
\begin{align}
    [\text{prog}] &\to [\text{stmt}]^*
    \\
    [\text{stmt}] &\to 
    \begin {cases}
        exit([\text{expr}]); \\
        var\space \text{identifier} = [\text{expr}];
    \end {cases}
    \\
    [\text{expr}] &\to \text{int\_lit}
\end{align}
$$