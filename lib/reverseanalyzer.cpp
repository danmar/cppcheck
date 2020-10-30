#include "reverseanalyzer.h"
#include "forwardanalyzer.h"
#include "analyzer.h"
#include "astutils.h"
#include "settings.h"
#include "symboldatabase.h"
#include "token.h"
#include "valueptr.h"

#include <algorithm>
#include <functional>

struct ReverseTraversal {
    ReverseTraversal(const ValuePtr<GenericAnalyzer>& analyzer, const Settings* settings)
        : analyzer(analyzer), settings(settings)
    {}
    ValuePtr<GenericAnalyzer> analyzer;
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
        GenericAnalyzer::Action action = analyzer->analyze(tok, GenericAnalyzer::Direction::Reverse);
        if (!action.isNone())
            analyzer->update(tok, action, GenericAnalyzer::Direction::Reverse);
        if (action.isInconclusive() && !analyzer->lowerToInconclusive())
            return false;
        if (action.isInvalid())
            return false;
        return true;
    }

    bool updateRecursive(Token* start) {
        bool continueB = true;
        visitAstNodes(start, [&](Token *tok) {
            continueB &= update(tok);
            if (continueB)
                return ChildrenToVisit::op1_and_op2;
            else
                return ChildrenToVisit::done;
        });
        return continueB;
    }

    GenericAnalyzer::Action analyzeRecursive(const Token* start) {
        GenericAnalyzer::Action result = GenericAnalyzer::Action::None;
        visitAstNodes(start, [&](const Token *tok) {
            result |= analyzer->analyze(tok, GenericAnalyzer::Direction::Reverse);
            if (result.isModified())
                return ChildrenToVisit::done;
            return ChildrenToVisit::op1_and_op2;
        });
        return result;
    }

    GenericAnalyzer::Action analyzeRange(const Token* start, const Token* end) {
        GenericAnalyzer::Action result = GenericAnalyzer::Action::None;
        for (const Token* tok = start; tok && tok != end; tok = tok->next()) {
            GenericAnalyzer::Action action = analyzer->analyze(tok, GenericAnalyzer::Direction::Reverse);
            if (action.isModified())
                return action;
            result |= action;
        }
        return result;
    }

    Token* isDeadCode(Token *tok)
    {
        int opSide = 0;
        for (; tok && tok->astParent(); tok = tok->astParent()) {
            Token *parent = tok->astParent();
            if (tok != parent->astOperand2())
                continue;
            if (Token::Match(parent, ":")) {
                if (astIsLHS(tok))
                    opSide = 1;
                else if (astIsRHS(tok))
                    opSide = 2;
                else
                    opSide = 0;
            }
            if (!Token::Match(parent, "%oror%|&&|?"))
                continue;
            Token* condTok = parent->astOperand1();
            if (!condTok)
                continue;
            bool checkThen, checkElse;
            std::tie(checkThen, checkElse) = evalCond(condTok);

            if (!checkThen && !checkElse) {
                GenericAnalyzer::Action action = analyzeRecursive(condTok);
                if (action.isRead() || action.isModified())
                    return parent;
            }

            if (parent->str() == "?") {
                if (!checkElse && opSide == 1)
                    return parent;
                if (!checkThen && opSide == 2)
                    return parent;
            }
            if (!checkThen && parent->str() == "&&")
                return parent;
            if (!checkElse && parent->str() == "||")
                return parent;
        }
        return nullptr;
    }

    void traverse(Token* start) {
        for (Token *tok = start->previous(); tok; tok = tok->previous()) {
            if (tok == start || (tok->str() == "{" && (tok->scope()->type == Scope::ScopeType::eFunction || tok->scope()->type == Scope::ScopeType::eLambda))) {
                break;
            }
            if (Token::Match(tok, "return|break|continue"))
                break;
            if (tok->str() == "}") {
                Token* condTok = getCondTokFromEnd(tok);
                if (!condTok)
                    break;
                const bool inLoop = condTok->astTop() && Token::Match(condTok->astTop()->previous(), "for|while (");
                // Evaluate condition of for and while loops first
                if (inLoop) {
                    GenericAnalyzer::Action action = analyzeRecursive(condTok);
                    if (action.isModified())
                        break;
                    valueFlowGenericForward(condTok, analyzer, settings);
                }
                Token* thenEnd = nullptr;
                Token* elseEnd = nullptr;
                const bool hasElse = Token::simpleMatch(tok->link()->tokAt(-2), "} else {");
                if (hasElse) {
                    elseEnd = tok;
                    thenEnd = tok->link()->tokAt(-2);
                } else {
                    thenEnd = tok;
                }

                GenericAnalyzer::Action thenAction = analyzeRange(thenEnd->link(), thenEnd);
                GenericAnalyzer::Action elseAction = GenericAnalyzer::Action::None;
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
                else if (thenAction.isIdempotent() || elseAction.isIdempotent() || thenAction.isRead() || elseAction.isRead())
                    break;
                if (thenAction.isModified() && elseAction.isModified())
                    break;

                if (!thenAction.isModified() && !elseAction.isModified())
                    valueFlowGenericForward(condTok, analyzer, settings);
                tok = condTok->astTop()->previous();
                continue;
            }
            if (tok->str() == "{") {
                if (tok->previous() &&
                (Token::simpleMatch(tok->previous(), "do") ||
                 (tok->strAt(-1) == ")" && Token::Match(tok->linkAt(-1)->previous(), "for|while (")))) {
                    GenericAnalyzer::Action action = analyzeRange(tok, tok->link());
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
            // Evaluate LHS of assignment before RHS
            if (Token* assignTok = assignExpr(tok)) {
                Token* assignTop = assignTok;
                bool continueB = true;
                while(assignTop->isAssignmentOp()) {
                    if (!Token::Match(assignTop->astOperand1(), "%assign%")) {
                        continueB &= updateRecursive(assignTop->astOperand1());
                    }
                    if (!assignTop->astParent())
                        break;
                    assignTop = assignTop->astParent();
                }
                // Simple assign
                if (assignTok->astParent() == assignTop || assignTok == assignTop) {
                    GenericAnalyzer::Action rhsAction = analyzer->analyze(assignTok->astOperand2(), GenericAnalyzer::Direction::Reverse);
                    GenericAnalyzer::Action lhsAction = analyzer->analyze(assignTok->astOperand1(), GenericAnalyzer::Direction::Reverse);
                    // Assignment from
                    if (rhsAction.isRead()) {
                        const std::string info = "Assignment from '" + assignTok->expressionString() + "'";
                        ValuePtr<GenericAnalyzer> a = analyzer->reanalyze(assignTok->astOperand1(), info);
                        if (a) {
                            valueFlowGenericForward(nextAfterAstRightmostLeaf(assignTok->astOperand2()), assignTok->astOperand2()->scope()->bodyEnd, a, settings);
                        }
                    // Assignment to
                    } else if (lhsAction.matches()) {
                        const std::string info = "Assignment to '" + assignTok->expressionString() + "'";
                        ValuePtr<GenericAnalyzer> a = analyzer->reanalyze(assignTok->astOperand2(), info);
                        if (a) {
                            valueFlowGenericForward(nextAfterAstRightmostLeaf(assignTok->astOperand2()), assignTok->astOperand2()->scope()->bodyEnd, a, settings);
                            valueFlowGenericReverse(assignTok->astOperand1()->previous(), a, settings);
                        }
                    }
                }
                if (!continueB)
                    break;
                valueFlowGenericForward(assignTop->astOperand2(), analyzer, settings);
                tok = previousBeforeAstLeftmostLeaf(assignTop);
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

void valueFlowGenericReverse(Token* start,
        const ValuePtr<GenericAnalyzer>& a,
        const Settings* settings)
{
    ReverseTraversal rt{a, settings};
    rt.traverse(start);
}

