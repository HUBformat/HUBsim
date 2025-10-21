from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
import pybind11

# Define the extension module
ext_modules = [
    Pybind11Extension(
        "hub_float",
        [
            "python_bindings/hub_float_binding.cpp",
            "src/hub_float.cpp",
        ],
        include_dirs=[
            # Path to pybind11 headers
            pybind11.get_include(),
            # Path to our source directory
            "src/",
        ],
        cxx_std=17,
        define_macros=[
            ("EXP_BITS", "8"),
            ("MANT_BITS", "23"),
        ],
    ),
]

setup(
    name="hub_float",
    version="0.1.0",
    description="Python bindings for hub_float - a custom floating-point implementation",
    author="HUBformat",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.6",
)
