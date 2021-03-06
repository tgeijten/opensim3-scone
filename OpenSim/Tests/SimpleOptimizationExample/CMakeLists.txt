cmake_minimum_required(VERSION 2.6)

INCLUDE_DIRECTORIES(${OpenSim_SOURCE_DIR} 
		    ${OpenSim_SOURCE_DIR}/Vendors)

LINK_LIBRARIES( debug osimCommon${CMAKE_DEBUG_POSTFIX} optimized osimCommon
		debug osimSimulation${CMAKE_DEBUG_POSTFIX} optimized osimSimulation
		debug osimActuators${CMAKE_DEBUG_POSTFIX} optimized osimActuators
		debug osimAnalyses${CMAKE_DEBUG_POSTFIX} optimized osimAnalyses
		debug osimTools${CMAKE_DEBUG_POSTFIX} optimized osimTools
		${SIMTK_ALL_LIBS})

# Variable definitions
SET(EXAMPLE_TARGET exampleOptimizationSimple)
SET(EXAMPLE_DIR ${OpenSim_SOURCE_DIR}/OpenSim/Examples/SimpleOptimizationExample)
SET(TEST_TARGET testSimpleOptimizationExample)
SET(TEST_DIR ${OpenSim_BINARY_DIR}/OpenSim/Tests/SimpleOptimizationExample)

FILE(GLOB SOURCE_FILES ${EXAMPLE_DIR}/*.h ${EXAMPLE_DIR}/*.cpp)
ADD_EXECUTABLE(${EXAMPLE_TARGET} ${SOURCE_FILES})
ADD_EXECUTABLE(${TEST_TARGET} ${TEST_TARGET}.cpp)

TARGET_LINK_LIBRARIES(${TEST_TARGET} ${LINK_LIBRARIES})

FILE(GLOB TEST_FILES 
	${EXAMPLE_DIR}/*.obj 
	${EXAMPLE_DIR}/*.osim 
	${EXAMPLE_DIR}/*.xml 
	${EXAMPLE_DIR}/*.sto 
	${EXAMPLE_DIR}/*.mot
	*.obj *.osim *.xml *.sto *.mot)

#
# Copy files to run dir
#
FOREACH (dataFile ${TEST_FILES})
 ADD_CUSTOM_COMMAND(
    TARGET ${TEST_TARGET}
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy
    ${dataFile}
    ${TEST_DIR})
ENDFOREACH (dataFile) 

#
# Testing
#
IF (EXECUTABLE_OUTPUT_PATH)
  SET (TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
ELSE (EXECUTABLE_OUTPUT_PATH)
  SET (TEST_PATH .)
ENDIF (EXECUTABLE_OUTPUT_PATH)

ADD_TEST(${TEST_TARGET} ${TEST_PATH}/${TEST_TARGET})

SET_TESTS_PROPERTIES(${TEST_TARGET} PROPERTIES DEPENDS ${TEST_TARGET}Runs)

SET_TARGET_PROPERTIES(${EXAMPLE_TARGET} PROPERTIES PROJECT_LABEL "Examples - ${EXAMPLE_TARGET}" )
SET_TARGET_PROPERTIES(${TEST_TARGET} PROPERTIES ${EXCLUDE_IF_MINIMAL_BUILD} PROJECT_LABEL "Tests - ${TEST_TARGET}")