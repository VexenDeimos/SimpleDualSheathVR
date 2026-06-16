cmake_minimum_required(VERSION 3.24)

project(SimpleDualSheathVR VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(xbyak CONFIG REQUIRED)

add_commonlibsse_plugin(${PROJECT_NAME}
	SOURCES
		src/PCH.h
		src/main.cpp
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE
		src/PCH.h
)

target_compile_features(${PROJECT_NAME}
	PRIVATE
		cxx_std_23
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		CommonLibSSE::CommonLibSSE
		xbyak::xbyak
)