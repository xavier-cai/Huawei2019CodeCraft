#ifndef PMAP_H
#define PMAP_H

#include <tuple>
#include <utility>
#include <map>

namespace std {

#define NO_CXX11_LESS_FEATURE

#ifdef NO_CXX11_LESS_FEATURE

template <typename T1, typename T2>
struct pair_less
    : public binary_function<pair<T1, T2>, pair<T1, T2>, bool>
{
    bool operator() (const pair<T1, T2>& l, const pair<T1, T2>& r) const
    {
        if (l.first == r.first)
            return l.second < r.second;
        return l.first < r.first;
    }
};//struct pair_less

template <typename TV, typename T1, typename T2>
class pmap : public map< pair<T1, T2>, TV, pair_less<T1, T2> > { };

#undef NO_CXX11_LESS_FEATURE

#else

template <typename TV, typename T1, typename T2>
using pmap = map< pair<T1, T2>, TV /* , pair_less<T1, T2> */ >;

#endif

};//namespace std

#endif