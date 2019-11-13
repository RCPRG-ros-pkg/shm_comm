###############################################################################
# CMakeLists.txt
#
# Contains definitions of external, 3rd-party targets
###############################################################################

###############################################################################
# doctest header-only library definition
###############################################################################

add_library(doctest INTERFACE)

target_include_directories(doctest
    INTERFACE
        doctest
)
