cmake_minimum_required(VERSION 3.5)

project(atomicDexPro LANGUAGES CXX VERSION 0.3.0)


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#find_program(IWYU_PATH NAMES include-what-you-use iwyu)
#if(NOT IWYU_PATH)
#    message(FATAL_ERROR "Could not find the program include-what-you-use")
#endif()
#set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
#set(CMAKE_C_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
#include_directories("/usr/lib/llvm-10/include/c++/v1/")


if (WIN32)
    find_package(ZLIB)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Override option" FORCE)
endif ()

##! Antara
##! We include the module from CMake for fetching dependencies
include(FetchContent)

##! We declare information about the dependance that we want to fetch.
FetchContent_Declare(
        antara-gaming-sdk
        URL https://github.com/KomodoPlatform/antara-gaming-sdk/archive/master.zip

)

set(ANTARA_USE_VCPKG ON)
set(ENABLE_BLOCKCHAIN_MODULES ON)

FetchContent_GetProperties(antara-gaming-sdk)
if (NOT antara-gaming-sdk_POPULATED)
    FetchContent_Populate(antara-gaming-sdk)
    add_subdirectory(${antara-gaming-sdk_SOURCE_DIR} ${antara-gaming-sdk_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX TRUE)
endif ()

if (UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

##! We fetch our dependence
if (APPLE)
    FetchContent_Declare(mm2
            URL https://github.com/KomodoPlatform/atomicDEX-API/releases/download/beta-2.0.2422/mm2-31d1803b0-Darwin-Release.zip)
elseif (UNIX AND NOT APPLE)
    FetchContent_Declare(mm2
            URL https://github.com/KomodoPlatform/atomicDEX-API/releases/download/beta-2.0.2422/mm2-31d1803b0-Linux-Release.zip)
else ()
    FetchContent_Declare(mm2
            URL https://github.com/KomodoPlatform/atomicDEX-API/releases/download/beta-2.0.2422/mm2-31d1803b0-Windows_NT-Release.zip)
endif ()

FetchContent_Declare(qmaterial URL https://github.com/KomodoPlatform/Qaterial/archive/master.zip)

FetchContent_Declare(jl777-coins
        URL https://github.com/KomodoPlatform/coins/archive/master.zip)

FetchContent_MakeAvailable(mm2 jl777-coins qmaterial)

##! Configure our needs.
if (UNIX)
    configure_file(${jl777-coins_SOURCE_DIR}/coins ${CMAKE_CURRENT_SOURCE_DIR}/assets/tools/mm2/coins COPYONLY)
    configure_file(${mm2_SOURCE_DIR}/mm2 ${CMAKE_CURRENT_SOURCE_DIR}/assets/tools/mm2/mm2 COPYONLY)
else ()
    configure_file(${jl777-coins_SOURCE_DIR}/coins ${CMAKE_BINARY_DIR}/bin/assets/tools/mm2/coins COPYONLY)
    configure_file(${mm2_SOURCE_DIR}/mm2.exe ${CMAKE_BINARY_DIR}/bin/assets/tools/mm2/mm2.exe COPYONLY)
    configure_file(${mm2_SOURCE_DIR}/msvcp140.dll ${CMAKE_BINARY_DIR}/bin/assets/tools/mm2/msvcp140.dll COPYONLY)
    configure_file(${mm2_SOURCE_DIR}/msvcr100.dll ${CMAKE_BINARY_DIR}/bin/assets/tools/mm2/msvcr100.dll COPYONLY)
    configure_file(${mm2_SOURCE_DIR}/vcruntime140.dll ${CMAKE_BINARY_DIR}/bin/assets/tools/mm2/vcruntime140.dll COPYONLY)
endif ()

FetchContent_Declare(libqr_code
        URL https://github.com/KomodoPlatform/qzxing/archive/master.zip)

FetchContent_GetProperties(libqr_code)
if (NOT libqr_code_POPULATED)
    FetchContent_Populate(libqr_code)
    add_subdirectory(${libqr_code_SOURCE_DIR}/src ${libqr_code_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

##! Calling this macros provided by the sdk will if you are on Apple init the environment for this OS (std::filesystem).
init_antara_env()

##! Get basis assets (default fonts, etc)
get_resources_basics_assets(${CMAKE_CURRENT_SOURCE_DIR})

##! Osx bundle icon
set(ICON)
configure_icon_osx(data/osx/atomicDexPro.icns ICON atomicDexPro.icns)
##! End antara

find_package(date CONFIG REQUIRED)
#find_package(httplib)
find_package(doctest CONFIG REQUIRED)
find_package(folly CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
#find_package(Taskflow CONFIG REQUIRED)
find_package(cpprestsdk CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem random)
find_package(Qt5 COMPONENTS Core Quick LinguistTools Svg Charts Widgets REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)
add_library(unofficial-bitcoin INTERFACE)
if (WIN32)
    target_link_directories(unofficial-bitcoin INTERFACE wally)
    target_link_libraries(unofficial-bitcoin INTERFACE wally)
    target_include_directories(unofficial-bitcoin INTERFACE wally)
else ()
    find_library(unofficial-secp secp256k1)
    find_library(unofficial-wally wallycore)
    find_path(unofficial-wally-headers wally_core.h)
    target_link_libraries(unofficial-bitcoin INTERFACE ${unofficial-wally} ${unofficial-secp})
    target_include_directories(unofficial-bitcoin INTERFACE ${unofficial-wally-headers})
    message(STATUS "Found wally -> ${unofficial-wally} ${unofficial-wally-headers}")
endif ()
add_library(unofficial-btc::bitcoin ALIAS unofficial-bitcoin)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(TS_FILES
        ${CMAKE_SOURCE_DIR}/atomic_defi_design/assets/languages/atomic_defi_en.ts
        ${CMAKE_SOURCE_DIR}/atomic_defi_design/assets/languages/atomic_defi_fr.ts
        ${CMAKE_SOURCE_DIR}/atomic_defi_design/assets/languages/atomic_defi_tr.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_SOURCE_DIR}/atomic_defi_design/assets/languages/")
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/atomic_defi_design/qml ${TS_FILES})
#qt5_add_translation(qmFiles ${TS_FILES})
message(STATUS "${QM_FILES}")
message(STATUS "${TS_FILES}")

##! Atomic QT Shared Deps Between main executable and unit tests
add_library(atomicDexPro_shared_deps INTERFACE)
target_sources(atomicDexPro_shared_deps INTERFACE
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.kill.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.internet.checker.service.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.ip.checker.service.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.app.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.cfg.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.mm2.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.coins.config.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.mm2.api.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.mm2.error.code.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.provider.coinpaprika.api.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.bindings.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.provider.coinpaprika.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.provider.cex.prices.api.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.provider.cex.prices.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.wallet.manager.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.utilities.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.wallet.config.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.security.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.update.service.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.notification.manager.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.orders.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.orders.proxy.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.candlestick.charts.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.addressbook.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.addressbook.proxy.filter.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.contact.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.trading.page.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.market.pairs.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.settings.page.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.orderbook.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.orderbook.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.orderbook.proxy.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.portfolio.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.portfolio.page.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.portfolio.proxy.filter.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.global.price.service.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.band.oracle.price.service.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.wallet.transactions.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.wallet.transactions.proxy.filter.model.cpp
        ${CMAKE_SOURCE_DIR}/src/atomic.dex.qt.wallet.page.cpp
        $<$<PLATFORM_ID:Darwin>:${CMAKE_SOURCE_DIR}/src/osx/atomic.dex.osx.manager.mm>
        ${CMAKE_SOURCE_DIR}/qml.qrc
        ${qmFiles}
        )

target_compile_definitions(atomicDexPro_shared_deps
        INTERFACE
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
        $<$<PLATFORM_ID:Windows>:HAS_REMOTE_API>
        $<$<PLATFORM_ID:Windows>:AUTO_DOWNLOAD>
        )

#target_compile_options(atomicDexPro_shared_deps INTERFACE -fstandalone-debug)

target_link_libraries(atomicDexPro_shared_deps
        INTERFACE
        Qt5::Core
        Qt5::Quick
        Qt5::Svg
        Qt5::Charts
        Qt5::Widgets
        qzxing
        antara::world
        Boost::filesystem
        Boost::random
        unofficial-sodium::sodium
        unofficial-btc::bitcoin
        reproc++
        antara::http_client
        Folly::folly
        date::tz
        Folly::folly_deps
        spdlog::spdlog
        spdlog::spdlog_header_only
        cpprestsdk::cpprest
        Qaterial
        #Taskflow::Taskflow
        )

target_include_directories(atomicDexPro_shared_deps
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/src/osx>
        ${libqr_code_SOURCE_DIR}/src
        )

##! Main executable
add_executable(atomicDexPro MACOSX_BUNDLE ${ICON}
        ${CMAKE_CURRENT_SOURCE_DIR}/data/windows/atomicDexPro.rc
        main.cpp
        )

##! Testing executable
add_executable(atomicDexPro_tests MACOSX_BUNDLE ${ICON}
        src/atomic.dex.tests.cpp
        src/atomic.dex.wallet.config.tests.cpp
        src/atomic.dex.utilities.tests.cpp
        src/atomic.dex.provider.cex.prices.tests.cpp
        src/atomic.dex.qt.utilities.tests.cpp
        src/atomic.dex.provider.cex.prices.api.tests.cpp)

target_link_libraries(atomicDexPro
        PRIVATE
        atomicDexPro_shared_deps
        $<$<PLATFORM_ID:Darwin>:/usr/local/lib/libcurl.a>
        )

target_link_libraries(atomicDexPro_tests
        PRIVATE
        atomicDexPro_shared_deps
        $<$<PLATFORM_ID:Darwin>:/usr/local/lib/libcurl.a>
        doctest::doctest)

##! Setting output directory
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
        )

set_target_properties(${PROJECT_NAME}_tests
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
        )


if (NOT APPLE)
    target_precompile_headers(${PROJECT_NAME} PRIVATE src/atomic.dex.pch.hpp)
endif ()
#target_enable_tsan(atomicDexPro_tests)
#target_enable_asan(atomicDexPro)

##! Move assets
if (LINUX)
    get_target_property(exe_runtime_directory_at ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY)
    set(output_dir_at "${exe_runtime_directory_at}/AntaraAtomicDexAppDir")

    magic_game_app_image_generation("${CMAKE_CURRENT_SOURCE_DIR}/data/linux"
            "atomicDexPro.desktop"
            "atomicDexPro.appdata.xml"
            "atomicDexPro_64.png"
            atomicDexPro
            AntaraAtomicDexAppDir
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            )

    magic_game_app_image_generation("${CMAKE_CURRENT_SOURCE_DIR}/data/linux"
            "atomicDexPro.desktop"
            "atomicDexPro.appdata.xml"
            "atomicDexPro_64.png"
            atomicDexPro_tests
            AntaraAtomicDexTestsAppDir
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            )

    set(from_dir "${CMAKE_CURRENT_SOURCE_DIR}/data/linux")
    set(desktop_file "atomicDexPro.desktop")
    set(appdata_file "atomicDexPro.appdata.xml")
    set(app_icon "atomicDexPro_64.png")
    message(STATUS "${output_dir_at}")
    configure_file(${from_dir}/${desktop_file} ${output_dir_at}/usr/share/applications/${desktop_file} COPYONLY)
    configure_file(${from_dir}/${appdata_file} ${output_dir_at}/usr/share/metainfo/${appdata_file} COPYONLY)
    configure_file(${from_dir}/${app_icon} ${output_dir_at}/usr/share/icons/hicolor/64x64/apps/${app_icon} COPYONLY)
    configure_file(${from_dir}/${app_icon} ${output_dir_at}/usr/share/icons/default/64x64/apps/${app_icon} COPYONLY)
endif ()

if (WIN32)
    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/bin/)
    configure_file(wally/wally.dll ${CMAKE_BINARY_DIR}/bin/ COPYONLY)
    set_target_properties(Folly::folly PROPERTIES INTERFACE_COMPILE_OPTIONS "")
    if (CMAKE_BUILD_TYPE MATCHES Release)
        message(STATUS "disabling console")
        target_compile_definitions(atomicDexPro PRIVATE WINDOWS_RELEASE_MAIN)
        target_link_options(atomicDexPro PUBLIC --for-linker -subsystem:windows)
    endif ()
endif ()

if (APPLE)
    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}.app/Contents/Resources)
    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}_tests.app/Contents/Resources)
    include(data/osx/atomicdex_install.cmake)
endif ()
