find_package(Boost)

if(Boost_FOUND)
    message(STATUS "Boost ${Boost_VERSION_STRING} is found")
else()
    message(STATUS "Boost is not found. Some examples won't be built")
endif()

add_subdirectory(xxx_glitch)
