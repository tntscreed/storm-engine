STORM_SETUP(
        TARGET_NAME config
        TYPE storm_module
        DEPENDENCIES core tomlplusplus nlohmann_json
        TEST_DEPENDENCIES catch2
)
