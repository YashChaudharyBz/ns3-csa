check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

build_lib(
    LIBNAME csa
    SOURCE_FILES model/csa-buffer.cc
                 model/csa-channel.cc
                 model/csa-header.cc
                 model/csa-mac.cc
                 model/csa-net-device.cc
                 model/csa-phy.cc
                 model/csa-solver.cc
                 model/csa-tag.cc
                 helper/csa-helper.cc
    HEADER_FILES model/csa-buffer.h
                 model/csa-channel.h
                 model/csa-header.h
                 model/csa-mac.h
                 model/csa-net-device.h
                 model/csa-phy.h
                 model/csa-solver.h
                 model/csa-tag.h
                 helper/csa-helper.h
    LIBRARIES_TO_LINK ${libcore}
)

