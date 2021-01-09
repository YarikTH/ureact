add_library(UreactDoctest INTERFACE)
target_include_directories(UreactDoctest SYSTEM INTERFACE include)
add_library(ureact::doctest ALIAS UreactDoctest)
