# vi:set sw=2 et:
project(ramer-synth VERSION ${CMAKE_PROJECT_VERSION})

juce_add_plugin(${PROJECT_NAME}
  PRODUCT_NAME "ramer-synth"
  COMPANY_NAME "ramerdigital"
  COMPANY_WEBSITE "https://ramerdigital.com/"
  BUNDLE_ID "com.ramerdigital.ramersynth"
  PLUGIN_MANUFACTURER_CODE RmrD
  PLUGIN_CODE RmrS

  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE

  COPY_PLUGIN_AFTER_BUILD ${SURGE_COPY_AFTER_BUILD}

  VST3_CATEGORIES Instrument Synth
  VST3_AUTO_MANIFEST FALSE

  VST2_CATEGORY kPlugCategSynth

  AU_MAIN_TYPE kAudioUnitType_MusicDevice
  AU_SANDBOX_SAFE TRUE

  FORMATS ${SURGE_JUCE_FORMATS}
)

target_sources(${PROJECT_NAME} PRIVATE
  PluginProcessor.cpp
  PluginProcessor.h
  PluginEditor.cpp
  PluginEditor.h
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  $<IF:$<CONFIG:DEBUG>,BUILD_IS_DEBUG,BUILD_IS_RELEASE>=1
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  surge-common
  surge-platform
  surge-juce
  juce::juce_graphics
  juce::juce_audio_processors
  juce::juce_audio_plugin_client
  juce::juce_gui_basics
  juce::juce_dsp
  juce::juce_audio_utils
  juce::juce_audio_devices
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
