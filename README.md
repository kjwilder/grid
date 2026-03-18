# cppgrid

A templated C++ 2-dimensional grid (matrix) class with specializations for
unsigned chars for image processing.

## Using this library

The library is header-only. Copy `grid.h` and `utils.h` into your project, or
depend on a tagged release using one of the methods below.

### Bazel

In your `MODULE.bazel`:

```python
http_archive = use_repo_rule("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "cppgrid",
    url = "https://github.com/kjwilder/grid/archive/refs/tags/v1.0.0.tar.gz",
    strip_prefix = "grid-1.0.0",
    sha256 = "<sha256 of the archive>",
)
```

Then in your `BUILD` file:

```python
cc_binary(
    name = "myapp",
    srcs = ["myapp.cc"],
    deps = ["@cppgrid//:grid"],
)
```

Replace `v1.0.0` and the `sha256` with the values from the
[release page](https://github.com/kjwilder/grid/releases). The sha256 is shown
in the release notes, or you can compute it with:

```sh
curl -sL https://github.com/kjwilder/grid/archive/refs/tags/v1.0.0.tar.gz | sha256sum
```

### Git submodule

```sh
git submodule add https://github.com/kjwilder/grid.git third_party/grid
git submodule update --init
```

Then include the headers from `third_party/grid/`.

## Creating a release

Releases are created by pushing a version tag to `main` after merging your
changes. The typical workflow is:

1. Do your work in a feature branch and open a PR as usual.
2. After the PR is merged, switch to `main` and pull:
   ```sh
   git checkout main && git pull
   ```
3. Tag the commit and push the tag:
   ```sh
   git tag v1.2.3
   git push origin v1.2.3
   ```

The release workflow will run the tests and, if they pass, publish a GitHub
release with auto-generated release notes.

Use [semantic versioning](https://semver.org): `vMAJOR.MINOR.PATCH`.
