/*
    File: hub_float_binding.cpp
    Python bindings for the hub_float class using pybind11.
*/

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "../src/hub_float.hpp"

namespace py = pybind11;

PYBIND11_MODULE(hub_float, m) {
    m.doc() = "Python bindings for hub_float - a custom floating-point implementation";
    
    // First bind the BitFields struct
    py::class_<hub_float::BitFields>(m, "BitFields")
        .def_readwrite("sign", &hub_float::BitFields::sign)
        .def_readwrite("custom_exp", &hub_float::BitFields::custom_exp)
        .def_readwrite("fraction", &hub_float::BitFields::fraction)
        .def_readwrite("custom_frac", &hub_float::BitFields::custom_frac)
        .def_readwrite("custom_frac_with_hub", &hub_float::BitFields::custom_frac_with_hub);
    
    py::class_<hub_float>(m, "HubFloat")
        // Constructors
        .def(py::init<>(), "Default constructor (initializes to zero)")
        .def(py::init<float>(), "Construct from float")
        .def(py::init<double>(), "Construct from double")
        .def(py::init<int>(), "Construct from int")
        .def(py::init<uint32_t>(), "Construct from raw binary representation")
        
        // Conversion to Python types
        .def("__float__", [](const hub_float& self) { return static_cast<double>(self); })
        .def("to_double", [](const hub_float& self) { return static_cast<double>(self); })
        
        // Arithmetic operators
        .def(py::self + py::self, "Addition")
        .def(py::self - py::self, "Subtraction")
        .def(py::self * py::self, "Multiplication")
        .def(py::self / py::self, "Division")
        .def(py::self += py::self, "In-place addition")
        .def(py::self -= py::self, "In-place subtraction")
        .def(py::self *= py::self, "In-place multiplication")
        .def(py::self /= py::self, "In-place division")
        
        // Mixed operations with Python numbers
        .def("__add__", [](const hub_float& self, double other) { return self + hub_float(other); })
        .def("__radd__", [](const hub_float& self, double other) { return hub_float(other) + self; })
        .def("__sub__", [](const hub_float& self, double other) { return self - hub_float(other); })
        .def("__rsub__", [](const hub_float& self, double other) { return hub_float(other) - self; })
        .def("__mul__", [](const hub_float& self, double other) { return self * hub_float(other); })
        .def("__rmul__", [](const hub_float& self, double other) { return hub_float(other) * self; })
        .def("__truediv__", [](const hub_float& self, double other) { return self / hub_float(other); })
        .def("__rtruediv__", [](const hub_float& self, double other) { return hub_float(other) / self; })
        
        // Comparison operators (using double conversion)
        .def("__eq__", [](const hub_float& self, const hub_float& other) { 
            return static_cast<double>(self) == static_cast<double>(other); 
        })
        .def("__ne__", [](const hub_float& self, const hub_float& other) { 
            return static_cast<double>(self) != static_cast<double>(other); 
        })
        .def("__lt__", [](const hub_float& self, const hub_float& other) { 
            return static_cast<double>(self) < static_cast<double>(other); 
        })
        .def("__le__", [](const hub_float& self, const hub_float& other) { 
            return static_cast<double>(self) <= static_cast<double>(other); 
        })
        .def("__gt__", [](const hub_float& self, const hub_float& other) { 
            return static_cast<double>(self) > static_cast<double>(other); 
        })
        .def("__ge__", [](const hub_float& self, const hub_float& other) { 
            return static_cast<double>(self) >= static_cast<double>(other); 
        })
        
        // String representation
        .def("__repr__", [](const hub_float& self) {
            return "HubFloat(" + std::to_string(static_cast<double>(self)) + ")";
        })
        .def("__str__", [](const hub_float& self) {
            return std::to_string(static_cast<double>(self));
        })
        
        // Utility methods
        .def("extract_bit_fields", &hub_float::extractBitFields, "Extract bit fields")
        .def("to_binary_string", &hub_float::toBinaryString, "Get binary string representation")
        .def("to_hex_string", &hub_float::toHexString, "Get hexadecimal string representation");
        
    // Module-level functions
    m.def("sqrt", [](const hub_float& x) { return sqrt(x); }, "Square root of hub_float");
    m.def("fma", [](const hub_float& a, const hub_float& b, const hub_float& c) { 
        return fma(a, b, c); 
    }, "Fused multiply-add: (a*b + c)");
        
    // Module-level constants
    m.attr("EXP_BITS") = EXP_BITS;
    m.attr("MANT_BITS") = MANT_BITS;
}
