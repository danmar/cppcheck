#include "forwardanalyzer.h"
#include "astutils.h"

struct ForwardTraversal
{
    enum class Progress {
        Continue,
        Break
    };
    ValuePtr<ForwardAnalyzer> analyzer;

    Progress update(Token* tok) {
        ForwardAnalyzer::Action action = analyzer->Analyze(tok);
        if (action != ForwardAnalyzer::Action::None)
            analyzer->Update(tok, action);
        if (action == ForwardAnalyzer::Action::Invalid)
            return Progress::Break;
        return Progress::Continue;
    }

    Progress updateRecursive(Token* tok) {
        if (!tok)
            return Progress::Continue;
        if (tok->astOperand1() && updateRecursive(tok->astOperand1()) == Progress::Break)
            return Progress::Break;
        if (update(tok) == Progress::Break)
            return Progress::Break;
        if (tok->astOperand2() && updateRecursive(tok->astOperand2()) == Progress::Break)
            return Progress::Break;
        return Progress::Continue;
    }

    ForwardAnalyzer::Action analyzeRange(const Token* start, const Token* end) {
        for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
            ForwardAnalyzer::Action action = analyzer->Analyze(tok);
            if (action != ForwardAnalyzer::Action::None && action != ForwardAnalyzer::Action::Read)
                return action;
        }
        return ForwardAnalyzer::Action::None;
    }

    Progress updateRange(Token* start, const Token* end) {
        for (Token *tok = start; tok && tok != end; tok = tok->next()) {
            if (Token::Match(tok, "asm|goto|break|continue"))
                return Progress::Break;
            if (Token::Match(tok, "return|throw")) {
                updateRecursive(tok);
                return Progress::Break;
            }

            if (Token::simpleMatch(tok, "}") && Token::simpleMatch(tok->link()->previous(), ") {") && Token::Match(tok->link()->linkAt(-1)->previous(), "if|while|for (")) {
                const Token * blockStart = tok->link()->linkAt(-1)->previous();
                const Token * condTok = getCondTok(blockStart);
                if (!condTok)
                    return Progress::Break;
                analyzer->LowerToPossible();
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
            }

            if (Token::Match(tok, "if|while|for (") && Token::simpleMatch(tok->next()->link(), ") {")) {
                Token * endCond = tok->next()->link();
                Token * endBlock = endCond->next()->link();
                const Token * condTok = getCondTok(tok);
                if (!condTok)
                    return Progress::Break;
                // Traverse condition
                if (updateRecursive(tok) == Progress::Break)
                    return Progress::Break;

                // Check if condition is true or false
                std::vector<int> result = analyzer->Evaluate(condTok);
                bool checkThen = std::any_of(result.begin(), result.end(), [](int x) { return x; });
                bool checkElse = std::any_of(result.begin(), result.end(), [](int x) { return !x; });

                if (!checkThen && !checkElse) {
                    ForwardAnalyzer::Action a = analyzeRange(endCond->next(), endBlock);
                    if (a == ForwardAnalyzer::Action::Write || a == ForwardAnalyzer::Action::Invalid) {
                        analyzer->LowerToPossible();
                    } else if (a == ForwardAnalyzer::Action::Inconclusive) {
                        analyzer->LowerToInconclusive();
                    }
                }

                // Traverse then block
                if (checkThen) {
                    if (updateRange(endCond->next(), endBlock) == Progress::Break)
                        return Progress::Break;
                }
                // Traverse else block
                if (Token::simpleMatch(endBlock, "} else {")) {
                    if (checkElse) {
                        Progress result = updateRange(endCond->next(), endBlock);
                        if (result == Progress::Break)
                            return Progress::Break;
                    }
                    tok = endBlock->linkAt(2);
                } else {
                    tok = endBlock;
                }
            } else if (Token::simpleMatch(tok, "} else {")) {
                tok = tok->linkAt(2);
            // Skip lambdas
            } else if (Token *lambdaEndToken = findLambdaEndToken(tok)) {
                tok = lambdaEndToken;
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

    static const Token* getCondTok(const Token* tok)
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
};

void valueFlowGenericForward(Token* start, const Token* end, const ValuePtr<ForwardAnalyzer>& fa)
{
    ForwardTraversal ft{fa};
    ft.updateRange(start, end);
}
