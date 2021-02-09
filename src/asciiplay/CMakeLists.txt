set(SOURCES
	main.cpp
	options.cpp
	decoder.cpp
	player.cpp
)

set(HEADERS
	options.h
	decoder.h
	decoder_utils.h
	player.h
)

find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

add_executable(asciiplay ${SOURCES} ${HEADERS})
set_property(TARGET asciiplay PROPERTY CXX_STANDARD 17)
target_include_directories(asciiplay PRIVATE .. ${EXT_INCLUDE_DIRS} ${CURSES_INCLUDE_DIRS})
target_link_libraries(asciiplay PRIVATE Threads::Threads ${CMAKE_DL_LIBS} ${CURSES_LIBRARIES})
