#include "forwardanalyzer.h"
#include "astutils.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueptr.h"

#include <algorithm>
#include <functional>

struct ForwardTraversal {
    enum class Progress { Continue, Break, Skip };
    ValuePtr<ForwardAnalyzer> analyzer;
    const Settings* settings;

    std::pair<bool, bool> evalCond(const Token* tok) {
        std::vector<int> result = analyzer->evaluate(tok);
        bool checkThen = std::any_of(result.begin(), result.end(), [](int x) {
            return x;
        });
        bool checkElse = std::any_of(result.begin(), result.end(), [](int x) {
            return !x;
        });
        return std::make_pair(checkThen, checkElse);
    }

    template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*>)>
    Progress traverseTok(T* tok, std::function<Progress(T*)> f, bool traverseUnknown, T** out = nullptr) {
        if (Token::Match(tok, "asm|goto|continue|setjmp|longjmp"))
            return Progress::Break;
        else if (Token::Match(tok, "return|throw") || isEscapeFunction(tok, &settings->library)) {
            traverseRecursive(tok->astOperand1(), f, traverseUnknown);
            traverseRecursive(tok->astOperand2(), f, traverseUnknown);
            return Progress::Break;
        } else if (isUnevaluated(tok)) {
            if (out)
                *out = tok->link();
            return Progress::Skip;
        } else if (tok->astOperand1() && tok->astOperand2() && Token::Match(tok, "?|&&|%oror%")) {
            if (traverseConditional(tok, f, traverseUnknown) == Progress::Break)
                return Progress::Break;
            if (out)
                *out = nextAfterAstRightmostLeaf(tok);
            return Progress::Skip;
            // Skip lambdas
        } else if (T* lambdaEndToken = findLambdaEndToken(tok)) {
            if (checkScope(lambdaEndToken).isModified())
                return Progress::Break;
            if (out)
                *out = lambdaEndToken->next();
            // Skip class scope
        } else if (tok->str() == "{" && tok->scope() && tok->scope()->isClassOrStruct()) {
            if (out)
                *out = tok->link();
        } else {
            if (f(tok) == Progress::Break)
                return Progress::Break;
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
            return Progress::Break;
        Progress p = tok->isAssignmentOp() ? Progress::Continue : traverseTok(tok, f, traverseUnknown);
        if (p == Progress::Break)
            return Progress::Break;
        if (p == Progress::Continue && secondOp && traverseRecursive(secondOp, f, traverseUnknown, recursion+1) == Progress::Break)
            return Progress::Break;
        if (tok->isAssignmentOp() && traverseTok(tok, f, traverseUnknown) == Progress::Break)
            return Progress::Break;
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
                if (!traverseUnknown && analyzer->isConditional())
                    return Progress::Break;
                checkThen = true;
                checkElse = true;
            }
            if (childTok->str() == ":") {
                if (checkThen && traverseRecursive(childTok->astOperand1(), f, traverseUnknown) == Progress::Break)
                    return Progress::Break;
                if (checkElse && traverseRecursive(childTok->astOperand2(), f, traverseUnknown) == Progress::Break)
                    return Progress::Break;
            } else {
                if (!checkThen && tok->str() == "&&")
                    return Progress::Continue;
                if (!checkElse && tok->str() == "||")
                    return Progress::Continue;
                if (traverseRecursive(childTok, f, traverseUnknown) == Progress::Break)
                    return Progress::Break;
            }
        }
        return Progress::Continue;
    }

    Progress update(Token* tok) {
        ForwardAnalyzer::Action action = analyzer->analyze(tok);
        if (!action.isNone())
            analyzer->update(tok, action);
        if (action.isInconclusive() && !analyzer->lowerToInconclusive())
            return Progress::Break;
        if (action.isInvalid())
            return Progress::Break;
        if (action.isWrite() && !action.isRead())
            // Analysis of this write will continue separately
            return Progress::Break;
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
    T* findRange(T* start, const Token* end, std::function<bool(ForwardAnalyzer::Action)> pred) {
        for (T* tok = start; tok && tok != end; tok = tok->next()) {
            ForwardAnalyzer::Action action = analyzer->analyze(tok);
            if (pred(action))
                return tok;
        }
        return nullptr;
    }

    ForwardAnalyzer::Action analyzeRecursive(const Token* start) {
        ForwardAnalyzer::Action result = ForwardAnalyzer::Action::None;
        std::function<Progress(const Token *)> f = [&](const Token* tok) {
            result = analyzer->analyze(tok);
            if (result.isModified() || result.isInconclusive())
                return Progress::Break;
            return Progress::Continue;
        };
        traverseRecursive(start, f, true);
        return result;
    }

    ForwardAnalyzer::Action analyzeRange(const Token* start, const Token* end) {
        ForwardAnalyzer::Action result = ForwardAnalyzer::Action::None;
        for (const Token* tok = start; tok && tok != end; tok = tok->next()) {
            ForwardAnalyzer::Action action = analyzer->analyze(tok);
            if (action.isModified() || action.isInconclusive())
                return action;
            result = action;
        }
        return result;
    }

    void forkScope(Token* endBlock, bool isModified = false) {
        if (analyzer->updateScope(endBlock, isModified)) {
            ForwardTraversal ft = *this;
            ft.updateRange(endBlock->link(), endBlock);
        }
    }

    static bool hasGoto(const Token* endBlock) {
        return Token::findsimplematch(endBlock->link(), "goto", endBlock);
    }

    bool isEscapeScope(const Token* endBlock, bool unknown = false) {
        const Token* ftok = nullptr;
        bool r = isReturnScope(endBlock, &settings->library, &ftok);
        if (!r && ftok)
            return unknown;
        return r;
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

    ForwardAnalyzer::Action checkScope(Token* endBlock) {
        ForwardAnalyzer::Action a = analyzeScope(endBlock);
        forkScope(endBlock, a.isModified());
        return a;
    }

    ForwardAnalyzer::Action checkScope(const Token* endBlock) {
        ForwardAnalyzer::Action a = analyzeScope(endBlock);
        return a;
    }

    Progress updateLoop(Token* endBlock, Token* condTok, Token* initTok = nullptr, Token* stepTok = nullptr) {
        const bool isDoWhile = precedes(endBlock, condTok);
        ForwardAnalyzer::Action bodyAnalysis = analyzeScope(endBlock);
        ForwardAnalyzer::Action allAnalysis = bodyAnalysis;
        if (condTok)
            allAnalysis |= analyzeRecursive(condTok);
        if (initTok)
            allAnalysis |= analyzeRecursive(initTok);
        if (stepTok)
            allAnalysis |= analyzeRecursive(stepTok);
        if (allAnalysis.isInconclusive()) {
            if (!analyzer->lowerToInconclusive())
                return Progress::Break;
        } else if (allAnalysis.isModified()) {
            if (!analyzer->lowerToPossible())
                return Progress::Break;
        }
        // Traverse condition after lowering
        if (condTok && (!isDoWhile || !bodyAnalysis.isModified())) {
            if (updateRecursive(condTok) == Progress::Break)
                return Progress::Break;

            bool checkThen, checkElse;
            std::tie(checkThen, checkElse) = evalCond(condTok);
            if (checkElse)
                // condition is false, we don't enter the loop
                return Progress::Break;
        }

        forkScope(endBlock, allAnalysis.isModified());
        if (bodyAnalysis.isModified()) {
            Token* writeTok = findRange(endBlock->link(), endBlock, std::mem_fn(&ForwardAnalyzer::Action::isModified));
            const Token* nextStatement = Token::findmatch(writeTok, ";|}", endBlock);
            if (!Token::Match(nextStatement, ";|} break ;"))
                return Progress::Break;
        } else {
            if (stepTok && updateRecursive(stepTok) == Progress::Break)
                return Progress::Break;
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
                    return Progress::Break;
                tok = nextAfterAstRightmostLeaf(assignTok);
                if (!tok)
                    return Progress::Break;
            } else if (tok->str() ==  "break") {
                const Scope* scope = findBreakScope(tok->scope());
                if (!scope)
                    return Progress::Break;
                tok = skipTo(tok, scope->bodyEnd, end);
                if (!analyzer->lowerToPossible())
                    return Progress::Break;
                // TODO: Don't break, instead move to the outer scope
                if (!tok)
                    return Progress::Break;
            } else if (Token::Match(tok, "%name% :") || tok->str() == "case") {
                if (!analyzer->lowerToPossible())
                    return Progress::Break;
            } else if (tok->link() && tok->str() == "}") {
                const Scope* scope = tok->scope();
                if (!scope)
                    return Progress::Break;
                if (Token::Match(tok->link()->previous(), ")|else {")) {
                    const bool inElse = Token::simpleMatch(tok->link()->previous(), "else {");
                    Token* condTok = getCondTokFromEnd(tok);
                    if (!condTok)
                        return Progress::Break;
                    if (!condTok->hasKnownIntValue()) {
                        if (!analyzer->lowerToPossible())
                            return Progress::Break;
                    } else if (condTok->values().front().intvalue == inElse) {
                        return Progress::Break;
                    }
                    // Handle for loop
                    Token* stepTok = getStepTokFromEnd(tok);
                    bool checkThen, checkElse;
                    std::tie(checkThen, checkElse) = evalCond(condTok);
                    if (stepTok && !checkElse) {
                        if (updateRecursive(stepTok) == Progress::Break)
                            return Progress::Break;
                        if (updateRecursive(condTok) == Progress::Break)
                            return Progress::Break;
                    }
                    analyzer->assume(condTok, !inElse, tok);
                    if (Token::simpleMatch(tok, "} else {"))
                        tok = tok->linkAt(2);
                } else if (scope->type == Scope::eTry) {
                    if (!analyzer->lowerToPossible())
                        return Progress::Break;
                } else if (scope->type == Scope::eLambda) {
                    return Progress::Break;
                } else if (scope->type == Scope::eDo && Token::simpleMatch(tok, "} while (")) {
                    if (updateLoop(tok, tok->tokAt(2)->astOperand2()) == Progress::Break)
                        return Progress::Break;
                    tok = tok->linkAt(2);
                } else if (Token::simpleMatch(tok->next(), "else {")) {
                    tok = tok->linkAt(2);
                }
            } else if (tok->isControlFlowKeyword() && Token::Match(tok, "if|while|for (") && Token::simpleMatch(tok->next()->link(), ") {")) {
                Token* endCond = tok->next()->link();
                Token* endBlock = endCond->next()->link();
                Token* condTok = getCondTok(tok);
                Token* initTok = getInitTok(tok);
                if (!condTok)
                    return Progress::Break;
                if (initTok && updateRecursive(initTok) == Progress::Break)
                    return Progress::Break;
                if (Token::Match(tok, "for|while (")) {
                    // For-range loop
                    if (Token::simpleMatch(condTok, ":")) {
                        Token* conTok = condTok->astOperand2();
                        if (conTok && updateRecursive(conTok) == Progress::Break)
                            return Progress::Break;
                        if (updateLoop(endBlock, condTok) == Progress::Break)
                            return Progress::Break;
                    } else {
                        Token* stepTok = getStepTok(tok);
                        if (updateLoop(endBlock, condTok, initTok, stepTok) == Progress::Break)
                            return Progress::Break;

                    }
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
                    bool hasElse = Token::simpleMatch(endBlock, "} else {");
                    bool bail = false;

                    // Traverse then block
                    bool returnThen = isEscapeScope(endBlock, true);
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
                    if (hasElse) {
                        returnElse = isEscapeScope(endBlock->linkAt(2), true);
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
                    if (returnThen && !hasElse) {
                        if (checkThen) {
                            return Progress::Break;
                        } else {
                            if (analyzer->isConditional())
                                return Progress::Break;
                            analyzer->assume(condTok, false);
                        }
                    }
                    if (thenAction.isInconclusive() || elseAction.isInconclusive()) {
                        if (!analyzer->lowerToInconclusive())
                            return Progress::Break;
                    } else if (thenAction.isModified() || elseAction.isModified()) {
                        if (!hasElse && analyzer->isConditional())
                            return Progress::Break;
                        if (!analyzer->lowerToPossible())
                            return Progress::Break;
                        analyzer->assume(condTok, elseAction.isModified());
                    }
                }
            } else if (Token::simpleMatch(tok, "try {")) {
                Token* endBlock = tok->next()->link();
                ForwardAnalyzer::Action a = analyzeScope(endBlock);
                if (updateRange(tok->next(), endBlock) == Progress::Break)
                    return Progress::Break;
                if (a.isModified())
                    analyzer->lowerToPossible();
                tok = endBlock;
            } else if (Token::simpleMatch(tok, "do {")) {
                Token* endBlock = tok->next()->link();
                Token* condTok = Token::simpleMatch(endBlock, "} while (") ? endBlock->tokAt(2)->astOperand2() : nullptr;
                if (updateLoop(endBlock, condTok) == Progress::Break)
                    return Progress::Break;
                if (condTok)
                    tok = endBlock->linkAt(2)->next();
                else
                    tok = endBlock;
            } else if (Token::Match(tok, "assert|ASSERT (")) {
                const Token* condTok = tok->next()->astOperand2();
                bool checkThen, checkElse;
                std::tie(checkThen, checkElse) = evalCond(condTok);
                if (checkElse)
                    return Progress::Break;
                if (!checkThen)
                    analyzer->assume(condTok, true, tok);
            } else if (Token::simpleMatch(tok, "switch (")) {
                if (updateRecursive(tok->next()->astOperand2()) == Progress::Break)
                    return Progress::Break;
                return Progress::Break;
            } else {
                if (updateTok(tok, &next) == Progress::Break)
                    return Progress::Break;
                if (next) {
                    if (precedes(next, end))
                        tok = next->previous();
                    else
                        return Progress::Break;
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

    static const Scope* findBreakScope(const Scope* scope) {
        while (scope && scope->type != Scope::eWhile && scope->type != Scope::eFor && scope->type != Scope::eSwitch)
            scope = scope->nestedIn;
        return scope;
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

void valueFlowGenericForward(Token* start, const Token* end, const ValuePtr<ForwardAnalyzer>& fa, const Settings* settings)
{
    ForwardTraversal ft{fa, settings};
    ft.updateRange(start, end);
}
