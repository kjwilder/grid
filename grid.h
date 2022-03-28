#ifndef GRID_H_
#define GRID_H_

#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <cassert>
#include <fstream>
#include <utility>
#include <vector>

#include "utils.h"

template <class T>
class grid {
 private:
  size_t nr, nc;  // number of rows, columns
  std::vector<T> sto;

  // Initialization Member Functions
  void initnew(size_t r, size_t c) {
    nr = r;
    nc = c;
    sto.resize(r * c);
  }
  void freegrid() {
    initnew(0, 0);
  }

  // Consistency Checking Functions
  int inrange(size_t r, size_t c) const {
    return r < nr && c < nc && sto.size() > 0;
  }
  int invariant() const {
    return sto.size() == nr * nc;
  }

 public:
  // Constructors, Destructor, Operator=
  grid() : nr(0), nc(0) { }
  explicit grid(int r) : nr(r), nc(1), sto(r) { }
  grid(int r, int c) : nr(r), nc(c), sto(r * c) { }
  grid(const grid &m) : nr(m.rows()), nc(m.cols()), sto(m.storage()) { }
  grid<T>& operator=(const grid &m);
  void subgrid(grid* m, int r, int c, int numrows, int numcols) const;

  // Basic Member Access Functions
  int rows() const { return nr; }
  int cols() const { return nc; }
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

  // Initialization Member Functions
  void init() { init(0, 0); }
  void init(size_t r) { init(r, 1); }
  void init(size_t r, size_t c) {
    if (r > 0 && c > 0 && r * c <= nr * nc && r * c * 2 >= nr * nc) {
      nr = r;
      nc = c;
    } else {
      freegrid();
      initnew(r, c);
    }
  }
  void clear(const T& val = 0) {
    std::fill(sto.begin(), sto.end(), val);
  }

  // I/O Functions
  void write(const char *file);
  int write(std::ofstream& os);
  int read(const char* file);
  int read(std::ifstream& is);
  int loadpgm(const char* pgmname);
  int savepgm(const char* pgmname);
  int size() const;
  void dump(int max = -1) const;
  void dump2(int max = -1) const;

  // Useful Utility Functions
  grid<T>& operator<<(grid &m);
  grid<T>& operator+=(const grid &m);
  grid<T>& operator-=(const grid &m);
  grid<T>& operator+=(T i);
  grid<T>& operator-=(T i) { return *this += -i;  }
  grid<T> operator*(const grid &m) const;
  void scale(T val);
  void transform(T minval, T maxval);
  const grid<T> transpose() const;
  grid<T> LU() const;
  grid<T> inverse() const;
  int offpixels() {
    int c = 0;
    for (int i = 0; i < nr * nc; ++i) {
      c += (sto[i] == 0);
      return c;
    }
  }
  int onpixels() { return(nr * nc - offpixels()); }
  void sort(int col = 0, int left = -1, int right = -1);
};  // class grid

// __________________________________________________________________________
// Dump a grid; Will only work for classes with ostream<<(const T&).

template <class T>
void grid<T>::dump(int max) const {
  if (max == -1)
    max = nc;
  for (auto j = 0; j < max; ++j) {
    for (auto i = 0; i < nr; ++i) {
      std::cout << ((*this)(i, j)) << " ";
    }
    std::cout << "\n";
  }
}

// __________________________________________________________________________
// Dump a grid; Will only work for classes with ostream<<(const T&).

template <class T>
void grid<T>::dump2(int max) const {
  if (max == -1)
    max = nr;
  for (auto i = 0; i < max; ++i) {
    for (auto j = 0; j < nc; ++j) {
      std::cout << ((*this)(i, j)) << " ";
    }
    std::cout << "\n";
  }
}

// __________________________________________________________________________
// Write out a grid to a file

template<class T>
void grid<T>::write(const char *file) {
  assert(invariant());
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
  assert(invariant());

  // if (!ofs.is_open())
  if (!ofs)
    return 0;

  varwrite(ofs, nr);
  varwrite(ofs, nc);
  arraywrite(ofs, sto, nr * nc);

  return 1;
}

// __________________________________________________________________________
// Calculate the size of a grid which is to be written out

template<class T>
int grid<T>::size() const {
  assert(invariant());
  return (sizeof(nr) + sizeof(nc) + nr * nc * sizeof(T));
}

// __________________________________________________________________________
// Read in a grid which was previously written to a file

template<class T>
int grid<T>::read(const char *file) {
  assert(invariant());
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

  freegrid();
  if (!memcmp(version, "GR11", 4)) {
    varread(ifs, nr);
    varread(ifs, nc);
    assert(nr >= 0 && nc >= 0);
    if (nr > 0 && nc > 0) {
      sto.resize(nr * nc);
      arrayread(ifs, sto, nr * nc);
    }
  } else if (!memcmp(version, "GR12", 4)) {
    ifs >> nr;
    ifs >> nc;
    assert(nr >= 0 && nc >= 0);
    if (nr > 0 && nc > 0) {
      sto = new T[nr * nc];
      for (int j = 0; j < nc; ++j) {
        for (int i = 0; i < nr; ++i) {
          ifs >> (*this)(i, j);
        }
      }
    }
  }
  assert(invariant());

  return 1;
}

// __________________________________________________________________________
// Read in a grid from an open istream

template<class T>
int grid<T>::read(std::ifstream& is) {
  assert(invariant());

  // if (!is.is_open() || is.eof())
  if (!is || is.eof())
    return 0;

  freegrid();
  varread(is, nr);
  varread(is, nc);
  assert(nr >= 0 && nc >= 0);
  if (nr > 0 && nc > 0) {
    sto = new T[nr * nc];
    arrayread(is, sto, nr * nc);
  }
  assert(invariant());
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
void grid<T>::subgrid(grid* m, int r, int c, int numrows, int numcols) const {
  assert(invariant() && m->invariant());
  assert(r >= 0 && c >= 0 && numrows >= 0 && numcols >= 0);
  assert(r + numrows <= rows() && c + numcols <= cols());

  if (this != m) {
    m->init(numrows, numcols);
    if (nr > 0 && nc > 0) {
      for (int i = 0; i < m->nr; ++i) {
        for (int j = 0; j < m->nc; ++j) {
          m(i, j) = (*this)(r + i, c + j);
        }
      }
    }
  } else {
    grid tmp;
    subgrid(tmp, r, c, numrows, numcols);
    m << tmp;
  }
  assert(invariant() && m->invariant());
}

// __________________________________________________________________________
// Set a grid equal to 'm'; obliterate 'm'.

template<class T>
grid<T>& grid<T>::operator<<(grid &m) {
  assert(invariant() && m.invariant());
  if (this != &m) {
    freegrid();
    nr = m.nr; nc = m.nc; sto = m.storage();
    m.initnew(0, 0);
  }
  assert(invariant() && m.invariant());
  return *this;
}

// __________________________________________________________________________
// Add another grid to the current grid.

template<class T>
grid<T>& grid<T>::operator+=(const grid& m) {
  assert(invariant() && m.invariant());
  assert(nr == m.nr && nc == m.nc);

  for (int i = 0; i < nr * nc; ++i) {
    sto[i] += m.storage()[i];
  }

  assert(invariant() && m.invariant());
  return *this;
}

// __________________________________________________________________________
// Subtract a grid from the current grid.

template<class T>
grid<T>& grid<T>::operator-=(const grid& m) {
  assert(invariant() && m.invariant());
  assert(nr == m.nr && nc == m.nc);

  for (int i = 0; i < nr * nc; ++i) {
    sto[i] -= m.storage()[i];
  }

  assert(invariant() && m.invariant());
  return *this;
}

// __________________________________________________________________________
// Add a fixed value to each element of a grid

template<class T>
grid<T>& grid<T>::operator+=(T ii) {
  for (int i = 0; i < nr * nc; ++i) {
    sto[i] += ii;
  }
  return *this;
}

// __________________________________________________________________________
// Scale the values of a grid so that the largest equals 1.

template<class T>
void grid<T>::scale(T val) {
  if (rows() == 0 || cols() == 0)
    return;
  const T gmax = *std::max_element(sto.begin(), sto.end());
  if (gmax <= 0) {
    return;
  }
  for (int i = 0; i < nr; ++i) {
    for (int j = 0; j < nc; ++j) {
      (*this)(i, j) = (*this)(i, j) * val / gmax;
    }
  }
}

// __________________________________________________________________________
// Perform a linear transformation on the values of a grid so that the
// values range from 'val1' to 'val2'

template<class T>
void grid<T>::transform(T val1, T val2) {
  if (rows() == 0 || cols() == 0)
    return;
  const T gmin = *std::min_element(sto.begin(), sto.end());
  const T gmax = *std::max_element(sto.begin(), sto.end());
  const T range = gmax - gmin;
  const T newrange = val2 - val1;
  if (range > 0) {
    for (int i = 0; i < nr; ++i) {
      for (int j = 0; j < nc; ++j) {
        (*this)(i, j) = ((*this)(i, j) - gmin) * newrange / range + val1;
      }
    }
  } else if (gmin < val1) {
    clear(val1);
  } else if (gmax > val2) {
    clear(val2);
  }
  assert(invariant());
}

// __________________________________________________________________________
// Multiply two grids together using matrix multiplication.

template<class T>
grid<T> grid<T>::operator*(const grid &m) const {
  assert(invariant() && m.invariant());
  assert(nc == m.nr);

  grid<T> tmp(nr, m.nc);
  tmp.clear();
  for (int i = 0; i < nr; ++i)
    for (int j = 0; j < m.nc; ++j)
      for (int k = 0; k < nc; ++k)
        tmp(i, j) += (*this)(i, k) * m(k, j);

  assert(invariant() && m.invariant() && tmp.invariant());
  return tmp;
}

// __________________________________________________________________________
// Determine the LU decomposition of a square grid.

template<class T>
grid<T> grid<T>::LU() const {
  assert(invariant());
  assert(nr == nc);

  grid<T> tmp = *this;
  for (int i = 0; i < nr - 1; ++i) {
    for (int j = i + 1; j < nr; ++j)
      tmp(j, i) /= tmp(i, i);
    for (int j = i + 1; j < nr; ++j)
      for (int k = i + 1; k < nr; ++k)
        tmp(j, k) -= tmp(j, i) * tmp(i, k);
  }

  assert(invariant() && tmp.invariant());
  return tmp;
}

// __________________________________________________________________________
// Calculate the matrix inverse of a grid.

template<class T>
grid<T> grid<T>::inverse() const {
  grid<T> tmp = *this;

  assert(invariant());
  assert(nr == nc);

  grid<int> p(nr);
  int i, j, k;
  for (j = 0; j < nr; ++j)
    p(j) = j;
  grid<double> hv(nr);
  hv.clear();

  for (j = 0; j < nr; ++j) {
    T max = fabs(tmp(j, j));
    int r = j;
    for (i = j + 1; i < nr; ++i) {
      if (fabs(tmp(i, j)) > max) {
        max = fabs(tmp(i, j));
        r = i;
      }
    }
    if (max == 0.0) {
      std::cerr << "Unable to invert a matrix" << std::endl;
      exit(1);
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
  assert(invariant() && tmp.invariant());
  return tmp;
}

// __________________________________________________________________________

template<class T>
const grid<T> grid<T>::transpose() const {
  grid<T> tp(nc, nr);
  assert(invariant());
  for (int i = 0; i < nc; ++i)
    for (int j = 0; j < nr; ++j)
      tp(i, j) = (*this)(j, i);
  assert(invariant() && tp.invariant());
  return tp;
}

// __________________________________________________________________________

template<class T>
void grid<T>::sort(int col, int left, int right) {
  if (left == -1 && right == -1) {
    left = 0;
    right = nr - 1;
  }

  int i = left;
  int j = right;

  T midval = (*this)((left + right) / 2, col);

  do {
    while ((*this)(i, col) < midval && i < right) {
      ++i;
    }
    while (midval < (*this)(j, col) && j > left) {
      --j;
    }

    if (i <= j) {
      for (int k = 0; k < nc; ++k) {
        std::swap((*this)(i, k), (*this)(j, k));
      }
      ++i;
      --j;
    }
  } while (i <= j);

  if (left < j)
    sort(col, left, j);
  if (i < right)
    sort(col, i, right);
}

// __________________________________________________________________________

template <class T>
int grid<T>::loadpgm(const char* pgmname) {
  // Open the pgm file
  Ifstream(ifs, pgmname);
  if (!ifs)
    return 0;

  // Find the first line that doesn't begin with white space.
  char buf[255];
  char pchar;
  int mode, r, c, maxval, matches;
  do {
    ifs.Getline(buf, 255);
  } while (!ifs.eof() && (buf[0] == 0 || buf[0] == ' '));
  if (ifs.eof())
    return 0;

  // Make sure the file is a pgm file.  Determine the pgm mode, the
  // dimensions, and the range of pixel values
  matches = sscanf(buf, "%c%1d%d%d%d", &pchar, &mode, &r, &c, &matches);
  if (matches < 2 || mode < 2 || mode > 6)
    return 0;

  if (matches < 5) {
    ifs.Getline(buf, 255);
    while (!ifs.eof() && (buf[0] == '#' || buf[0] == 0))
    ifs.Getline(buf, 255);
    sscanf(buf, "%d %d", &r, &c);
    ifs.Getline(buf, 255);
    sscanf(buf, "%d", &maxval);
    if (ifs.eof() || r <= 0 || c <= 0 || maxval <= 0)
      return 0;
  }

  init(r, c);
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
int grid<T>::savepgm(const char* pgmname) {
  // Open the pgm file
  Ofstream(ofs, pgmname);
  if (!ofs)
    return 0;

  // Print the pgm header
  ofs << "P5\n" << nr << " " << nc << "\n255\n";

  for (int j = 0; j < nc; ++j) {
    for (int i = 0; i < nr; ++i) {
      char c = (*this)(i, j);
      ofs.write(&c, 1);
    }
  }
  return 1;
}

// __________________________________________________________________________
//
template<class T>
const grid<T> operator+(const grid<T>& m, const grid<T>& n) {
  grid<T> p = m; return (p += n);
}

template<class T>
const grid<T> operator-(const grid<T>& m, const grid<T>& n) {
  grid<T> p = m; return (p -= n);
}

typedef grid<char> cgrid;
typedef grid<unsigned char> ucgrid;
typedef grid<int> igrid;
typedef grid<unsigned int> uigrid;
typedef grid<int64_t> lgrid;
typedef grid<float> fgrid;
typedef grid<double> dgrid;
typedef std::vector<cgrid> cgrids;
typedef std::vector<ucgrid> ucgrids;
typedef std::vector<ucgrids> ucgridss;
typedef std::vector<igrid> igrids;
typedef std::vector<uigrid> uigrids;
typedef std::vector<lgrid> lgrids;
typedef std::vector<fgrid> fgrids;
typedef std::vector<dgrid> dgrids;

#endif  // GRID_H_