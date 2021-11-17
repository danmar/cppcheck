
#include "summaries.h"

#include "analyzerinfo.h"
#include "settings.h"
#include "symboldatabase.h"
#include "tokenize.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <vector>



std::string Summaries::create(const Tokenizer *tokenizer, const std::string &cfg)
{
    const SymbolDatabase *symbolDatabase = tokenizer->getSymbolDatabase();
    const Settings *settings = tokenizer->getSettings();

    std::ostringstream ostr;
    for (const Scope *scope : symbolDatabase->functionScopes) {
        const Function *f = scope->function;
        if (!f)
            continue;

        // Summarize function
        std::set<std::string> noreturn;
        std::set<std::string> globalVars;
        std::set<std::string> calledFunctions;
        for (const Token* tok = scope->bodyStart; tok != scope->bodyEnd; tok = tok->next()) {
            if (tok->variable() && tok->variable()->isGlobal())
                globalVars.insert(tok->variable()->name());
            if (Token::Match(tok, "%name% (") && !Token::simpleMatch(tok->linkAt(1), ") {")) {
                calledFunctions.insert(tok->str());
                if (Token::simpleMatch(tok->linkAt(1), ") ; }"))
                    noreturn.insert(tok->str());
            }
        }

        // Write summary for function
        auto join = [](const std::set<std::string> &data) -> std::string {
            std::string ret;
            const char *sep = "";
            for (std::string d: data)
            {
                ret += sep + d;
                sep = ",";
            }
            return ret;
        };

        ostr << f->name();
        if (!globalVars.empty())
            ostr << " global:[" << join(globalVars) << "]";
        if (!calledFunctions.empty())
            ostr << " call:[" << join(calledFunctions) << "]";
        if (!noreturn.empty())
            ostr << " noreturn:[" << join(noreturn) << "]";
        ostr << std::endl;
    }

    if (!settings->buildDir.empty()) {
        std::string filename = AnalyzerInformation::getAnalyzerInfoFile(settings->buildDir, tokenizer->list.getSourceFilePath(), cfg);
        std::string::size_type pos = filename.rfind(".a");
        if (pos != std::string::npos) {
            filename[pos+1] = 's';
            std::ofstream fout(filename);
            fout << ostr.str();
        }
    }

    return ostr.str();
}




static std::vector<std::string> getSummaryFiles(const std::string &filename)
{
    std::vector<std::string> ret;
    std::ifstream fin(filename);
    if (!fin.is_open())
        return ret;
    std::string line;
    while (std::getline(fin, line)) {
        std::string::size_type dotA = line.find(".a");
        std::string::size_type colon = line.find(":");
        if (colon > line.size() || dotA > colon)
            continue;
        std::string f = line.substr(0,colon);
        f[dotA + 1] = 's';
        ret.push_back(f);
    }
    return ret;
}

static std::vector<std::string> getSummaryData(const std::string &line, const std::string &data)
{
    std::vector<std::string> ret;
    const std::string::size_type start = line.find(" " + data + ":[");
    if (start == std::string::npos)
        return ret;
    const std::string::size_type end = line.find("]", start);
    if (end >= line.size())
        return ret;

    std::string::size_type pos1 = start + 3 + data.size();
    while (pos1 < end) {
        std::string::size_type pos2 = line.find_first_of(",]",pos1);
        ret.push_back(line.substr(pos1, pos2-pos1-1));
        pos1 = pos2 + 1;
    }

    return ret;
}

static void removeFunctionCalls(const std::string& calledFunction,
                                std::map<std::string, std::vector<std::string>> &functionCalledBy,
                                std::map<std::string, std::vector<std::string>> &functionCalls,
                                std::vector<std::string> &add)
{
    std::vector<std::string> calledBy = functionCalledBy[calledFunction];
    functionCalledBy.erase(calledFunction);
    for (const std::string &c: calledBy) {
        std::vector<std::string> &calls = functionCalls[c];
        calls.erase(std::remove(calls.begin(), calls.end(), calledFunction), calls.end());
        if (calls.empty()) {
            add.push_back(calledFunction);
            removeFunctionCalls(c, functionCalledBy, functionCalls, add);
        }
    }
}

void Summaries::loadReturn(const std::string &buildDir, std::set<std::string> &summaryReturn)
{
    if (buildDir.empty())
        return;

    std::vector<std::string> return1;
    std::map<std::string, std::vector<std::string>> functionCalls;
    std::map<std::string, std::vector<std::string>> functionCalledBy;

    // extract "functionNoreturn" and "functionCalledBy" from summaries
    std::vector<std::string> summaryFiles = getSummaryFiles(buildDir + "/files.txt");
    for (const std::string &filename: summaryFiles) {
        std::ifstream fin(buildDir + '/' + filename);
        if (!fin.is_open())
            continue;
        std::string line;
        while (std::getline(fin, line)) {
            // Get function name
            const std::string::size_type pos1 = 0;
            const std::string::size_type pos2 = line.find(" ", pos1);
            const std::string functionName = (pos2 == std::string::npos) ? line : line.substr(0, pos2);
            std::vector<std::string> call = getSummaryData(line, "call");
            functionCalls[functionName] = call;
            if (call.empty())
                return1.push_back(functionName);
            else {
                for (const std::string &c: call) {
                    functionCalledBy[c].push_back(functionName);
                }
            }
        }
    }
    summaryReturn.insert(return1.cbegin(), return1.cend());

    // recursively set "summaryNoreturn"
    for (const std::string &f: return1) {
        std::vector<std::string> return2;
        removeFunctionCalls(f, functionCalledBy, functionCalls, return2);
        summaryReturn.insert(return2.cbegin(), return2.cend());
    }
}
