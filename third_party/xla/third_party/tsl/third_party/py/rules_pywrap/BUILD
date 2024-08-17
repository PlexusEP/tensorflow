load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

# copybara:uncomment package(default_applicable_licenses = ["//tensorflow:license"])

exports_files(["pybind_extension.py.tpl"])

bzl_library(
    name = "pywrap_bzl",
    srcs = [
        "pywrap.bzl",
        "pywrap.google.bzl",
        "pywrap.impl.bzl",
    ],
    parse_tests = False,
    visibility = ["//visibility:public"],
)
