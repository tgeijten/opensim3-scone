SET(APPNAME cmc)

INCLUDE_DIRECTORIES(${OpenSim_SOURCE_DIR} 
						  ${OpenSim_SOURCE_DIR}/Vendors)

# cmake should automatically figure out what other libraries are needed
LINK_LIBRARIES(debug ${OPENSIM_LIBRARY_PREFIX}Tools${CMAKE_DEBUG_POSTFIX} optimized ${OPENSIM_LIBRARY_PREFIX}Tools
               debug ${OPENSIM_LIBRARY_PREFIX}Common${CMAKE_DEBUG_POSTFIX} optimized ${OPENSIM_LIBRARY_PREFIX}Common
               debug ${OPENSIM_LIBRARY_PREFIX}Simulation${CMAKE_DEBUG_POSTFIX} optimized ${OPENSIM_LIBRARY_PREFIX}Simulation)

ADD_EXECUTABLE(${APPNAME} ${APPNAME}.cpp)
INSTALL_TARGETS(/bin/ ${APPNAME})
SET_TARGET_PROPERTIES(${APPNAME} PROPERTIES PROJECT_LABEL "Applications - ${APPNAME}")

#
# Testing
#

#IF (UNIX)
#  ADD_DEFINITIONS(-fprofile-arcs -ftest-coverage)
#  LINK_LIBRARIES(gcov)
#ENDIF (UNIX)

IF(BUILD_TESTING)
	SUBDIRS(test)
ENDIF(BUILD_TESTING)



