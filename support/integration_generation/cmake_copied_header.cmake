# We copied ureact into root folder, so here its path
set(UREACT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Create ureact::ureact interface target
add_library(ureact_ureact INTERFACE)
add_library(ureact::ureact ALIAS ureact_ureact)
target_include_directories(ureact_ureact INTERFACE ${UREACT_INCLUDE_DIR})
target_compile_features(ureact_ureact INTERFACE cxx_std_11)
