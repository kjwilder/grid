cc_library(
  name = "grid",
  srcs = ["grid.cc"],
  hdrs = ["utils.h", "grid.h"],
)

cc_test(
  name = "utils_test",
  copts = ["-Wall", "-Werror", "-Wno-absolute-value"],
  size = "small",
  srcs = ["utils.h", "tests/utils_test.cc"],
  deps = ["@com_google_googletest//:gtest_main"],
)

cc_test(
  name = "grid_test",
  copts = ["-Wall", "-Werror", "-Wno-absolute-value"],
  size = "small",
  srcs = ["utils.h", "grid.h", "tests/grid_test.cc"],
  deps = ["@com_google_googletest//:gtest_main"],
)

cc_test(
  name = "statistics_test",
  copts = ["-Wall", "-Werror", "-Wno-absolute-value"],
  size = "small",
  srcs = ["utils.h", "grid.h", "tests/statistics_test.cc"],
  deps = ["@com_google_googletest//:gtest_main"],
)

cc_test(
  name = "arithmetic_test",
  copts = ["-Wall", "-Werror", "-Wno-absolute-value"],
  size = "small",
  srcs = ["utils.h", "grid.h", "tests/arithmetic_test.cc"],
  deps = ["@com_google_googletest//:gtest_main"],
)