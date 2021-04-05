#include "reverseanalyzer.h"
#include "analyzer.h"
#include "astutils.h"
#include "errortypes.h"
#include "forwardanalyzer.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueptr.h"

#include <algorithm>

struct ReverseTraversal {
    ReverseTraversal(const ValuePtr<Analyzer>& analyzer, const Settings* settings)
        : analyzer(analyzer), settings(settings)
    {}
    ValuePtr<Analyzer> analyzer;
    const Settings* settings;

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

    bool update(Token* tok) {
        Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Reverse);
        if (!action.isNone())
            analyzer->update(tok, action, Analyzer::Direction::Reverse);
        if (action.isInconclusive() && !analyzer->lowerToInconclusive())
            return false;
        if (action.isInvalid())
            return false;
        return true;
    }

    bool updateRecursive(Token* start) {
        bool continueB = true;
        visitAstNodes(start, [&](Token* tok) {
            continueB &= update(tok);
            if (continueB)
                return ChildrenToVisit::op1_and_op2;
            else
                return ChildrenToVisit::done;
        });
        return continueB;
    }

    Analyzer::Action analyzeRecursive(const Token* start) {
        Analyzer::Action result = Analyzer::Action::None;
        visitAstNodes(start, [&](const Token* tok) {
            result |= analyzer->analyze(tok, Analyzer::Direction::Reverse);
            if (result.isModified())
                return ChildrenToVisit::done;
            return ChildrenToVisit::op1_and_op2;
        });
        return result;
    }

    Analyzer::Action analyzeRange(const Token* start, const Token* end) {
        Analyzer::Action result = Analyzer::Action::None;
        for (const Token* tok = start; tok && tok != end; tok = tok->next()) {
            Analyzer::Action action = analyzer->analyze(tok, Analyzer::Direction::Reverse);
            if (action.isModified())
                return action;
            result |= action;
        }
        return result;
    }

    Token* isDeadCode(Token* tok) {
        int opSide = 0;
        for (; tok && tok->astParent(); tok = tok->astParent()) {
            Token* parent = tok->astParent();
            if (Token::simpleMatch(parent, ":")) {
                if (astIsLHS(tok))
                    opSide = 1;
                else if (astIsRHS(tok))
                    opSide = 2;
                else
                    opSide = 0;
            }
            if (tok != parent->astOperand2())
                continue;
            if (!Token::Match(parent, "%oror%|&&|?"))
                continue;
            Token* condTok = parent->astOperand1();
            if (!condTok)
                continue;
            bool checkThen, checkElse;
            std::tie(checkThen, checkElse) = evalCond(condTok);

            if (parent->str() == "?") {
                if (checkElse && opSide == 1)
                    return parent;
                if (checkThen && opSide == 2)
                    return parent;
            }
            if (!checkThen && parent->str() == "&&")
                return parent;
            if (!checkElse && parent->str() == "||")
                return parent;
        }
        return nullptr;
    }

    void traverse(Token* start, const Token* end = nullptr) {
        if (start == end)
            return;
        std::size_t i = start->index();
        for (Token* tok = start->previous(); tok != end; tok = tok->previous()) {
            if (tok->index() >= i)
                throw InternalError(tok, "Cyclic reverse analysis.");
            i = tok->index();
            if (tok == start || (tok->str() == "{" && (tok->scope()->type == Scope::ScopeType::eFunction ||
                                 tok->scope()->type == Scope::ScopeType::eLambda))) {
                break;
            }
            if (Token::Match(tok, "return|break|continue"))
                break;
            if (Token::Match(tok, "%name% :"))
                break;
            if (Token::simpleMatch(tok, ":"))
                continue;
            // Evaluate LHS of assignment before RHS
            if (Token* assignTok = assignExpr(tok)) {
                // If assignTok has broken ast then stop
                if (!assignTok->astOperand1() || !assignTok->astOperand2())
                    break;
                Token* assignTop = assignTok;
                bool continueB = true;
                while (assignTop->isAssignmentOp()) {
                    if (!Token::Match(assignTop->astOperand1(), "%assign%")) {
                        continueB &= updateRecursive(assignTop->astOperand1());
                    }
                    if (!assignTop->astParent())
                        break;
                    assignTop = assignTop->astParent();
                }
                // Is assignment in dead code
                if (Token* parent = isDeadCode(assignTok)) {
                    tok = parent;
                    continue;
                }
                // Simple assign
                if (assignTok->astParent() == assignTop || assignTok == assignTop) {
                    Analyzer::Action rhsAction =
                        analyzer->analyze(assignTok->astOperand2(), Analyzer::Direction::Reverse);
                    Analyzer::Action lhsAction =
                        analyzer->analyze(assignTok->astOperand1(), Analyzer::Direction::Reverse);
                    // Assignment from
                    if (rhsAction.isRead() && !lhsAction.isInvalid()) {
                        const std::string info = "Assignment from '" + assignTok->expressionString() + "'";
                        ValuePtr<Analyzer> a = analyzer->reanalyze(assignTok->astOperand1(), info);
                        if (a) {
                            valueFlowGenericForward(nextAfterAstRightmostLeaf(assignTok->astOperand2()),
                                                    assignTok->astOperand2()->scope()->bodyEnd,
                                                    a,
                                                    settings);
                        }
                        // Assignment to
                    } else if (lhsAction.matches() && !assignTok->astOperand2()->hasKnownValue()) {
                        const std::string info = "Assignment to '" + assignTok->expressionString() + "'";
                        ValuePtr<Analyzer> a = analyzer->reanalyze(assignTok->astOperand2(), info);
                        if (a) {
                            valueFlowGenericForward(nextAfterAstRightmostLeaf(assignTok->astOperand2()),
                                                    assignTok->astOperand2()->scope()->bodyEnd,
                                                    a,
                                                    settings);
                            valueFlowGenericReverse(assignTok->astOperand1()->previous(), end, a, settings);
                        }
                    }
                }
                if (!continueB)
                    break;
                Analyzer::Action a = valueFlowGenericForward(assignTop->astOperand2(), analyzer, settings);
                if (a.isModified())
                    break;
                tok = previousBeforeAstLeftmostLeaf(assignTop)->next();
                continue;
            }
            if (tok->str() == "}") {
                Token* condTok = getCondTokFromEnd(tok);
                if (!condTok)
                    break;
                Analyzer::Action condAction = analyzeRecursive(condTok);
                const bool inLoop = condTok->astTop() && Token::Match(condTok->astTop()->previous(), "for|while (");
                // Evaluate condition of for and while loops first
                if (inLoop) {
                    if (condAction.isModified())
                        break;
                    valueFlowGenericForward(condTok, analyzer, settings);
                }
                Token* thenEnd;
                const bool hasElse = Token::simpleMatch(tok->link()->tokAt(-2), "} else {");
                if (hasElse) {
                    thenEnd = tok->link()->tokAt(-2);
                } else {
                    thenEnd = tok;
                }

                Analyzer::Action thenAction = analyzeRange(thenEnd->link(), thenEnd);
                Analyzer::Action elseAction = Analyzer::Action::None;
                if (hasElse) {
                    elseAction = analyzeRange(tok->link(), tok);
                }
                if (thenAction.isModified() && inLoop)
                    break;
                else if (thenAction.isModified() && !elseAction.isModified())
                    analyzer->assume(condTok, hasElse, condTok);
                else if (elseAction.isModified() && !thenAction.isModified())
                    analyzer->assume(condTok, !hasElse, condTok);
                // Bail if one of the branches are read to avoid FPs due to over constraints
                else if (thenAction.isIdempotent() || elseAction.isIdempotent() || thenAction.isRead() ||
                         elseAction.isRead())
                    break;
                if (thenAction.isInvalid() || elseAction.isInvalid())
                    break;

                if (!thenAction.isModified() && !elseAction.isModified())
                    valueFlowGenericForward(condTok, analyzer, settings);
                else if (condAction.isRead())
                    break;
                // If the condition modifies the variable then bail
                if (condAction.isModified())
                    break;
                tok = condTok->astTop()->previous();
                continue;
            }
            if (tok->str() == "{") {
                if (tok->previous() &&
                    (Token::simpleMatch(tok->previous(), "do") ||
                     (tok->strAt(-1) == ")" && Token::Match(tok->linkAt(-1)->previous(), "for|while (")))) {
                    Analyzer::Action action = analyzeRange(tok, tok->link());
                    if (action.isModified())
                        break;
                }
                if (Token::simpleMatch(tok->tokAt(-2), "} else {"))
                    tok = tok->linkAt(-2);
                if (Token::simpleMatch(tok->previous(), ") {"))
                    tok = tok->previous()->link();
                continue;
            }
            if (Token* next = isUnevaluated(tok)) {
                tok = next;
                continue;
            }
            if (Token* parent = isDeadCode(tok)) {
                tok = parent;
                continue;
            }
            if (!update(tok))
                break;
        }
    }

    static Token* assignExpr(Token* tok) {
        while (tok->astParent() && (astIsRHS(tok) || !tok->astParent()->isBinaryOp())) {
            if (tok->astParent()->isAssignmentOp())
                return tok->astParent();
            tok = tok->astParent();
        }
        return nullptr;
    }

    static Token* isUnevaluated(Token* tok) {
        if (Token::Match(tok, ")|>") && tok->link()) {
            Token* start = tok->link();
            if (Token::Match(start->previous(), "sizeof|decltype ("))
                return start->previous();
            if (Token::simpleMatch(start, "<"))
                return start;
        }
        return nullptr;
    }
};

void valueFlowGenericReverse(Token* start, const ValuePtr<Analyzer>& a, const Settings* settings)
{
    ReverseTraversal rt{a, settings};
    rt.traverse(start);
}

void valueFlowGenericReverse(Token* start, const Token* end, const ValuePtr<Analyzer>& a, const Settings* settings)
{
    ReverseTraversal rt{a, settings};
    rt.traverse(start, end);
}
