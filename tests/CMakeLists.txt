cmake_minimum_required (VERSION 3.13.0)

project(test-corgi-test)

add_executable(test-corgi-test "") 

# I'm not sure I understand all the advantages on using target_sources but at least
# it's one way to avoid adding variables, and to define our target early
target_sources(${PROJECT_NAME}
   PUBLIC 
       main.cpp 
       test_fixture.cpp
       TestA.cpp 
       TestB.cpp
       TestTime.cpp)

if(MSVC)
target_compile_options(${PROJECT_NAME} PRIVATE -W4 -WX)
else()
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
endif()
       
target_link_libraries(${PROJECT_NAME} corgi-test)

set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_STANDARD 20)

add_test( NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
