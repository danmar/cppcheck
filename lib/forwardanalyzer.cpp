#include "forwardanalyzer.h"
#include "analyzer.h"
#include "astutils.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueptr.h"

#include <algorithm>
#include <functional>

struct ForwardTraversal {
    enum class Progress { Continue, Break, Skip };
    enum class Terminate { None, Bail, Escape, Modified, Inconclusive, Conditional };
    ForwardTraversal(const ValuePtr<Analyzer>& analyzer, const Settings* settings)
        : analyzer(analyzer), settings(settings), actions(Analyzer::Action::None), analyzeOnly(false)
    {}
    ValuePtr<Analyzer> analyzer;
    const Settings* settings;
    Analyzer::Action actions;
    bool analyzeOnly;
    Terminate terminate = Terminate::None;

    Progress Break(Terminate t = Terminate::None)
    {
        if (!analyzeOnly && t != Terminate::None)
            terminate = t;
        return Progress::Break;
    }

    struct Branch {
        Analyzer::Action action = Analyzer::Action::None;
        bool check = false;
        bool escape = false;
        bool escapeUnknown = false;
        const Token* endBlock = nullptr;
        bool isEscape() const {
            return escape || escapeUnknown;
        }
        bool isConclusiveEscape() const {
            return escape && !escapeUnknown;
        }
        bool isModified() const {
            return action.isModified() && !isConclusiveEscape();
        }
        bool isInconclusive() const {
            return action.isInconclusive() && !isConclusiveEscape();
        }
        bool isDead() const {
            return action.isModified() || action.isInconclusive() || isEscape();
        }
    };

    bool stopUpdates() {
        analyzeOnly = true;
        return actions.isModified();
    }

    std::pair<bool, bool> evalCond(const Token* tok) {
        std::vector<int> result = analyzer->evaluate(tok);
        // TODO: We should convert to bool
        bool checkThen = std::any_of(result.begin(), result.end(), [](int x) {
            return x == 1;
        });
        bool checkElse = std::any_of(result.begin(), result.end(), [](int x) {
            return x == 0;
        });
        return std::make_pair(checkThen, checkElse);
    }

    template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*>)>
    Progress traverseTok(T* tok, std::function<Progress(T*)> f, bool traverseUnknown, T** out = nullptr) {
        if (Token::Match(tok, "asm|goto|continue|setjmp|longjmp"))
            return Break();
        else if (Token::Match(tok, "return|throw") || isEscapeFunction(tok, &settings->library)) {
            traverseRecursive(tok->astOperand1(), f, traverseUnknown);
            traverseRecursive(tok->astOperand2(), f, traverseUnknown);
            return Break(Terminate::Escape);
        } else if (isUnevaluated(tok)) {
            if (out)
                *out = tok->link();
            return Progress::Skip;
        } else if (tok->astOperand1() && tok->astOperand2() && Token::Match(tok, "?|&&|%oror%")) {
            if (traverseConditional(tok, f, traverseUnknown) == Progress::Break)
                return Break();
            if (out)
                *out = nextAfterAstRightmostLeaf(tok);
            return Progress::Skip;
            // Skip lambdas
        } else if (T* lambdaEndToken = findLambdaEndToken(tok)) {
            if (checkScope(lambdaEndToken).isModified())
                return Break(Terminate::Bail);
            if (out)
                *out = lambdaEndToken->next();
            // Skip class scope
        } else if (tok->str() == "{" && tok->scope() && tok->scope()->isClassOrStruct()) {
            if (out)
                *out = tok->link();
        } else {
            if (f(tok) == Progress::Break)
                return Break();
        }
        return Progress::Continue;
    }

    template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*>)>
    Progress traverseRecursive(T* tok, std::function<Progress(T*)> f, bool traverseUnknown, unsigned int recursion=0) {
        if (!tok)
            return Progress::Continue;
        if (recursion > 10000)
            return Progress::Skip;
        T* firstOp = tok->astOperand1();
        T* secondOp = tok->astOperand2();
        // Evaluate RHS of assignment before LHS
        if (tok->isAssignmentOp())
            std::swap(firstOp, secondOp);
        if (firstOp && traverseRecursive(firstOp, f, traverseUnknown, recursion+1) == Progress::Break)
            return Break();
        Progress p = tok->isAssignmentOp() ? Progress::Continue : traverseTok(tok, f, traverseUnknown);
        if (p == Progress::Break)
            return Break();
        if (p == Progress::Continue && secondOp && traverseRecursive(secondOp, f, traverseUnknown, recursion+1) == Progress::Break)
            return Break();
        if (tok->isAssignmentOp() && traverseTok(tok, f, traverseUnknown) == Progress::Break)
            return Break();
        return Progress::Continue;
    }

    template<class T, class F, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*>)>
    Progress traverseConditional(T* tok, F f, bool traverseUnknown) {
        if (Token::Match(tok, "?|&&|%oror%") && tok->astOperand1() && tok->astOperand2()) {
            T* condTok = tok->astOperand1();
            T* childTok = tok->astOperand2();
            bool checkThen, checkElse;
            std::tie(checkThen, checkElse) = evalCond(condTok);
            if (!checkThen && !checkElse) {
                // Stop if the value is conditional
                if (!traverseUnknown && analyzer->isConditional() && stopUpdates()) {
                    return Break(Terminate::Conditional);
                }
                checkThen = true;
                checkElse = true;
            }
            if (childTok->str() == ":") {
                if (checkThen && traverseRecursive(childTok->astOperand1(), f, traverseUnknown) == Progress::Break)
                    return Break();
                if (checkElse && traverseRecursive(childTok->astOperand2(), f, traverseUnknown) == Progress::Break)
                    return Break();
            } else {
                if (!checkThen && tok->str() == "&&")
                    return Progress::Continue;
                if (!checkElse && tok->str() == "||")
                    return Progress::Continue;
                if (traverseRecursive(childTok, f, traverseUnknown) == Progress::Break)
                    return Break();
            }
        }
        return Progress::Continue;
    }

    Progress update(Token* tok) {
        Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Forward);
        actions |= action;
        if (!action.isNone() && !analyzeOnly)
            analyzer->update(tok, action, Analyzer::Direction::Forward);
        if (action.isInconclusive() && !analyzer->lowerToInconclusive())
            return Break(Terminate::Inconclusive);
        if (action.isInvalid())
            return Break(Terminate::Modified);
        if (action.isWrite() && !action.isRead())
            // Analysis of this write will continue separately
            return Break(Terminate::Modified);
        return Progress::Continue;
    }

    Progress updateTok(Token* tok, Token** out = nullptr) {
        std::function<Progress(Token*)> f = [this](Token* tok2) {
            return update(tok2);
        };
        return traverseTok(tok, f, false, out);
    }

    Progress updateRecursive(Token* tok) {
        std::function<Progress(Token*)> f = [this](Token* tok2) {
            return update(tok2);
        };
        return traverseRecursive(tok, f, false);
    }

    template <class T>
    T* findRange(T* start, const Token* end, std::function<bool(Analyzer::Action)> pred) {
        for (T* tok = start; tok && tok != end; tok = tok->next()) {
            Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Forward);
            if (pred(action))
                return tok;
        }
        return nullptr;
    }

    Analyzer::Action analyzeRecursive(const Token* start) {
        Analyzer::Action result = Analyzer::Action::None;
        std::function<Progress(const Token*)> f = [&](const Token* tok) {
            result = analyzer->analyze(tok, Analyzer::Direction::Forward);
            if (result.isModified() || result.isInconclusive())
                return Break();
            return Progress::Continue;
        };
        traverseRecursive(start, f, true);
        return result;
    }

    Analyzer::Action analyzeRange(const Token* start, const Token* end) {
        Analyzer::Action result = Analyzer::Action::None;
        for (const Token* tok = start; tok && tok != end; tok = tok->next()) {
            Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Forward);
            if (action.isModified() || action.isInconclusive())
                return action;
            result |= action;
        }
        return result;
    }

    void forkRange(Token* start, const Token* end) {
        ForwardTraversal ft = *this;
        ft.updateRange(start, end);
    }

    std::vector<ForwardTraversal> forkScope(Token* endBlock, bool isModified = false)
    {
        if (analyzer->updateScope(endBlock, isModified)) {
            ForwardTraversal ft = *this;
            ft.analyzer->forkScope(endBlock);
            ft.updateRange(endBlock->link(), endBlock);
            return {ft};
        }
        return std::vector<ForwardTraversal>{};
    }

    static bool hasGoto(const Token* endBlock) {
        return Token::findsimplematch(endBlock->link(), "goto", endBlock);
    }

    bool isEscapeScope(const Token* endBlock, bool& unknown) {
        const Token* ftok = nullptr;
        bool r = isReturnScope(endBlock, &settings->library, &ftok);
        if (!r && ftok)
            unknown = true;
        return r;
    }

    enum class Status {
        None,
        Escaped,
        Modified,
        Inconclusive,
    };

    Analyzer::Action analyzeScope(const Token* endBlock) {
        return analyzeRange(endBlock->link(), endBlock);
    }

    Analyzer::Action checkScope(Token* endBlock) {
        Analyzer::Action a = analyzeScope(endBlock);
        forkScope(endBlock, a.isModified());
        return a;
    }

    Analyzer::Action checkScope(const Token* endBlock) {
        Analyzer::Action a = analyzeScope(endBlock);
        return a;
    }

    void continueUpdateRangeAfterLoop(std::vector<ForwardTraversal>& ftv, Token* start, const Token* endToken)
    {
        for (ForwardTraversal& ft : ftv) {
            // If analysis has terminated normally, then continue analysis
            if (ft.terminate == Terminate::None)
                ft.updateRange(start, endToken);
        }
    }

    Progress updateLoop(const Token* endToken,
                        Token* endBlock,
                        Token* condTok,
                        Token* initTok = nullptr,
                        Token* stepTok = nullptr)
    {
        const bool isDoWhile = precedes(endBlock, condTok);
        Analyzer::Action bodyAnalysis = analyzeScope(endBlock);
        Analyzer::Action allAnalysis = bodyAnalysis;
        if (condTok)
            allAnalysis |= analyzeRecursive(condTok);
        if (initTok)
            allAnalysis |= analyzeRecursive(initTok);
        if (stepTok)
            allAnalysis |= analyzeRecursive(stepTok);
        actions |= allAnalysis;
        if (allAnalysis.isInconclusive()) {
            if (!analyzer->lowerToInconclusive())
                return Break(Terminate::Bail);
        } else if (allAnalysis.isModified()) {
            if (!analyzer->lowerToPossible())
                return Break(Terminate::Bail);
        }
        // Traverse condition after lowering
        if (condTok && (!isDoWhile || (!bodyAnalysis.isModified() && !bodyAnalysis.isIdempotent()))) {
            if (updateRecursive(condTok) == Progress::Break)
                return Break();

            bool checkThen = true;
            bool checkElse = false;
            std::tie(checkThen, checkElse) = evalCond(condTok);
            // condition is false, we don't enter the loop
            if (checkElse)
                return Progress::Continue;
        }

        std::vector<ForwardTraversal> ftv = forkScope(endBlock, allAnalysis.isModified());
        if (bodyAnalysis.isModified()) {
            Token* writeTok = findRange(endBlock->link(), endBlock, std::mem_fn(&Analyzer::Action::isModified));
            const Token* nextStatement = Token::findmatch(writeTok, ";|}", endBlock);
            if (!Token::Match(nextStatement, ";|} break ;")) {
                continueUpdateRangeAfterLoop(ftv, endBlock, endToken);
                return Break(Terminate::Bail);
            }
        } else {
            if (stepTok && updateRecursive(stepTok) == Progress::Break) {
                continueUpdateRangeAfterLoop(ftv, endBlock, endToken);
                return Break(Terminate::Bail);
            }
        }
        // TODO: Should we traverse the body?
        // updateRange(endBlock->link(), endBlock);
        return Progress::Continue;
    }

    Progress updateRange(Token* start, const Token* end) {
        for (Token* tok = start; tok && tok != end; tok = tok->next()) {
            Token* next = nullptr;

            if (tok->link()) {
                // Skip casts..
                if (tok->str() == "(" && !tok->astOperand2() && tok->isCast()) {
                    tok = tok->link();
                    continue;
                }
                // Skip template arguments..
                if (tok->str() == "<") {
                    tok = tok->link();
                    continue;
                }
            }

            // Evaluate RHS of assignment before LHS
            if (Token* assignTok = assignExpr(tok)) {
                if (updateRecursive(assignTok) == Progress::Break)
                    return Break();
                tok = nextAfterAstRightmostLeaf(assignTok);
                if (!tok)
                    return Break();
            } else if (tok->str() ==  "break") {
                const Token *scopeEndToken = findNextTokenFromBreak(tok);
                if (!scopeEndToken)
                    return Break();
                tok = skipTo(tok, scopeEndToken, end);
                if (!analyzer->lowerToPossible())
                    return Break(Terminate::Bail);
                // TODO: Don't break, instead move to the outer scope
                if (!tok)
                    return Break();
            } else if (Token::Match(tok, "%name% :") || tok->str() == "case") {
                if (!analyzer->lowerToPossible())
                    return Break(Terminate::Bail);
            } else if (tok->link() && tok->str() == "}") {
                const Scope* scope = tok->scope();
                if (!scope)
                    return Break();
                if (Token::Match(tok->link()->previous(), ")|else {")) {
                    const Token* tok2 = tok->link()->previous();
                    const bool inElse = Token::simpleMatch(tok2, "else {");
                    const bool inLoop = inElse ? false : Token::Match(tok2->link()->previous(), "while|for (");
                    Token* condTok = getCondTokFromEnd(tok);
                    if (!condTok)
                        return Break();
                    if (!condTok->hasKnownIntValue() || inLoop) {
                        if (!analyzer->lowerToPossible())
                            return Break(Terminate::Bail);
                    } else if (condTok->values().front().intvalue == inElse) {
                        return Break();
                    }
                    // Handle for loop
                    Token* stepTok = getStepTokFromEnd(tok);
                    bool checkThen, checkElse;
                    std::tie(checkThen, checkElse) = evalCond(condTok);
                    if (stepTok && !checkElse) {
                        if (updateRecursive(stepTok) == Progress::Break)
                            return Break();
                        if (updateRecursive(condTok) == Progress::Break)
                            return Break();
                    }
                    analyzer->assume(condTok, !inElse, tok);
                    if (Token::simpleMatch(tok, "} else {"))
                        tok = tok->linkAt(2);
                } else if (scope->type == Scope::eTry) {
                    if (!analyzer->lowerToPossible())
                        return Break(Terminate::Bail);
                } else if (scope->type == Scope::eLambda) {
                    return Break();
                } else if (scope->type == Scope::eDo && Token::simpleMatch(tok, "} while (")) {
                    if (updateLoop(end, tok, tok->tokAt(2)->astOperand2()) == Progress::Break)
                        return Break();
                    tok = tok->linkAt(2);
                } else if (Token::simpleMatch(tok->next(), "else {")) {
                    tok = tok->linkAt(2);
                }
            } else if (tok->isControlFlowKeyword() && Token::Match(tok, "if|while|for (") && Token::simpleMatch(tok->next()->link(), ") {")) {
                Token* endCond = tok->next()->link();
                Token* endBlock = endCond->next()->link();
                Token* condTok = getCondTok(tok);
                Token* initTok = getInitTok(tok);
                if (initTok && updateRecursive(initTok) == Progress::Break)
                    return Break();
                if (Token::Match(tok, "for|while (")) {
                    // For-range loop
                    if (Token::simpleMatch(condTok, ":")) {
                        Token* conTok = condTok->astOperand2();
                        if (conTok && updateRecursive(conTok) == Progress::Break)
                            return Break();
                        if (updateLoop(end, endBlock, condTok) == Progress::Break)
                            return Break();
                    } else {
                        Token* stepTok = getStepTok(tok);
                        if (updateLoop(end, endBlock, condTok, initTok, stepTok) == Progress::Break)
                            return Break();
                    }
                    tok = endBlock;
                } else {
                    // Traverse condition
                    if (updateRecursive(condTok) == Progress::Break)
                        return Break();
                    Branch thenBranch{};
                    Branch elseBranch{};
                    // Check if condition is true or false
                    std::tie(thenBranch.check, elseBranch.check) = evalCond(condTok);
                    bool hasElse = Token::simpleMatch(endBlock, "} else {");
                    bool bail = false;

                    // Traverse then block
                    thenBranch.escape = isEscapeScope(endBlock, thenBranch.escapeUnknown);
                    if (thenBranch.check) {
                        if (updateRange(endCond->next(), endBlock) == Progress::Break)
                            return Break();
                    } else if (!elseBranch.check) {
                        thenBranch.action = checkScope(endBlock);
                        if (hasGoto(endBlock))
                            bail = true;
                    }
                    // Traverse else block
                    if (hasElse) {
                        elseBranch.escape = isEscapeScope(endBlock->linkAt(2), elseBranch.escapeUnknown);
                        if (elseBranch.check) {
                            Progress result = updateRange(endBlock->tokAt(2), endBlock->linkAt(2));
                            if (result == Progress::Break)
                                return Break();
                        } else if (!thenBranch.check) {
                            elseBranch.action = checkScope(endBlock->linkAt(2));
                            if (hasGoto(endBlock))
                                bail = true;
                        }
                        tok = endBlock->linkAt(2);
                    } else {
                        tok = endBlock;
                    }
                    actions |= (thenBranch.action | elseBranch.action);
                    if (bail)
                        return Break();
                    if (thenBranch.isDead() && elseBranch.isDead()) {
                        if (thenBranch.isModified() && elseBranch.isModified())
                            return Break(Terminate::Modified);
                        if (thenBranch.isConclusiveEscape() && elseBranch.isConclusiveEscape())
                            return Break(Terminate::Escape);
                        return Break(Terminate::Bail);
                    }
                    // Conditional return
                    if (thenBranch.isEscape() && !hasElse) {
                        if (!thenBranch.isConclusiveEscape()) {
                            if (!analyzer->lowerToInconclusive())
                                return Break(Terminate::Bail);
                        } else if (thenBranch.check) {
                            return Break();
                        } else {
                            if (analyzer->isConditional() && stopUpdates())
                                return Break(Terminate::Conditional);
                            analyzer->assume(condTok, false);
                        }
                    }
                    if (thenBranch.isInconclusive() || elseBranch.isInconclusive()) {
                        if (!analyzer->lowerToInconclusive())
                            return Break(Terminate::Bail);
                    } else if (thenBranch.isModified() || elseBranch.isModified()) {
                        if (!hasElse && analyzer->isConditional() && stopUpdates())
                            return Break(Terminate::Conditional);
                        if (!analyzer->lowerToPossible())
                            return Break(Terminate::Bail);
                        analyzer->assume(condTok, elseBranch.isModified());
                    }
                }
            } else if (Token::simpleMatch(tok, "try {")) {
                Token* endBlock = tok->next()->link();
                Analyzer::Action a = analyzeScope(endBlock);
                if (updateRange(tok->next(), endBlock) == Progress::Break)
                    return Break();
                if (a.isModified())
                    analyzer->lowerToPossible();
                tok = endBlock;
            } else if (Token::simpleMatch(tok, "do {")) {
                Token* endBlock = tok->next()->link();
                Token* condTok = Token::simpleMatch(endBlock, "} while (") ? endBlock->tokAt(2)->astOperand2() : nullptr;
                if (updateLoop(end, endBlock, condTok) == Progress::Break)
                    return Break();
                if (condTok)
                    tok = endBlock->linkAt(2)->next();
                else
                    tok = endBlock;
            } else if (Token::Match(tok, "assert|ASSERT (")) {
                const Token* condTok = tok->next()->astOperand2();
                bool checkThen, checkElse;
                std::tie(checkThen, checkElse) = evalCond(condTok);
                if (checkElse)
                    return Break();
                if (!checkThen)
                    analyzer->assume(condTok, true, tok);
            } else if (Token::simpleMatch(tok, "switch (")) {
                if (updateRecursive(tok->next()->astOperand2()) == Progress::Break)
                    return Break();
                return Break();
            } else {
                if (updateTok(tok, &next) == Progress::Break)
                    return Break();
                if (next) {
                    if (precedes(next, end))
                        tok = next->previous();
                    else
                        return Break();
                }
            }
            // Prevent infinite recursion
            if (tok->next() == start)
                break;
        }
        return Progress::Continue;
    }

    static bool isUnevaluated(const Token* tok) {
        if (Token::Match(tok->previous(), "sizeof|decltype ("))
            return true;
        return false;
    }

    static Token* assignExpr(Token* tok) {
        while (tok->astParent() && astIsLHS(tok)) {
            if (tok->astParent()->isAssignmentOp())
                return tok->astParent();
            tok = tok->astParent();
        }
        return nullptr;
    }

    static Token* skipTo(Token* tok, const Token* dest, const Token* end = nullptr) {
        if (end && dest->index() > end->index())
            return nullptr;
        int i = dest->index() - tok->index();
        if (i > 0)
            return tok->tokAt(dest->index() - tok->index());
        return nullptr;
    }

    static bool isConditional(const Token* tok) {
        const Token* parent = tok->astParent();
        while (parent && !Token::Match(parent, "%oror%|&&|:")) {
            tok = parent;
            parent = parent->astParent();
        }
        return parent && (parent->str() == ":" || parent->astOperand2() == tok);
    }

    static Token* getInitTok(Token* tok) {
        if (!tok)
            return nullptr;
        if (Token::Match(tok, "%name% ("))
            return getInitTok(tok->next());
        if (tok->str() !=  "(")
            return nullptr;
        if (!Token::simpleMatch(tok->astOperand2(), ";"))
            return nullptr;
        if (Token::simpleMatch(tok->astOperand2()->astOperand1(), ";"))
            return nullptr;
        return tok->astOperand2()->astOperand1();
    }

    static Token* getStepTok(Token* tok) {
        if (!tok)
            return nullptr;
        if (Token::Match(tok, "%name% ("))
            return getStepTok(tok->next());
        if (tok->str() != "(")
            return nullptr;
        if (!Token::simpleMatch(tok->astOperand2(), ";"))
            return nullptr;
        if (!Token::simpleMatch(tok->astOperand2()->astOperand2(), ";"))
            return nullptr;
        return tok->astOperand2()->astOperand2()->astOperand2();
    }

    static Token* getStepTokFromEnd(Token* tok) {
        if (!Token::simpleMatch(tok, "}"))
            return nullptr;
        Token* end = tok->link()->previous();
        if (!Token::simpleMatch(end, ")"))
            return nullptr;
        return getStepTok(end->link());
    }

};

Analyzer::Action valueFlowGenericForward(Token* start,
        const Token* end,
        const ValuePtr<Analyzer>& a,
        const Settings* settings)
{
    ForwardTraversal ft{a, settings};
    ft.updateRange(start, end);
    return ft.actions;
}

Analyzer::Action valueFlowGenericForward(Token* start, const ValuePtr<Analyzer>& a, const Settings* settings)
{
    ForwardTraversal ft{a, settings};
    ft.updateRecursive(start);
    return ft.actions;
}
