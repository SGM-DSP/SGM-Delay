cmake_minimum_required(VERSION 3.22)

project(SGMDelay VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SGMDelay
    COMPANY_NAME "Sagami"
    BUNDLE_ID "jp.sagami.SGMDelay"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Sgmi
    PLUGIN_CODE Sgdl
    FORMATS VST3 Standalone
    PRODUCT_NAME "SGM Delay"
)

juce_generate_juce_header(SGMDelay)

target_sources(SGMDelay
    PRIVATE
        Source/DelayEngine.cpp
        Source/PluginEditor.cpp
        Source/PluginProcessor.cpp
)

target_compile_definitions(SGMDelay
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SGMDelay
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

find_program(WIX_EXE wix)
if(WIX_EXE)
    add_custom_target(installer
        COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/Installer/build-installer.ps1" -Configuration "$<CONFIG>" -ProductVersion "${PROJECT_VERSION}"
        DEPENDS SGMDelay_VST3 SGMDelay_Standalone
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building SGM Delay MSI installer with WiX"
        VERBATIM
    )
else()
    message(STATUS "WiX Toolset CLI was not found. Install WiX v4 and run Installer/build-installer.ps1 to create an MSI.")
endif()
