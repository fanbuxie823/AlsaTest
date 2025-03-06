cmake_minimum_required(VERSION 3.22)
project(alsaTest)

set(CMAKE_CXX_STANDARD 17)


add_executable(alsaTest main.cc AlsaWrapper.cc
        SoxResample.cc
        Recorder.cc
        )
target_include_directories(alsaTest PUBLIC
        .
        ${SOXR_INCLUDE_DIRS}
)
target_link_libraries(alsaTest PUBLIC
        asound
        soxr
)
