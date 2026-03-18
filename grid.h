#ifndef GRID_H_
#define GRID_H_

#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <fstream>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

#include "utils.h"

namespace grid_h {

template <class T>
class grid {
 private:
  size_t nr, nc;  // number of rows, columns
  std::vector<T> sto;  // column major grid storage

  // Consistency Checking Functions
  int consistent() const { return sto.size() == nr * nc; }
  int inrange(size_t r, size_t c) const { return r < nr && c < nc; }

 public:
  // Constructors, Operator=
  grid() : nr(0), nc(0) { }
  explicit grid(size_t r) : nr(r), nc(1), sto(r) { }
  grid(size_t r, size_t c) : nr(r), nc(c), sto(r * c) { }
  grid(size_t r, size_t c, const std::vector<T>& v) : nr(r), nc(c), sto(v) {
    assert(r * c == v.size()); }
  grid(const grid &m) : nr(m.rows()), nc(m.cols()), sto(m.storage()) { }
  explicit grid(const std::vector<T>& v) : nr(v.size()), nc(1), sto(v) { }

  grid& operator=(const grid &m);
  bool operator==(const grid& m) const {
    return nr == m.nr && nc == m.nc && sto == m.sto;
  }
  bool operator==(const T& val) const {
    return std::any_of(begin(), end(), [val](const T& el){ return el == val; });
  }
  void subgrid(
      grid* m, size_t r, size_t c, size_t numrows, size_t numcols) const;
  typename std::vector<T>::iterator begin() { return sto.begin(); }
  typename std::vector<T>::iterator end() { return sto.end(); }
  typename std::vector<T>::const_iterator begin() const { return sto.begin(); }
  typename std::vector<T>::const_iterator end() const { return sto.end(); }

  // Basic Member Access Functions
  size_t rows() const { return nr; }
  size_t cols() const { return nc; }
  const std::vector<T>& storage() const { return sto; }

  const T& operator()(size_t r) const {
    assert(inrange(r, 0));
    return sto[r];
  }
  T& operator()(size_t r) {
    assert(inrange(r, 0));
    return sto[r];
  }
  const T& operator()(size_t r, size_t c) const {
    assert(inrange(r, c));
    return sto[c * nr + r];
  }
  T& operator()(size_t r, size_t c) {
    assert(inrange(r, c));
    return sto[c * nr + r];
  }
  T& operator[](size_t r) { return sto[r]; }
  const T& operator[](size_t r) const { return sto[r]; }
  void set(size_t r, size_t c, T val) {
    if (inrange(r, c)) {
      (*this)(r, c) = val;
    }
  }
  T get(size_t r, size_t c) const { return inrange(r, c) ? (*this)(r, c) : 0; }

  void resize(size_t r, size_t c) { nr = r; nc = c; sto.resize(r * c); }
  void resize(size_t r) { resize(r, 1); }
  void resize() { resize(0, 0); }
  grid& fill(const T& val) { std::fill(begin(), end(), val); return *this; }
  grid& clear() { return fill(0); }

  // I/O Functions
  void write(const char *file);
  int write(std::ofstream& os);
  int read(const char* file);
  int read(std::ifstream& is);
  int loadpgm(const std::string& pgmname);
  int savepgm(const std::string& pgmname);
  size_t size() const;
  void dump(size_t max = 0, bool invert = false) const;

  // Useful Utility Functions
  grid& operator<<(grid &m);
  grid& operator+=(const grid &m);
  grid& operator-=(const grid &m);
  grid& operator+=(T val);
  grid operator+ (T val) const;
  grid& operator-=(T val) { return *this += -val;  }
  grid operator*(const grid &m) const;
  grid& scale(T val);
  grid& transform(T minval, T maxval);
  const grid transpose() const;
  grid LU() const;
  grid inverse() const;
  int offpixels() const {
    int c = 0;
    for (size_t i = 0; i < nr * nc; ++i) {
      c += (sto[i] == 0);
    }
    return c;
  }
  int onpixels() const { return(nr * nc - offpixels()); }
  void sort(size_t col);
};  // class grid

// __________________________________________________________________________
// Dump a grid - for classes with ostream<<(const T&).

template <class T>
void grid<T>::dump(size_t max, bool invert) const {
  if (invert) {
    if (max == 0) {
      max = nr;
    }
    max = std::min(nr, max);
    for (size_t i = 0; i < max; ++i) {
      for (size_t j = 0; j < nc; ++j) {
        std::cout << ((*this)(i, j)) << " ";
      }
      std::cout << "\n";
    }
  } else {
    if (max == 0) {
      max = nc;
    }
    max = std::min(nc, max);
    for (size_t j = 0; j < max; ++j) {
      for (size_t i = 0; i < nr; ++i) {
        std::cout << ((*this)(i, j)) << " ";
      }
      std::cout << "\n";
    }
  }
}

// __________________________________________________________________________
// Write out a grid to a file

template<class T>
void grid<T>::write(const char *file) {
  assert(consistent());
  Ofstream(ofs, file);
  if (!ofs) {
    std::cerr << "Unable to write a grid to file [" << file << "]."
      << std::endl;
    return;
  }

  ofs.write("GR11", 4);
  varwrite(ofs, nr);
  varwrite(ofs, nc);
  arraywrite(ofs, sto, nr * nc);
}

// __________________________________________________________________________
// Write out a grid to an open output stream

template<class T>
int grid<T>::write(std::ofstream& ofs) {
  assert(consistent());

  // if (!ofs.is_open())
  if (!ofs)
    return 0;

  varwrite(ofs, nr);
  varwrite(ofs, nc);
  arraywrite(ofs, sto, nr * nc);

  return 1;
}

// __________________________________________________________________________
// Calculate the storage size of a grid to be written out

template<class T>
size_t grid<T>::size() const {
  return sizeof(nr) + sizeof(nc) + nr * nc * sizeof(T);
}

// __________________________________________________________________________
// Read in a grid which was previously written to a file

template<class T>
int grid<T>::read(const char *file) {
  assert(consistent());
  Ifstream(ifs, file);
  if (!ifs)
    return 0;

  char version[4];
  ifs.read(version, 4);
  if (memcmp(version, "GR11", 4) != 0 && memcmp(version, "GR12", 4) != 0) {
    if (loadpgm(file)) {
      return 1;
    } else {
      std::cerr << "The file [" << file << "] is not a grid or pgm file"
        << std::endl;
      return 0;
    }
  }

  resize(0, 0);
  if (!memcmp(version, "GR11", 4)) {
    varread(ifs, nr);
    varread(ifs, nc);
    if (nr > 0 && nc > 0) {
      sto.resize(nr * nc);
      arrayread(ifs, sto, nr * nc);
    }
  } else if (!memcmp(version, "GR12", 4)) {
    ifs >> nr;
    ifs >> nc;
    if (nr > 0 && nc > 0) {
      sto.resize(nr * nc);
      for (size_t j = 0; j < nc; ++j) {
        for (size_t i = 0; i < nr; ++i) {
          ifs >> (*this)(i, j);
        }
      }
    }
  }
  assert(consistent());

  return 1;
}

// __________________________________________________________________________
// Read in a grid from an open istream

template<class T>
int grid<T>::read(std::ifstream& is) {
  assert(consistent());

  // if (!is.is_open() || is.eof())
  if (!is || is.eof())
    return 0;

  resize(0, 0);
  varread(is, nr);
  varread(is, nc);
  if (nr > 0 && nc > 0) {
    sto.resize(nr * nc);
    arrayread(is, sto, nr * nc);
  }
  assert(consistent());
  return 1;
}

// __________________________________________________________________________
// Set a grid equal to a grid 'm'; leave 'm' unchanged.

template<class T>
grid<T>& grid<T>::operator=(const grid& m) {
  if (this != &m) {
    nr = m.nr;
    nc = m.nc;
    sto = m.storage();
  }
  return *this;
}

// __________________________________________________________________________
// Set '*m' equal to a subgrid of *this.

template<class T>
void grid<T>::subgrid(
    grid* m, size_t r, size_t c, size_t numrows, size_t numcols) const {
  assert(r + numrows <= nr && c + numcols <= nc);

  if (this != m) {
    m->resize(numrows, numcols);
    if (nr > 0 && nc > 0) {
      for (size_t i = 0; i < m->nr; ++i) {
        for (size_t j = 0; j < m->nc; ++j) {
          (*m)(i, j) = (*this)(r + i, c + j);
        }
      }
    }
  } else {
    grid tmp;
    subgrid(&tmp, r, c, numrows, numcols);
    *m << tmp;
  }
}

// __________________________________________________________________________
// Set a grid equal to 'm'; obliterate 'm'.

template<class T>
grid<T>& grid<T>::operator<<(grid &m) {
  if (this != &m) {
    nr = m.nr;
    nc = m.nc;
    std::swap(sto, m.sto);
    m.nr = 0;
    m.nc = 0;
    m.sto.clear();
  }
  return *this;
}

// __________________________________________________________________________
// Add another grid to the current grid.

template<class T>
grid<T>& grid<T>::operator+=(const grid& m) {
  assert(nr == m.nr && nc == m.nc);
  std::transform(begin(), end(), m.sto.begin(), begin(), std::plus<T>());
  return *this;
}

// __________________________________________________________________________
// Subtract a grid from the current grid.

template<class T>
grid<T>& grid<T>::operator-=(const grid& m) {
  assert(nr == m.nr && nc == m.nc);
  std::transform(begin(), end(), m.sto.begin(), begin(), std::minus<T>());
  return *this;
}

// __________________________________________________________________________
// Add a fixed value to each element of a grid

template<class T>
grid<T>& grid<T>::operator+=(T val) {
  for (auto& el : sto) { el += val; }
  return *this;
}

// __________________________________________________________________________
// Add a fixed value to each element of a grid

template<class T>
grid<T> grid<T>::operator+(T val) const {
  return grid<T>(*this) += val;
}

// __________________________________________________________________________
// Scale the values of a grid so that the largest magnitude is 'val'.

template<class T>
grid<T>& grid<T>::scale(T val) {
  if (nr > 0 && nc > 0) {
    const T gmin = *std::min_element(begin(), end());
    const T gmax = *std::max_element(begin(), end());
    const T absmax = std::max(std::abs(gmin), std::abs(gmax));
    if (absmax != 0) {
      for (auto& el : *this) { el = el * val / absmax; }
    }
  }
  return *this;
}

// __________________________________________________________________________
// Linearly transform the values of a grid so that the values range
// from 'val1' to 'val2'.

template<class T>
grid<T>& grid<T>::transform(T val1, T val2) {
  if (nr == 0 || nc == 0)
    return *this;
  const T gmin = *std::min_element(begin(), end());
  const T gmax = *std::max_element(begin(), end());
  const T oldrange = gmax - gmin;
  const T newrange = val2 - val1;
  if (oldrange != 0) {
    for (auto& el : *this) {
      el = (el - gmin) * newrange / oldrange + val1;
    }
  } else if (gmin < val1) {
    fill(val1);
  } else if (gmax > val2) {
    fill(val2);
  }
  return *this;
}

// __________________________________________________________________________
// Multiply two grids together using matrix multiplication.

template<class T>
grid<T> grid<T>::operator*(const grid &m) const {
  grid<T> tmp(nr, m.nc);
  assert(nc == m.nr);
  tmp.clear();
  for (size_t i = 0; i < nr; ++i)
    for (size_t j = 0; j < m.nc; ++j)
      for (size_t k = 0; k < nc; ++k)
        tmp(i, j) += (*this)(i, k) * m(k, j);
  return tmp;
}

// __________________________________________________________________________
// Determine the LU decomposition of a square grid.

template<class T>
grid<T> grid<T>::LU() const {
  grid<T> tmp = *this;
  assert(nr == nc);
  if (nr > 0) {
    for (size_t i = 0; i < nr - 1; ++i) {
      for (size_t j = i + 1; j < nr; ++j)
        tmp(j, i) /= tmp(i, i);
      for (size_t j = i + 1; j < nr; ++j)
        for (size_t k = i + 1; k < nr; ++k)
          tmp(j, k) -= tmp(j, i) * tmp(i, k);
    }
  }
  return tmp;
}

// __________________________________________________________________________
// Calculate the matrix inverse of a grid.

template<class T>
grid<T> grid<T>::inverse() const {
  grid<T> tmp = *this;
  assert(nr == nc);
  grid<int> p(nr);
  size_t i, j, k;
  for (j = 0; j < nr; ++j)
    p(j) = j;
  grid<double> hv(nr);
  hv.clear();

  for (j = 0; j < nr; ++j) {
    T max = fabs(tmp(j, j));
    size_t r = j;
    for (i = j + 1; i < nr; ++i) {
      if (fabs(tmp(i, j)) > max) {
        max = fabs(tmp(i, j));
        r = i;
      }
    }
    if (max == 0.0) {
      throw std::runtime_error("Unable to invert a singular matrix");
    }
    if (r > j) {
      for (k = 0; k < nr; ++k)
        std::swap(tmp(j, k), tmp(r, k));
      std::swap(p(j), p(r));
    }
    T hr = 1 / tmp(j, j);
    for (i = 0; i < nr; ++i)
      tmp(i, j) *= hr;
    tmp(j, j) = hr;
    for (k = 0; k < nr; ++k) {
      if (k == j)
        continue;
      for (i = 0; i < nr; ++i) {
        if (i == j)
          continue;
        tmp(i, k) -= tmp(i, j) * tmp(j, k);
      }
      tmp(j, k) *= (-hr);
    }
  }
  for (i = 0; i < nr; ++i) {
    for (k = 0; k < nr; ++k)
      hv(p(k)) = tmp(i, k);
    for (k = 0; k < nr; ++k)
      tmp(i, k) = hv(k);
  }
  return tmp;
}

// __________________________________________________________________________

template<class T>
const grid<T> grid<T>::transpose() const {
  grid<T> tp(nc, nr);
  for (size_t i = 0; i < nc; ++i)
    for (size_t j = 0; j < nr; ++j)
      tp(i, j) = (*this)(j, i);
  return tp;
}

// __________________________________________________________________________
// Reorder all columns using the permutation that sorts a specified column.

template<class T>
void grid<T>::sort(size_t col) {
  if (storage().size() == 0) {
    return;
  }
  auto col_start = begin() + col * rows();
  std::vector<size_t> permutation(rows());
  std::iota(permutation.begin(), permutation.end(), 0);
  std::sort(permutation.begin(), permutation.end(),
      [col_start](size_t i, size_t j) {
        return *(col_start + i) < *(col_start + j); });
  std::vector<T> tmp(rows());
  auto pos = begin();
  for (size_t j = 0; j < cols(); ++j) {
    std::copy(pos, pos + rows(), tmp.begin());
    for (auto ip = permutation.begin(); ip != permutation.end(); ++ip) {
      *(pos++) = tmp[*ip];
    }
  }
}

// __________________________________________________________________________

template <class T>
int grid<T>::loadpgm(const std::string& pgmname) {
  Ifstream(ifs, pgmname);
  if (!ifs) {
    return 0;
  }

  // Find the first line that doesn't begin with white space.
  char buf[255];
  char pchar;
  int mode, r, c, maxval, matches;
  do {
    ifs.Getline(buf, 255);
  } while (!ifs.eof() && (buf[0] == 0 || buf[0] == ' '));
  if (ifs.eof()) {
    return 0;
  }

  // Make sure the file is a pgm file.  Determine the pgm mode, the
  // dimensions, and the range of pixel values
  matches = sscanf(buf, "%c%1d%d%d%d", &pchar, &mode, &r, &c, &matches);
  if (matches < 2 || mode < 2 || mode > 6) {
    return 0;
  }

  if (matches < 5) {
    ifs.Getline(buf, 255);
    while (!ifs.eof() && (buf[0] == '#' || buf[0] == 0)) {
      ifs.Getline(buf, 255);
    }
    sscanf(buf, "%d %d", &r, &c);
    ifs.Getline(buf, 255);
    sscanf(buf, "%d", &maxval);
    if (ifs.eof() || r <= 0 || c <= 0 || maxval <= 0) {
      return 0;
    }
  }

  resize(r, c);
  int i, j;
  switch (mode) {
    case 2:
      for (j = 0; j < c; ++j) {
        int tmpi;
        for (i = 0; i < r; ++i) {
          ifs >> tmpi;
          (*this)(i, j) = tmpi;
        }
      }
      break;

    case 3:
      for (j = 0; j < c; j++) {
        int tmpi[3];
        for (i = 0; i < r; ++i) {
          ifs >> tmpi[0] >> tmpi[1] >> tmpi[2];
          (*this)(i, j) = static_cast<unsigned char>(
              0.212671 * tmpi[0] + 0.715160 * tmpi[1] + 0.072169 * tmpi[2]);
        }
      }
    break;

    case 5:
      for (j = 0; j < c; ++j) {
        for (i = 0; i < r; ++i) {
          char c;
          ifs.read(&c, 1);
          (*this)(i, j) = c;
        }
      }
      break;

    case 6:
      for (j=0; j < c; j++) {
        std::vector<char> tmpc(3);
        for (i = 0; i < r; ++i) {
          ifs.read(&tmpc[0], 3);
          (*this)(i, j) = static_cast<unsigned char>(
              0.212671 * static_cast<unsigned char>(tmpc[0]) +
              0.715160 * static_cast<unsigned char>(tmpc[1]) +
              0.072169 * static_cast<unsigned char>(tmpc[2]));
        }
      }
      break;

    default:
      return 0;
      break;
  }
  return 1;
}

// __________________________________________________________________________

template <class T>
int grid<T>::savepgm(const std::string& pgmname) {
  Ofstream(ofs, pgmname);
  if (!ofs) {
    return 0;
  }
  ofs << "P5\n" << nr << " " << nc << "\n255\n";
  for (size_t j = 0; j < nc; ++j) {
    for (size_t i = 0; i < nr; ++i) {
      char c = static_cast<char>((*this)(i, j));
      ofs.write(&c, 1);
    }
  }
  return 1;
}

// __________________________________________________________________________
//
template<class T>
const grid<T> operator+(const grid<T>& m, const grid<T>& n) {
  auto p = m; return p += n;
}

template<class T>
const grid<T> operator-(const grid<T>& m, const grid<T>& n) {
  auto p = m; return p -= n;
}

};  // namespace grid_h

#endif  // GRID_H_
