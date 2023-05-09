SET(LINK_DEPS
	xstudio::module
	Imath::Imath
	GLEW::glew
)

find_package(Imath)
find_package(GLEW)

create_plugin_with_alias(viewport_hud xstudio::viewport::viewport_hud 0.1.0  "${LINK_DEPS}")

add_subdirectory(qml)