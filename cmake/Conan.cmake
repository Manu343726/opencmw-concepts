macro(run_conan)
    # Download automatically, you can also just copy the conan.cmake file
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.16.1/conan.cmake"
             "${CMAKE_BINARY_DIR}/conan.cmake")
    endif()

    include(${CMAKE_BINARY_DIR}/conan.cmake)

    conan_add_remote(
        NAME
        conan-center
        URL
        https://api.bintray.com/conan/conan/conan-center)

    conan_add_remote(
        NAME
        bincrafters
        URL
        https://api.bintray.com/conan/bincrafters/public-conan)

    conan_add_remote(
        NAME
        tiny_refl
        URL
        https://api.bintray.com/conan/manu343726/conan-packages)

    conan_cmake_run(
        REQUIRES
        ${CONAN_EXTRA_REQUIRES}
        catch2/2.13.3
        docopt.cpp/0.6.2
        fmt/6.2.1
        spdlog/1.5.0
        tinyrefl/0.5.4
        BUILD_REQUIRES
        tinyrefl-tool/0.5.4
        OPTIONS
        ${CONAN_EXTRA_OPTIONS}
        BASIC_SETUP
        CMAKE_TARGETS # individual targets to link to
        BUILD
        missing)
endmacro()
