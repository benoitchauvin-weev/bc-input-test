cmake_minimum_required(VERSION 3.28)

project(bc_input_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Listed by hand, as the engine does: no GLOB.
add_executable(bc_input_test
	src/main.cpp
	src/app/App.cpp
	src/core/Params.cpp
	src/game/AnimationSystem.cpp
	src/game/Game.cpp
	src/game/GameplaySystem.cpp
	src/game/PhysicsSystem.cpp
	src/game/ScriptedAgent.cpp
	src/input/Input.cpp
	src/input/Input_Win.cpp
	src/input/InputEvent.cpp
	src/input/InputFrame.cpp
	src/input/InputFrameBuilder.cpp
	src/input/InputKeyMap.cpp
	src/input/InputQueue.cpp
	src/input/InputRecording.cpp
	src/render/Renderer_Win.cpp
	src/test/FrameRateProbe.cpp
	src/test/InputTest.cpp
	src/timer/Timer.cpp
	src/window/Window_Win.cpp
)

target_include_directories(bc_input_test PRIVATE src)

target_compile_definitions(bc_input_test PRIVATE
	UNICODE
	_UNICODE
	$<IF:$<CONFIG:Debug>,WV_DEBUG=1,WV_DEBUG=0>
)

target_compile_options(bc_input_test PRIVATE
	-Wall
	-Wextra
	-Wundef
	-fno-rtti
	-fno-exceptions
)

# XInput is loaded at run time (xinput1_4.dll), so it is not linked.
target_link_libraries(bc_input_test PRIVATE user32 gdi32 dwmapi)
