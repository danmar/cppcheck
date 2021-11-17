#ifndef GUARD_PATHANALYSIS_H
#define GUARD_PATHANALYSIS_H

#include "errortypes.h"

#include <functional>

class Library;
class Scope;
class Token;

struct PathAnalysis {
    enum class Progress {
        Continue,
        Break
    };
    PathAnalysis(const Token* start, const Library& library)
        : start(start), library(&library)
    {}
    const Token * start;
    const Library * library;

    struct Info {
        const Token* tok;
        ErrorPath errorPath;
        bool known;
    };

    void forward(const std::function<Progress(const Info&)>& f) const;

    Info forwardFind(std::function<bool(const Info&)> pred) {
        Info result{};
        forward([&](const Info& info) {
            if (pred(info)) {
                result = info;
                return Progress::Break;
            }
            return Progress::Continue;
        });
        return result;
    }
private:

    Progress forwardRecursive(const Token* tok, Info info, const std::function<PathAnalysis::Progress(const Info&)>& f) const;
    Progress forwardRange(const Token* startToken, const Token* endToken, Info info, const std::function<Progress(const Info&)>& f) const;

    static const Scope* findOuterScope(const Scope * scope);

    static std::pair<bool, bool> checkCond(const Token * tok, bool& known);
};

/**
 * @brief Returns true if there is a path between the two tokens
 *
 * @param start Starting point of the path
 * @param dest The path destination
 * @param errorPath Adds the path traversal to the errorPath
 */
bool reaches(const Token * start, const Token * dest, const Library& library, ErrorPath* errorPath);

#endif

