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

    std::pair<bool, bool> checkCond(const Token* tok)
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
        if (tok->astOperand1() && updateRecursive(tok->astOperand1()) == Progress::Break)
            return Progress::Break;
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
            const Token * condTok = tok->astOperand1();
            if (!condTok)
                return Progress::Break;
            Token * childTok = tok->astOperand2();
            bool checkThen, checkElse;
            std::tie(checkThen, checkElse) = checkCond(condTok);
            if (!checkThen && !checkElse) {
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
                if (updateRecursive(childTok->astOperand2()) == Progress::Break)
                    return Progress::Break;
            }
        }
        return Progress::Continue;
    }

    ForwardAnalyzer::Action analyzeRange(const Token* start, const Token* end) {
        for (const Token *tok = start; tok && tok != end; tok = tok->next()) {
            ForwardAnalyzer::Action action = analyzer->Analyze(tok);
            if (action.isModified() || action.isInconclusive())
                return action;
        }
        return ForwardAnalyzer::Action::None;
    }

    enum class Status {
        None,
        Escaped,
        Modified,
        Inconclusive,
    };

    Status checkScope(const Token* endBlock, bool checkReturn=true) {
        if (checkReturn && isReturnScope(endBlock, &settings->library))
            return Status::Escaped;
        ForwardAnalyzer::Action a = analyzeRange(endBlock->link(), endBlock);
        if (analyzer->UpdateScope(endBlock, a.isModified())) {
            ForwardTraversal ft = *this;
            ft.updateRange(endBlock->link(), endBlock);
        }            
        if (a.isInconclusive()) {
            return Status::Inconclusive;
        } else if (a.isModified()) {
            return Status::Modified;
        }
        return Status::None;
    }

    Progress updateRange(Token* start, const Token* end) {
        for (Token *tok = start; tok && tok != end; tok = tok->next()) {
            if (Token::Match(tok, "asm|goto|continue"))
                return Progress::Break;
            if (Token::Match(tok, "return|throw") || isEscapeFunction(tok, &settings->library)) {
                updateRecursive(tok);
                return Progress::Break;
            }

            if (Token::simpleMatch(tok, "break")) {
                const Scope* scope = findBreakScope(tok->scope());
                if (!scope)
                    return Progress::Break;
                tok = skipTo(tok, scope->bodyEnd, end);
                analyzer->LowerToPossible();
            } else if (Token::simpleMatch(tok, "}") && Token::simpleMatch(tok->link()->previous(), ") {") && Token::Match(tok->link()->linkAt(-1)->previous(), "if|while|for (")) {
                const Token * blockStart = tok->link()->linkAt(-1)->previous();
                const Token * condTok = getCondTok(blockStart);
                if (!condTok)
                    return Progress::Break;
                if (!condTok->hasKnownIntValue())
                    analyzer->LowerToPossible();
                else if (condTok->values().front().intvalue == 0)
                    return Progress::Break;
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
                // Traverse condition
                if (updateRecursive(condTok) == Progress::Break)
                    return Progress::Break;

                if (Token::Match(tok, "for|while (")) {
                    Status loopStatus = checkScope(endBlock);
                    if (loopStatus == Status::Inconclusive)
                        analyzer->LowerToInconclusive();
                    else if (loopStatus == Status::Modified)
                        analyzer->LowerToPossible();  
                } else {
                    // Check if condition is true or false
                    bool checkThen, checkElse;
                    std::tie(checkThen, checkElse) = checkCond(condTok);
                    Status thenStatus = Status::None;
                    Status elseStatus = Status::None;

                    // Traverse then block
                    if (checkThen) {
                        if (updateRange(endCond->next(), endBlock) == Progress::Break)
                            return Progress::Break;
                    } else if (!checkElse) {
                        thenStatus = checkScope(endBlock);
                    }
                    // Traverse else block
                    if (Token::simpleMatch(endBlock, "} else {")) {
                        if (checkElse) {
                            Progress result = updateRange(endCond->next(), endBlock);
                            if (result == Progress::Break)
                                return Progress::Break;
                        } else if (!checkThen) {
                            elseStatus = checkScope(endBlock->linkAt(2));
                        }
                        tok = endBlock->linkAt(2);
                    } else {
                        tok = endBlock;
                    }
                    if (thenStatus == Status::Escaped && elseStatus == Status::Escaped) {
                        return Progress::Break;
                    } else if (thenStatus == Status::Escaped || elseStatus == Status::Escaped) {
                        if (thenStatus == Status::Modified || elseStatus == Status::Modified) {
                            return Progress::Break;
                        }
                    } else if (thenStatus == Status::Inconclusive || elseStatus == Status::Inconclusive) {
                        if (!analyzer->LowerToInconclusive())
                            return Progress::Break;
                    } else if (thenStatus == Status::Modified || elseStatus == Status::Modified) {
                        if (!analyzer->LowerToPossible())
                            return Progress::Break;
                    }
                }
            } else if (Token::simpleMatch(tok, "} else {")) {
                tok = tok->linkAt(2);
            } else if (Token::Match(tok, "?|&&|%oror%")) {
                updateConditional(tok);
                tok = nextAfterAstRightmostLeaf(tok);
            } else if (Token::simpleMatch(tok, "switch (")) {
                return Progress::Break;
            // Skip lambdas
            } else if (Token *lambdaEndToken = findLambdaEndToken(tok)) {
                if (checkScope(lambdaEndToken, false) == Status::Modified)
                    return Progress::Break;
                tok = lambdaEndToken;
            // Skip unevaluated context
            } else if (Token::Match(tok, "sizeof|decltype (")) {
                tok = tok->next()->link();
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
        while(tok != dest && tok != end)
            tok = tok->next();
        return tok;
    }
};

void valueFlowGenericForward(Token* start, const Token* end, const ValuePtr<ForwardAnalyzer>& fa, const Settings* settings)
{
    ForwardTraversal ft{fa, settings};
    ft.updateRange(start, end);
}
