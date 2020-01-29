#include "forwardanalyzer.h"
#include "astutils.h"
#include "settings.h"
#include "symboldatabase.h"

struct ForwardTraversal
{
    enum class Progress {
        Continue,
        Break
    };
    ValuePtr<ForwardAnalyzer> analyzer;
    const Settings* settings;

    std::pair<bool, bool> evalCond(const Token* tok)
    {
        std::vector<int> result = analyzer->Evaluate(tok);
        bool checkThen = std::any_of(result.begin(), result.end(), [](int x) { return x; });
        bool checkElse = std::any_of(result.begin(), result.end(), [](int x) { return !x; });
        return std::make_pair(checkThen, checkElse);
    }

    Progress update(Token* tok) {
        ForwardAnalyzer::Action action = analyzer->Analyze(tok);
        if (!action.isNone())
            analyzer->Update(tok, action);
        if (action.isInvalid())
            return Progress::Break;
        return Progress::Continue;
    }

    Progress updateRecursive(Token* tok) {
        if (!tok)
            return Progress::Continue;
        // Check for lambda expression
        if (Token *lambdaEndToken = findLambdaEndToken(tok)) {
            if (checkScope(lambdaEndToken).isModified())
                return Progress::Break;
            else
                return Progress::Continue;
        }
        if (tok->astOperand1() && updateRecursive(tok->astOperand1()) == Progress::Break)
            return Progress::Break;
        if (isUnevaluated(tok))
            return Progress::Continue;
        // Check for conditional expression
        if (Token::Match(tok, "?|&&|%oror%")) {
            if (updateConditional(tok) == Progress::Break)
                return Progress::Break;
        } else {
            if (update(tok) == Progress::Break)
                return Progress::Break;
            if (tok->astOperand2() && updateRecursive(tok->astOperand2()) == Progress::Break)
                return Progress::Break;
        }
        return Progress::Continue;
    }

    Progress updateConditional(Token* tok) {
        if (Token::Match(tok, "?|&&|%oror%")) {
            Token * condTok = tok->astOperand1();
            if (updateRecursive(condTok) == Progress::Break)
                return Progress::Break;
            Token * childTok = tok->astOperand2();
            bool checkThen, checkElse;
            std::tie(checkThen, checkElse) = evalCond(condTok);
            if (!checkThen && !checkElse) {
                // Stop if the value is conditional
                if (analyzer->IsConditional())
                    return Progress::Break;
                checkThen = true;
                checkElse = true;
            }
            if (Token::simpleMatch(childTok, ":")) {
                if (checkThen && updateRecursive(childTok->astOperand1()) == Progress::Break)
                    return Progress::Break;
                if (checkElse && updateRecursive(childTok->astOperand2()) == Progress::Break)
                    return Progress::Break;
            } else {
                if (!checkThen && Token::simpleMatch(tok, "&&"))
                    return Progress::Continue;
                if (!checkElse && Token::simpleMatch(tok, "||"))
                    return Progress::Continue;
                if (updateRecursive(childTok) == Progress::Break)
                    return Progress::Break;
            }
        }
        return Progress::Continue;
    }

    template<class T, class Predicate>
    T* findRange(T* start, const Token* end, Predicate pred) {
        for (T *tok = start; tok && tok != end; tok = tok->next()) {
            ForwardAnalyzer::Action action = analyzer->Analyze(tok);
            if (pred(action))
                return tok;
        }
        return nullptr;
    }

    template<class T>
    T* findActionRange(T* start, const Token* end, ForwardAnalyzer::Action action) {
        return findRange(start, end, [&](ForwardAnalyzer::Action a) {
            return a == action;
        });
    }

    ForwardAnalyzer::Action analyzeRange(const Token* start, const Token* end) {
        ForwardAnalyzer::Action result = ForwardAnalyzer::Action::None;
        for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
            ForwardAnalyzer::Action action = analyzer->Analyze(tok);
            if (action.isModified() || action.isInconclusive())
                return action;
            result = action;
        }
        return result;
    }

    void forkScope(const Token* endBlock, bool isModified=false)
    {
        if (analyzer->UpdateScope(endBlock, isModified)) {
            ForwardTraversal ft = *this;
            ft.updateRange(endBlock->link(), endBlock);
        }
    }

    static bool hasGoto(const Token* endBlock) {
        return Token::findsimplematch(endBlock->link(), "goto", endBlock);
    }

    bool isEscapeScope(const Token* endBlock) {
        return isReturnScope(endBlock, &settings->library);
    }

    enum class Status {
        None,
        Escaped,
        Modified,
        Inconclusive,
    };

    ForwardAnalyzer::Action analyzeScope(const Token* endBlock) {
        return analyzeRange(endBlock->link(), endBlock);       
    }
        
    ForwardAnalyzer::Action checkScope(const Token* endBlock) {
        ForwardAnalyzer::Action a = analyzeScope(endBlock);
        forkScope(endBlock, a.isModified()); 
        return a;
    }

    Progress updateLoop(Token* endBlock, Token* condTok) {
        ForwardAnalyzer::Action a = analyzeScope(endBlock);
        if (a.isInconclusive()) {
            if (!analyzer->LowerToInconclusive())
                return Progress::Break;
        } else if (a.isModified()) {
            if (!analyzer->LowerToPossible())
                return Progress::Break;
        }
        // Traverse condition after lowering
        if (condTok && updateRecursive(condTok) == Progress::Break)
            return Progress::Break;
        forkScope(endBlock, a.isModified()); 
        if (a.isModified()) {
            Token * writeTok = findRange(endBlock->link(), endBlock, std::mem_fn(&ForwardAnalyzer::Action::isModified));
            const Token * nextStatement = Token::findmatch(writeTok, ";|}", endBlock);
            if (!Token::Match(nextStatement, ";|} break ;"))
                return Progress::Break;
        }
        // TODO: Shoule we traverse the body?
        // updateRange(endBlock->link(), endBlock);
        return Progress::Continue;
    }

    Progress updateRange(Token* start, const Token* end) {
        for (Token *tok = start; tok && tok != end; tok = tok->next()) {
            if (Token::Match(tok, "asm|goto|continue|case"))
                return Progress::Break;
            if (Token::Match(tok, "return|throw") || isEscapeFunction(tok, &settings->library)) {
                updateRecursive(tok);
                return Progress::Break;
            }

            // Evaluate RHS of assignment before LHS
            if (Token* assignTok = assignExpr(tok)) {
                if (updateRecursive(assignTok->astOperand2()) == Progress::Break)
                    return Progress::Break;
                if (updateRecursive(assignTok->astOperand1()) == Progress::Break)
                    return Progress::Break;
                if (update(assignTok) == Progress::Break)
                    return Progress::Break;
                tok = nextAfterAstRightmostLeaf(assignTok);
                if (!tok)
                    return Progress::Break;
            } else if (Token::simpleMatch(tok, "break")) {
                const Scope* scope = findBreakScope(tok->scope());
                if (!scope)
                    return Progress::Break;
                tok = skipTo(tok, scope->bodyEnd, end);
                if (!analyzer->LowerToPossible())
                    return Progress::Break;
                // TODO: Dont break, instead move to the outer scope
                if (!tok)
                    return Progress::Break;
            } else if (Token::Match(tok, "%name% :")) {
                if (!analyzer->LowerToPossible())
                    return Progress::Break;
            } else if (Token::simpleMatch(tok, "}") && Token::simpleMatch(tok->link()->previous(), ") {") && Token::Match(tok->link()->linkAt(-1)->previous(), "if|while|for (")) {
                const Token * blockStart = tok->link()->linkAt(-1)->previous();
                const Token * condTok = getCondTok(blockStart);
                if (!condTok)
                    return Progress::Break;
                if (!condTok->hasKnownIntValue()) {
                    if (!analyzer->LowerToPossible())
                        return Progress::Break;
                } else if (condTok->values().front().intvalue == 0)
                    return Progress::Break;
                analyzer->Assume(condTok, true);
                // std::vector<int> result = analyzer->Evaluate(tok);
                // if (result.empty())
                //     return Progress::Break;

                // info.errorPath.emplace_back(condTok, "Assuming condition is true.");
                // Traverse a loop a second time
                if (Token::Match(blockStart, "for|while (")) {
                    return Progress::Break;
#if 0
                    const Token* endCond = blockStart->linkAt(1);
                    bool traverseLoop = true;
                    // Only traverse simple for loops
                    if (Token::simpleMatch(blockStart, "for") && !Token::Match(endCond->tokAt(-3), "; ++|--|%var% %var%|++|-- ) {"))
                        traverseLoop = false;
                    // Traverse loop a second time
                    if (traverseLoop) {
                        // Traverse condition
                        if (updateRecursive(condTok) == Progress::Break)
                            return Progress::Break;
                        // TODO: Should we traverse the body: updateRange(tok->link(), tok)?
                    }
#endif
                }
                if (Token::simpleMatch(tok, "} else {"))
                    tok = tok->linkAt(2);
            } else if (Token::Match(tok, "if|while|for (") && Token::simpleMatch(tok->next()->link(), ") {")) {
                Token * endCond = tok->next()->link();
                Token * endBlock = endCond->next()->link();
                Token * condTok = getCondTok(tok);
                if (!condTok)
                    return Progress::Break;

                if (Token::Match(tok, "for|while (")) {
                    if (updateLoop(endBlock, condTok) == Progress::Break)
                        return Progress::Break;
                    tok = endBlock;
                } else {
                    // Traverse condition
                    if (updateRecursive(condTok) == Progress::Break)
                        return Progress::Break;
                    // Check if condition is true or false
                    bool checkThen, checkElse;
                    std::tie(checkThen, checkElse) = evalCond(condTok);
                    ForwardAnalyzer::Action thenAction = ForwardAnalyzer::Action::None;
                    ForwardAnalyzer::Action elseAction = ForwardAnalyzer::Action::None;
                    bool hasElse = false;
                    bool bail = false;

                    // Traverse then block
                    bool returnThen = isEscapeScope(endBlock);
                    bool returnElse = false;
                    if (checkThen) {
                        if (updateRange(endCond->next(), endBlock) == Progress::Break)
                            return Progress::Break;
                    } else if (!checkElse) {
                        thenAction = checkScope(endBlock);
                        if (hasGoto(endBlock))
                            bail = true;
                    }
                    // Traverse else block
                    if (Token::simpleMatch(endBlock, "} else {")) {
                        hasElse = true;
                        returnElse = isEscapeScope(endBlock->linkAt(2));
                        if (checkElse) {
                            Progress result = updateRange(endBlock->tokAt(2), endBlock->linkAt(2));
                            if (result == Progress::Break)
                                return Progress::Break;
                        } else if (!checkThen) {
                            elseAction = checkScope(endBlock->linkAt(2));
                            if (hasGoto(endBlock))
                                bail = true;
                        }
                        tok = endBlock->linkAt(2);
                    } else {
                        tok = endBlock;
                    }
                    if (bail)
                        return Progress::Break;
                    if (returnThen && returnElse)
                        return Progress::Break;
                    else if (thenAction.isModified() && elseAction.isModified())
                        return Progress::Break;
                    else if ((returnThen || returnElse) && (thenAction.isModified() || elseAction.isModified()))
                        return Progress::Break;
                    // Conditional return
                    if (returnThen && !hasElse && !checkThen) {
                        if (analyzer->IsConditional())
                            return Progress::Break;
                        analyzer->Assume(condTok, false);
                    }
                    if (thenAction.isInconclusive() || elseAction.isInconclusive()) {
                        if (!analyzer->LowerToInconclusive())
                            return Progress::Break;
                    }
                    else if (thenAction.isModified() || elseAction.isModified()) {
                        if (!analyzer->LowerToPossible())
                            return Progress::Break;
                        analyzer->Assume(condTok, elseAction.isModified());
                    }
                }
            } else if (Token::simpleMatch(tok, "} else {")) {
                tok = tok->linkAt(2);
            } else if (Token::simpleMatch(tok, "do {")) {
                Token * endBlock = tok->next()->link();
                if (updateLoop(endBlock, nullptr) == Progress::Break)
                    return Progress::Break;
                tok = endBlock;
            } else if (Token::Match(tok, "assert|ASSERT (")) {
                const Token* condTok = tok->next()->astOperand2();
                bool checkThen, checkElse;
                std::tie(checkThen, checkElse) = evalCond(condTok);
                if (checkElse)
                    return Progress::Break;
                if (!checkThen)
                    analyzer->Assume(condTok, true);
            } else if (Token::Match(tok, "?|&&|%oror%")) {
                if (updateConditional(tok) == Progress::Break)
                    return Progress::Break;
                tok = nextAfterAstRightmostLeaf(tok);
            } else if (Token::simpleMatch(tok, "switch (")) {
                if (updateRecursive(tok->next()->astOperand2()) == Progress::Break)
                    return Progress::Break;
                return Progress::Break;
            // Skip lambdas
            } else if (Token *lambdaEndToken = findLambdaEndToken(tok)) {
                if (checkScope(lambdaEndToken).isModified())
                    return Progress::Break;
                tok = lambdaEndToken;
            // Skip unevaluated context
            } else if (isUnevaluated(tok)) {
                tok = tok->link();
            } else {
                if (update(tok) == Progress::Break)
                    return Progress::Break;
            }
            // Prevent infinite recursion
            if (tok->next() == start)
                break;
        }
        return Progress::Continue;
    }

    static bool isUnevaluated(const Token* tok)
    {
        if (Token::Match(tok->previous(), "sizeof|decltype ("))
            return true;
        return false;
    }

    static Token* assignExpr(Token* tok)
    {
        while(tok->astParent() && astIsLHS(tok)) {
            if (Token::Match(tok->astParent(), "%assign%"))
                return tok->astParent();
            tok = tok->astParent();
        }
        return nullptr;
    }

    template<class T>
    static T* getCondTok(T* tok)
    {
        if (!tok)
            return nullptr;
        if (Token::simpleMatch(tok, "("))
            return getCondTok(tok->previous());
        if (Token::simpleMatch(tok, "for") && Token::simpleMatch(tok->next()->astOperand2(), ";") && tok->next()->astOperand2()->astOperand2())
            return tok->next()->astOperand2()->astOperand2()->astOperand1();
        if (Token::simpleMatch(tok->next()->astOperand2(), ";"))
            return tok->next()->astOperand2()->astOperand1();
        return tok->next()->astOperand2();
    }

    static const Scope* findBreakScope(const Scope * scope)
    {
        while(scope && scope->type != Scope::eWhile && scope->type != Scope::eFor && scope->type != Scope::eSwitch)
            scope = scope->nestedIn;
        return scope;
    }

    static Token* skipTo(Token* tok, const Token* dest, const Token* end = nullptr)
    {
        if (end && dest->index() > end->index())
            return nullptr;
        int i = dest->index() - tok->index();
        if (i > 0)
            return tok->tokAt(dest->index() - tok->index());
        return nullptr;
    }

    static bool isConditional(const Token* tok) 
    {
        const Token *parent = tok->astParent();
        while (parent && !Token::Match(parent, "%oror%|&&|:")) {
            tok = parent;
            parent = parent->astParent();
        }
        return parent && (parent->str() == ":" || parent->astOperand2() == tok);
    }
};

void valueFlowGenericForward(Token* start, const Token* end, const ValuePtr<ForwardAnalyzer>& fa, const Settings* settings)
{
    ForwardTraversal ft{fa, settings};
    ft.updateRange(start, end);
}
