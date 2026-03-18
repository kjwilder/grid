#ifndef UTILS_H_
#define UTILS_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace utils_h {

#define Ifstream(v, f) std::ifstream v(f)
#define Ofstream(v, f) std::ofstream v(f)
#define Open(f) open(f)
#define Getline(b, s) getline(b, s)

// __________________________________________________________________________
// (Somewhat) platform independent reads and writes.

#ifdef INTEL
#define varwrite(s, v) \
{ for (int vw_i = sizeof(v) - 1; vw_i >= 0; --vw_i) \
  s.write(((char * const) &v) + vw_i, 1); }
#define varread(s, v) \
{ for (int vr_i = sizeof(v) - 1; vr_i >= 0; --vr_i) \
  s.read(((char * const) &v) + vr_i, 1); }
#else
#define varwrite(s, v) s.write((char * const) &v, sizeof(v));
#define varread(s, v) s.read((char * const) &v, sizeof(v));
#endif

#define arraywrite(s, v, n) \
{ for (int aw_i = 0; aw_i < n; ++aw_i) varwrite(s, (v[aw_i])) }
#define arrayread(s, v, n) \
{ for (int ar_i = 0; ar_i < n; ++ar_i) varread(s, (v[ar_i])) }

// __________________________________________________________________________

template <class T>
inline T distance_squared(
    const T& x1, const T& y1, const T& x2, const T& y2) {
  const T xdiff = x2 - x1;
  const T ydiff = y2 - y1;
  return xdiff * xdiff + ydiff * ydiff;
}

template <class T>
inline double distance(
    const T& x1, const T& y1, const T& x2, const T& y2) {
  return sqrt(distance_squared(x1, y1, x2, y2));
}

template <class T>
inline T linfdist(
    const T& x1, const T& y1, const T& x2, const T& y2) {
  return std::max(std::abs(x2 - x1), std::abs(y2 - y1));
}

template <class T>
inline double angle(const T x1, const T y1, const T x2, const T y2) {
  return atan2(y1 - y2, x2 - x1);
}

};  // namespace utils_h

#endif  // UTILS_H_
