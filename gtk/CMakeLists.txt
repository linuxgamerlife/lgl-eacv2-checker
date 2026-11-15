cmake_minimum_required(VERSION 3.20)
project(lgl-eacv2-checker VERSION 0.1.0 LANGUAGES C CXX)

include(GNUInstallDirs)

option(EAC_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0>=4.20)
pkg_check_modules(ADWAITA REQUIRED IMPORTED_TARGET libadwaita-1>=1.6)
find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

set(RESOURCE_XML ${CMAKE_CURRENT_SOURCE_DIR}/data/ec.gresource.xml)
set(RESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/ec-resources.c)
set(RESOURCE_DEPS
    ${CMAKE_CURRENT_SOURCE_DIR}/data/style.css
    ${CMAKE_CURRENT_SOURCE_DIR}/data/lgl-eacv2-checker.png)
add_custom_command(
    OUTPUT ${RESOURCE_C}
    COMMAND ${GLIB_COMPILE_RESOURCES} ${RESOURCE_XML}
            --target=${RESOURCE_C} --generate-source
            --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/data
    DEPENDS ${RESOURCE_XML} ${RESOURCE_DEPS}
    VERBATIM)

add_library(eac_core STATIC
    src/core/check_result.cpp
    src/core/checks.cpp
    src/core/command_runner.cpp
    src/core/file_reader.cpp
    src/core/settings.cpp
    src/core/theme.cpp)
target_include_directories(eac_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(eac_core PUBLIC PkgConfig::GTKMM)
target_compile_features(eac_core PUBLIC cxx_std_20)

add_executable(lgl-eacv2-checker
    src/main.cpp
    src/ui/check_row.cpp
    src/ui/theme_watcher.cpp
    src/ui/window.cpp
    ${RESOURCE_C})
set_source_files_properties(${RESOURCE_C} PROPERTIES COMPILE_OPTIONS "-w")
target_link_libraries(lgl-eacv2-checker PRIVATE eac_core PkgConfig::GTKMM PkgConfig::ADWAITA)
target_compile_definitions(lgl-eacv2-checker PRIVATE EAC_VERSION="${PROJECT_VERSION}")

foreach(target eac_core lgl-eacv2-checker)
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic;-Werror>)
    if(EAC_ENABLE_SANITIZERS)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PUBLIC -fsanitize=address,undefined)
    endif()
endforeach()

if(BUILD_TESTING)
    foreach(test_name checks settings theme)
        add_executable(${test_name}_test tests/${test_name}_test.cpp)
        target_link_libraries(${test_name}_test PRIVATE eac_core)
        target_compile_options(${test_name}_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
        if(EAC_ENABLE_SANITIZERS)
            target_compile_options(${test_name}_test PRIVATE
                -fsanitize=address,undefined -fno-omit-frame-pointer)
        endif()
        add_test(NAME ${test_name}_test COMMAND ${test_name}_test)
    endforeach()
    find_program(XVFB_RUN xvfb-run)
    find_program(TIMEOUT_COMMAND timeout)
    if(XVFB_RUN AND TIMEOUT_COMMAND)
        add_test(
            NAME application_stays_open
            COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/application_stays_open.sh
                    ${TIMEOUT_COMMAND} ${XVFB_RUN}
                    $<TARGET_FILE:lgl-eacv2-checker>)
    endif()
endif()

install(TARGETS lgl-eacv2-checker RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
    ${CMAKE_SOURCE_DIR}/packaging/com.linuxgamerlife.lgl-eacv2-checker.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES
    ${CMAKE_SOURCE_DIR}/packaging/com.linuxgamerlife.lgl-eacv2-checker.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
foreach(size IN ITEMS 48 64 128 256)
    install(
        FILES ${CMAKE_SOURCE_DIR}/packaging/icons/${size}x${size}/lgl-eacv2-checker.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${size}x${size}/apps)
endforeach()
install(
    FILES ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_SOURCE_DIR}/CHANGELOG.md
          ${CMAKE_SOURCE_DIR}/DESIGN.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/lgl-eacv2-checker)
