set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/third_party/libdbus-c++-chromium/")

find_package(PkgConfig)

pkg_check_modules(PC_DBUS_CPP dbus-c++-1=0.5.0)

# this is not correct the check of chromium version but it should work...
if (NOT PC_DBUS_CPP_VERSION VERSION_EQUAL "0.5.0")
    message("Proper version of libdbusc++ is not found")
    message("Try to set PKG_CONFIG_PATH where you libdbus-c++-chromium is installed")
    message("Or install it here ${CMAKE_SOURCE_DIR}/third_party/libdbus-c++-chromium/")
    message("Using:")
    message("  git clone https://chromium.googlesource.com/chromiumos/third_party/dbus-cplusplus/ -b release-R75-12105.B --single-branch")
    message("  cd dbus-cplusplus/")
    message("  ./bootstrap && ./configure --prefix='${CMAKE_BINARY_DIR}/third_party/libdbus-c++-chromium/' --enable-shared --disable-static --disable-glib --disable-ecore")
    message("  make install")
    message(FATAL_ERROR "")
endif ()

set(DBUS_CPP_INSTALL_DIR ${PC_DBUS_CPP_PREFIX})

if (PC_DBUS_CPP_LIBRARIES)
    set(DBusCpp_LIBRARIES ${PC_DBUS_CPP_LIBRARIES})
else ()
    find_library(DBusCpp_LIBRARIES
            NAMES dbus-c++-1
            HINTS ${PC_DBUS_CPP_LIBDIR}
            ${PC_DBUS_CPP_LIBRARY_DIRS}
            )
endif ()

if (PC_DBUS_CPP_INCLUDE_DIRS)
    set(DBusCpp_INCLUDE_DIRS ${PC_DBUS_CPP_INCLUDE_DIRS})
else ()
    find_path(DBusCpp_INCLUDE_DIRS
            NAMES dbus-c++/dbus.h
            HINTS ${PC_DBUS_CPP_INCLUDEDIR}
            ${PC_DBUS_CPP_INCLUDE_DIRS}
            )
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBusCpp
        REQUIRED_VARS DBusCpp_INCLUDE_DIRS DBusCpp_LIBRARIES
        )

if (DBusCpp_FOUND AND NOT TARGET DBusCpp::DBusCpp)
    add_library(DBusCpp::DBusCpp INTERFACE IMPORTED)
    set_target_properties(DBusCpp::DBusCpp PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${DBusCpp_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${DBusCpp_LIBRARIES}"
            )
    if (PC_DBUS_CPP_CFLAGS)
        set_target_properties(DBusCpp::DBusCpp PROPERTIES
                INTERFACE_COMPILE_OPTIONS "${PC_DBUS_CPP_CFLAGS}"
                )
    endif ()

    if (PC_DBUS_CPP_LDFLAGS)
        set_target_properties(DBusCpp::DBusCpp PROPERTIES
                INTERFACE_LINK_OPTIONS "${PC_DBUS_CPP_LDFLAGS}"
                )
    endif ()

    find_program(DBusCpp_XML2CPP
            NAMES dbusxx-xml2cpp
            HINTS ${PC_DBUS_CPP_PREFIX}/bin
            )
    if (PC_DBUS_CPP_LIBDIR)
        set(DBusCpp_LIBDIR ${PC_DBUS_CPP_LIBDIR})
        set_property(TARGET DBusCpp::DBusCpp
                PROPERTY INTERFACE_LINK_DIRECTORIES ${PC_DBUS_CPP_LIBDIR}
                )
    elseif (PC_DBUS_CPP_LIBRARY_DIRS)
        set(DBusCpp_LIBDIR ${PC_DBUS_CPP_LIBRARY_DIRS})
    endif ()
endif ()