##Header files
set(HEADER_FILES
        types.hh
        args.hh
        exception.hh
        trace.hh
        database.hh
        write_manager.hh
        interpreter_sp.hh
        interpreter_fsip.hh
        storage_manager.hh
        partition_base.hh
        partition_file.hh
        tcp_server.hh
        tcp_connection.hh
        )

##Source files
set(SOURCE_FILES
        types.cc
        args.cc
        exception.cc
        trace.cc
        interpreter_sp.cc
        interpreter_fsip.cc
        partition_base.cc
        partition_file.cc
        tcp_server.cc
        tcp_connection.cc
        )

#Create library which is later linked to the main executable
ADD_LIBRARY(${CMAKE_PROJECT_NAME}_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})

#Link main executable with code lib
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}_main ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)
TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}_main ${CMAKE_PROJECT_NAME}_lib -lstdc++fs ${CMAKE_THREAD_LIBS_INIT})

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}_server_main ${CMAKE_CURRENT_SOURCE_DIR}/server_main.cc)
TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}_server_main ${CMAKE_PROJECT_NAME}_lib -lstdc++fs ${CMAKE_THREAD_LIBS_INIT} ${Boost_SYSTEM_LIBRARY} OpenSSL::SSL)

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}_client_main ${CMAKE_CURRENT_SOURCE_DIR}/client_main.cc)
TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME}_client_main ${Boost_SYSTEM_LIBRARY} OpenSSL::SSL)
