if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03")
endif()

set(AGGE_TEST_SOURCES
	BitmapTests.cpp
	BlendersTests.cpp
	ClipperTests.cpp
	CurvesTests.cpp
	DashTests.cpp
	FiguresTests.cpp
	FillingRulesTests.cpp
	GenericBlendersTests.cpp
	MiscTests.cpp
	ParallelExecutionTests.cpp
	PathJoiningTests.cpp
	PathTests.cpp
	PodVectorTests.cpp
	PreciseDeltaTests.cpp
	RasterizerTests.cpp
	RendererTests.cpp
	RendererParallelTests.cpp
	ScanlineAdapterTests.cpp
	SIMDBlendersTests.cpp
	StrokeFeaturesTests.cpp
	StrokeTests.cpp
	VectorRasterizerTests.cpp
)

if (WIN32)
	set(AGGE_TEST_SOURCES ${AGGE_TEST_SOURCES} platform/win32/RawBitmapTests.cpp platform/win32/surface.cpp)
else()
endif()

find_package(Threads)

add_library(agge.tests SHARED ${AGGE_TEST_SOURCES})

target_link_libraries(agge.tests agge tests-common utee ${CMAKE_THREAD_LIBS_INIT})
