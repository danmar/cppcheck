#include "forwardanalyzer.h"
#include "analyzer.h"
#include "astutils.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueptr.h"

#include <algorithm>
#include <cstdio>
#include <functional>
#include <tuple>
#include <utility>

struct OnExit {
    std::function<void()> f;

    ~OnExit() {
        f();
    }
};

struct ForwardTraversal {
    enum class Progress { Continue, Break, Skip };
    enum class Terminate { None, Bail, Escape, Modified, Inconclusive, Conditional };
    ForwardTraversal(const ValuePtr<Analyzer>& analyzer, const Settings* settings)
        : analyzer(analyzer), settings(settings), actions(Analyzer::Action::None), analyzeOnly(false), analyzeTerminate(false)
    {}
    ValuePtr<Analyzer> analyzer;
    const Settings* settings;
    Analyzer::Action actions;
    bool analyzeOnly;
    bool analyzeTerminate;
    Analyzer::Terminate terminate = Analyzer::Terminate::None;
    bool forked = false;
    std::vector<Token*> loopEnds = {};

    Progress Break(Analyzer::Terminate t = Analyzer::Terminate::None) {
        if ((!analyzeOnly || analyzeTerminate) && t != Analyzer::Terminate::None)
            terminate = t;
        return Progress::Break;
    }

    struct Branch {
        Branch(Token* tok = nullptr) : endBlock(tok) {}
        Token* endBlock = nullptr;
        Analyzer::Action action = Analyzer::Action::None;
        bool check = false;
        bool escape = false;
        bool escapeUnknown = false;
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

    std::pair<bool, bool> evalCond(const Token* tok, const Token* ctx = nullptr) const {
        if (!tok)
            return std::make_pair(false, false);
        std::vector<int> result = analyzer->evaluate(tok, ctx);
        // TODO: We should convert to bool
        bool checkThen = std::any_of(result.begin(), result.end(), [](int x) {
            return x != 0;
        });
        bool checkElse = std::any_of(result.begin(), result.end(), [](int x) {
            return x == 0;
        });
        return std::make_pair(checkThen, checkElse);
    }

    bool isConditionTrue(const Token* tok, const Token* ctx = nullptr) const {
        return evalCond(tok, ctx).first;
    }

    bool isConditionFalse(const Token* tok, const Token* ctx = nullptr) const {
        return evalCond(tok, ctx).second;
    }

    template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
    Progress traverseTok(T* tok, std::function<Progress(T*)> f, bool traverseUnknown, T** out = nullptr) {
        if (Token::Match(tok, "asm|goto|setjmp|longjmp"))
            return Break(Analyzer::Terminate::Bail);
        else if (Token::simpleMatch(tok, "continue")) {
            if (loopEnds.empty())
                return Break(Analyzer::Terminate::Escape);
            // If we are in a loop then jump to the end
            if (out)
                *out = loopEnds.back();
        } else if (Token::Match(tok, "return|throw") || isEscapeFunction(tok, &settings->library)) {
            traverseRecursive(tok->astOperand1(), f, traverseUnknown);
            traverseRecursive(tok->astOperand2(), f, traverseUnknown);
            return Break(Analyzer::Terminate::Escape);
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
                return Break(Analyzer::Terminate::Bail);
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

    template<class T, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
    Progress traverseRecursive(T* tok, std::function<Progress(T*)> f, bool traverseUnknown, unsigned int recursion=0) {
        if (!tok)
            return Progress::Continue;
        if (recursion > 10000)
            return Progress::Skip;
        T* firstOp = tok->astOperand1();
        T* secondOp = tok->astOperand2();
        // Evaluate:
        //     1. RHS of assignment before LHS
        //     2. Unary op before operand
        if (tok->isAssignmentOp() || !secondOp)
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

    template<class T, class F, REQUIRES("T must be a Token class", std::is_convertible<T*, const Token*> )>
    Progress traverseConditional(T* tok, F f, bool traverseUnknown) {
        if (Token::Match(tok, "?|&&|%oror%") && tok->astOperand1() && tok->astOperand2()) {
            T* condTok = tok->astOperand1();
            T* childTok = tok->astOperand2();
            bool checkThen, checkElse;
            std::tie(checkThen, checkElse) = evalCond(condTok);
            if (!checkThen && !checkElse) {
                if (!traverseUnknown && analyzer->stopOnCondition(condTok) && stopUpdates()) {
                    return Progress::Continue;
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
            return Break(Analyzer::Terminate::Inconclusive);
        if (action.isInvalid())
            return Break(Analyzer::Terminate::Modified);
        if (action.isWrite() && !action.isRead())
            // Analysis of this write will continue separately
            return Break(Analyzer::Terminate::Modified);
        return Progress::Continue;
    }

    Progress updateTok(Token* tok, Token** out = nullptr) {
        std::function<Progress(Token*)> f = [this](Token* tok2) {
            return update(tok2);
        };
        return traverseTok(tok, f, false, out);
    }

    Progress updateRecursive(Token* tok) {
        forked = false;
        std::function<Progress(Token*)> f = [this](Token* tok2) {
            return update(tok2);
        };
        return traverseRecursive(tok, f, false);
    }

    template<class T>
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

    ForwardTraversal fork(bool analyze = false) const {
        ForwardTraversal ft = *this;
        if (analyze) {
            ft.analyzeOnly = true;
            ft.analyzeTerminate = true;
        }
        ft.actions = Analyzer::Action::None;
        ft.forked = true;
        return ft;
    }

    std::vector<ForwardTraversal> tryForkScope(Token* endBlock, bool isModified = false) {
        if (analyzer->updateScope(endBlock, isModified)) {
            ForwardTraversal ft = fork();
            return {ft};
        }
        return std::vector<ForwardTraversal> {};
    }

    std::vector<ForwardTraversal> tryForkUpdateScope(Token* endBlock, bool isModified = false) {
        std::vector<ForwardTraversal> result = tryForkScope(endBlock, isModified);
        for (ForwardTraversal& ft : result)
            ft.updateScope(endBlock);
        return result;
    }

    static bool hasGoto(const Token* endBlock) {
        return Token::findsimplematch(endBlock->link(), "goto", endBlock);
    }

    bool hasInnerReturnScope(const Token* start, const Token* end) const {
        for (const Token* tok=start; tok != end; tok = tok->previous()) {
            if (Token::simpleMatch(tok, "}")) {
                const Token* ftok = nullptr;
                bool r = isReturnScope(tok, &settings->library, &ftok);
                if (r)
                    return true;
            }
        }
        return false;
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
        tryForkUpdateScope(endBlock, a.isModified());
        return a;
    }

    Analyzer::Action checkScope(const Token* endBlock) {
        Analyzer::Action a = analyzeScope(endBlock);
        return a;
    }

    bool checkBranch(Branch& branch) {
        Analyzer::Action a = analyzeScope(branch.endBlock);
        branch.action = a;
        std::vector<ForwardTraversal> ft1 = tryForkUpdateScope(branch.endBlock, a.isModified());
        bool bail = hasGoto(branch.endBlock);
        if (!a.isModified() && !bail) {
            if (ft1.empty()) {
                // Traverse into the branch to see if there is a conditional escape
                if (!branch.escape && hasInnerReturnScope(branch.endBlock->previous(), branch.endBlock->link())) {
                    ForwardTraversal ft2 = fork(true);
                    ft2.updateScope(branch.endBlock);
                    if (ft2.terminate == Analyzer::Terminate::Escape) {
                        branch.escape = true;
                        branch.escapeUnknown = false;
                    }
                }
            } else {
                if (ft1.front().terminate == Analyzer::Terminate::Escape) {
                    branch.escape = true;
                    branch.escapeUnknown = false;
                }
            }
        }
        return bail;
    }

    bool reentersLoop(Token* endBlock, const Token* condTok, const Token* stepTok) {
        if (!condTok)
            return true;
        if (Token::simpleMatch(condTok, ":"))
            return true;
        bool changed = false;
        if (stepTok) {
            std::pair<const Token*, const Token*> exprToks = stepTok->findExpressionStartEndTokens();
            if (exprToks.first != nullptr && exprToks.second != nullptr)
                changed |= isExpressionChanged(condTok, exprToks.first, exprToks.second->next(), settings, true);
        }
        changed |= isExpressionChanged(condTok, endBlock->link(), endBlock, settings, true);
        // Check for mutation in the condition
        changed |= nullptr !=
                   findAstNode(condTok, [&](const Token* tok) {
            return isVariableChanged(tok, 0, settings, true);
        });
        if (!changed)
            return true;
        ForwardTraversal ft = fork(true);
        ft.analyzer->assume(condTok, false, Analyzer::Assume::Absolute);
        ft.updateScope(endBlock);
        return ft.isConditionTrue(condTok);
    }

    Progress updateInnerLoop(Token* endBlock, Token* stepTok, Token* condTok) {
        loopEnds.push_back(endBlock);
        OnExit oe{[&] {
                loopEnds.pop_back();
            }};
        if (endBlock && updateScope(endBlock) == Progress::Break)
            return Break();
        if (stepTok && updateRecursive(stepTok) == Progress::Break)
            return Break();
        if (condTok && updateRecursive(condTok) == Progress::Break)
            return Break();
        return Progress::Continue;
    }

    Progress updateLoop(const Token* endToken,
                        Token* endBlock,
                        Token* condTok,
                        Token* initTok = nullptr,
                        Token* stepTok = nullptr,
                        bool exit = false) {
        if (initTok && updateRecursive(initTok) == Progress::Break)
            return Break();
        const bool isDoWhile = precedes(endBlock, condTok);
        bool checkThen = true;
        bool checkElse = false;
        if (condTok && !Token::simpleMatch(condTok, ":"))
            std::tie(checkThen, checkElse) = evalCond(condTok, isDoWhile ? endBlock->previous() : nullptr);
        if (checkElse && exit)
            return Progress::Continue;
        Analyzer::Action bodyAnalysis = analyzeScope(endBlock);
        Analyzer::Action allAnalysis = bodyAnalysis;
        Analyzer::Action condAnalysis;
        if (condTok) {
            condAnalysis = analyzeRecursive(condTok);
            allAnalysis |= condAnalysis;
        }
        if (stepTok)
            allAnalysis |= analyzeRecursive(stepTok);
        actions |= allAnalysis;
        // do while(false) is not really a loop
        if (checkElse && isDoWhile &&
            (condTok->hasKnownIntValue() || (!bodyAnalysis.isModified() && condAnalysis.isRead()))) {
            if (updateRange(endBlock->link(), endBlock) == Progress::Break)
                return Break();
            return updateRecursive(condTok);
        }
        if (allAnalysis.isInconclusive()) {
            if (!analyzer->lowerToInconclusive())
                return Break(Analyzer::Terminate::Bail);
        } else if (allAnalysis.isModified() || (exit && allAnalysis.isIdempotent())) {
            if (!analyzer->lowerToPossible())
                return Break(Analyzer::Terminate::Bail);
        }

        if (condTok && !Token::simpleMatch(condTok, ":")) {
            if (!isDoWhile || (!bodyAnalysis.isModified() && !bodyAnalysis.isIdempotent()))
                if (updateRecursive(condTok) == Progress::Break)
                    return Break();
        }
        if (!checkThen && !checkElse && !isDoWhile && analyzer->stopOnCondition(condTok) && stopUpdates())
            return Break(Analyzer::Terminate::Conditional);
        // condition is false, we don't enter the loop
        if (checkElse)
            return Progress::Continue;
        if (checkThen || isDoWhile) {
            // Since we are re-entering the loop then assume the condition is true to update the state
            if (exit)
                analyzer->assume(condTok, true, Analyzer::Assume::Quiet | Analyzer::Assume::Absolute);
            if (updateInnerLoop(endBlock, stepTok, condTok) == Progress::Break)
                return Break();
            // If loop re-enters then it could be modified again
            if (allAnalysis.isModified() && reentersLoop(endBlock, condTok, stepTok))
                return Break(Analyzer::Terminate::Bail);
            if (allAnalysis.isIncremental())
                return Break(Analyzer::Terminate::Bail);
        } else if (allAnalysis.isModified()) {
            std::vector<ForwardTraversal> ftv = tryForkScope(endBlock, allAnalysis.isModified());
            bool forkContinue = true;
            for (ForwardTraversal& ft : ftv) {
                if (condTok)
                    ft.analyzer->assume(condTok, false, Analyzer::Assume::Quiet);
                if (ft.updateInnerLoop(endBlock, stepTok, condTok) == Progress::Break)
                    forkContinue = false;
            }

            if (allAnalysis.isModified() || !forkContinue) {
                // TODO: Don't bail on missing condition
                if (!condTok)
                    return Break(Analyzer::Terminate::Bail);
                if (analyzer->isConditional() && stopUpdates())
                    return Break(Analyzer::Terminate::Conditional);
                analyzer->assume(condTok, false);
            }
            if (forkContinue) {
                for (ForwardTraversal& ft : ftv) {
                    if (!ft.actions.isIncremental())
                        ft.updateRange(endBlock, endToken);
                }
            }
            if (allAnalysis.isIncremental())
                return Break(Analyzer::Terminate::Bail);
        } else {
            if (updateInnerLoop(endBlock, stepTok, condTok) == Progress::Break)
                return Progress::Break;
            if (allAnalysis.isIncremental())
                return Break(Analyzer::Terminate::Bail);
        }
        return Progress::Continue;
    }

    Progress updateLoopExit(const Token* endToken,
                            Token* endBlock,
                            Token* condTok,
                            Token* initTok = nullptr,
                            Token* stepTok = nullptr) {
        return updateLoop(endToken, endBlock, condTok, initTok, stepTok, true);
    }

    Progress updateScope(Token* endBlock) {
        if (forked)
            analyzer->forkScope(endBlock);
        return updateRange(endBlock->link(), endBlock);
    }

    Progress updateRange(Token* start, const Token* end, int depth = 20) {
        forked = false;
        if (depth < 0)
            return Break(Analyzer::Terminate::Bail);
        std::size_t i = 0;
        for (Token* tok = start; precedes(tok, end); tok = tok->next()) {
            Token* next = nullptr;
            if (tok->index() <= i)
                throw InternalError(tok, "Cyclic forward analysis.");
            i = tok->index();

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
                    return Break(Analyzer::Terminate::Bail);
                // TODO: Don't break, instead move to the outer scope
                if (!tok)
                    return Break();
            } else if (Token::Match(tok, "%name% :") || tok->str() == "case") {
                if (!analyzer->lowerToPossible())
                    return Break(Analyzer::Terminate::Bail);
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
                            return Break(Analyzer::Terminate::Bail);
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
                    analyzer->assume(condTok, !inElse, Analyzer::Assume::Quiet);
                    if (Token::simpleMatch(tok, "} else {"))
                        tok = tok->linkAt(2);
                } else if (scope->type == Scope::eTry) {
                    if (!analyzer->lowerToPossible())
                        return Break(Analyzer::Terminate::Bail);
                } else if (scope->type == Scope::eLambda) {
                    return Break();
                } else if (scope->type == Scope::eDo && Token::simpleMatch(tok, "} while (")) {
                    if (updateLoopExit(end, tok, tok->tokAt(2)->astOperand2()) == Progress::Break)
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
                        bool isEmpty = false;
                        std::vector<int> result = analyzer->evaluate(Analyzer::Evaluate::ContainerEmpty, conTok);
                        if (result.empty())
                            analyzer->assume(conTok, false, Analyzer::Assume::ContainerEmpty);
                        else
                            isEmpty = result.front() != 0;
                        if (!isEmpty && updateLoop(end, endBlock, condTok) == Progress::Break)
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
                    Branch thenBranch{endBlock};
                    Branch elseBranch{endBlock->tokAt(2) ? endBlock->linkAt(2) : nullptr};
                    // Check if condition is true or false
                    std::tie(thenBranch.check, elseBranch.check) = evalCond(condTok);
                    if (!thenBranch.check && !elseBranch.check && analyzer->stopOnCondition(condTok) && stopUpdates())
                        return Break(Analyzer::Terminate::Conditional);
                    bool hasElse = Token::simpleMatch(endBlock, "} else {");
                    bool bail = false;

                    // Traverse then block
                    thenBranch.escape = isEscapeScope(endBlock, thenBranch.escapeUnknown);
                    if (thenBranch.check) {
                        if (updateRange(endCond->next(), endBlock, depth - 1) == Progress::Break)
                            return Break();
                    } else if (!elseBranch.check) {
                        if (checkBranch(thenBranch))
                            bail = true;
                    }
                    // Traverse else block
                    if (hasElse) {
                        elseBranch.escape = isEscapeScope(endBlock->linkAt(2), elseBranch.escapeUnknown);
                        if (elseBranch.check) {
                            Progress result = updateRange(endBlock->tokAt(2), endBlock->linkAt(2), depth - 1);
                            if (result == Progress::Break)
                                return Break();
                        } else if (!thenBranch.check) {
                            if (checkBranch(elseBranch))
                                bail = true;
                        }
                        tok = endBlock->linkAt(2);
                    } else {
                        tok = endBlock;
                    }
                    actions |= (thenBranch.action | elseBranch.action);
                    if (bail)
                        return Break(Analyzer::Terminate::Bail);
                    if (thenBranch.isDead() && elseBranch.isDead()) {
                        if (thenBranch.isModified() && elseBranch.isModified())
                            return Break(Analyzer::Terminate::Modified);
                        if (thenBranch.isConclusiveEscape() && elseBranch.isConclusiveEscape())
                            return Break(Analyzer::Terminate::Escape);
                        return Break(Analyzer::Terminate::Bail);
                    }
                    // Conditional return
                    if (thenBranch.isEscape() && !hasElse) {
                        if (!thenBranch.isConclusiveEscape()) {
                            if (!analyzer->lowerToInconclusive())
                                return Break(Analyzer::Terminate::Bail);
                        } else if (thenBranch.check) {
                            return Break();
                        } else {
                            if (analyzer->isConditional() && stopUpdates())
                                return Break(Analyzer::Terminate::Conditional);
                            analyzer->assume(condTok, false);
                        }
                    }
                    if (thenBranch.isInconclusive() || elseBranch.isInconclusive()) {
                        if (!analyzer->lowerToInconclusive())
                            return Break(Analyzer::Terminate::Bail);
                    } else if (thenBranch.isModified() || elseBranch.isModified()) {
                        if (!hasElse && analyzer->isConditional() && stopUpdates())
                            return Break(Analyzer::Terminate::Conditional);
                        if (!analyzer->lowerToPossible())
                            return Break(Analyzer::Terminate::Bail);
                        analyzer->assume(condTok, elseBranch.isModified());
                    }
                }
            } else if (Token::simpleMatch(tok, "try {")) {
                Token* endBlock = tok->next()->link();
                ForwardTraversal tryTraversal = fork();
                tryTraversal.updateRange(tok->next(), endBlock, depth - 1);
                bool bail = tryTraversal.actions.isModified();
                if (bail)
                    analyzer->lowerToPossible();

                while (Token::simpleMatch(endBlock, "} catch (")) {
                    Token* endCatch = endBlock->linkAt(2);
                    if (!Token::simpleMatch(endCatch, ") {"))
                        return Break();
                    endBlock = endCatch->linkAt(1);
                    ForwardTraversal ft = fork();
                    ft.updateRange(endBlock->link(), endBlock, depth - 1);
                    bail |= ft.terminate != Analyzer::Terminate::None || ft.actions.isModified();
                }
                if (bail)
                    return Break();
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
                    analyzer->assume(condTok, true, Analyzer::Assume::Quiet | Analyzer::Assume::Absolute);
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

Analyzer::Result valueFlowGenericForward(Token* start, const Token* end, const ValuePtr<Analyzer>& a, const Settings* settings)
{
    ForwardTraversal ft{a, settings};
    ft.updateRange(start, end);
    return {ft.actions, ft.terminate};
}

Analyzer::Result valueFlowGenericForward(Token* start, const ValuePtr<Analyzer>& a, const Settings* settings)
{
    ForwardTraversal ft{a, settings};
    ft.updateRecursive(start);
    return {ft.actions, ft.terminate};
}
