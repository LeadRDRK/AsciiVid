find_package(PNG REQUIRED)

set(SOURCES
	main.cpp
	options.cpp
	encoder.cpp
	writer.cpp
	audioloader.cpp
)

set(HEADERS
	options.h
	encoder.h
	writer.h
	writer_utils.h
	audioloader.h
	../types.h
)

add_executable(asciienc ${SOURCES} ${HEADERS})
set_property(TARGET asciienc PROPERTY CXX_STANDARD 17)
target_link_libraries(asciienc PRIVATE ${PNG_LIBRARIES})
target_include_directories(asciienc PRIVATE .. ${EXT_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
