load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "utils_test",
    size = "small",
    srcs = [
        "tests/utils_test.cc",
        "utils.h",
    ],
    copts = [
        "-Wall",
        "-Werror",
    ],
    deps = ["@googletest//:gtest_main"],
)

cc_test(
    name = "grid_test",
    size = "small",
    srcs = [
        "grid.h",
        "tests/grid_test.cc",
        "utils.h",
    ],
    copts = [
        "-Wall",
        "-Werror",
    ],
    deps = ["@googletest//:gtest_main"],
)
