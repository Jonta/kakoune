#ifndef completion_hh_INCLUDED
#define completion_hh_INCLUDED

#include <functional>
#include <algorithm>

#include "units.hh"
#include "string.hh"
#include "vector.hh"
#include "ranked_match.hh"

namespace Kakoune
{

class Context;

using CandidateList = Vector<String>;

struct Completions
{
    CandidateList candidates;
    ByteCount start;
    ByteCount end;

    Completions()
        : start(0), end(0) {}

    Completions(ByteCount start, ByteCount end)
        : start(start), end(end) {}

    Completions(ByteCount start, ByteCount end, CandidateList candidates)
        : start(start), end(end), candidates(std::move(candidates)) {}
};

enum class CompletionFlags
{
    None,
    Fast
};
using Completer = std::function<Completions (const Context&, CompletionFlags,
                                             StringView, ByteCount)>;

inline Completions complete_nothing(const Context& context, CompletionFlags,
                                    StringView, ByteCount cursor_pos)
{
    return Completions(cursor_pos, cursor_pos);
}

Completions shell_complete(const Context& context, CompletionFlags,
                           StringView, ByteCount cursor_pos);

inline Completions offset_pos(Completions completion, ByteCount offset)
{
    return { completion.start + offset, completion.end + offset,
             std::move(completion.candidates) };
}

template<typename Container>
CandidateList complete(StringView prefix, ByteCount cursor_pos,
                       const Container& container)
{
    using std::begin;
    static_assert(not std::is_same<decltype(*begin(container)), String>::value,
                  "complete require long lived strings");

    prefix = prefix.substr(0, cursor_pos);
    Vector<RankedMatch> matches;
    for (const auto& str : container)
    {
        if (RankedMatch match{str, prefix})
            matches.push_back(match);
    }
    std::sort(matches.begin(), matches.end());
    CandidateList res;
    for (auto& m : matches)
        res.push_back(m.candidate().str());
    return res;
}

}
#endif // completion_hh_INCLUDED
