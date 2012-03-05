set(BASE "network")
set(BASE_LIB "m${BASE}")
set(QMF_AGENT "matahari-qmf-${BASE}d")
set(DBUS_AGENT "matahari-dbus-${BASE}d")

generate_headers_from_schema(${CMAKE_CURRENT_SOURCE_DIR}/schema.xml
                             ${BASE}.h)

# Library
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(${BASE_LIB} SHARED ${BASE}.c ${BASE}_${VARIANT}.c ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.h)
set_target_properties(${BASE_LIB} PROPERTIES SOVERSION 1.0.0)
target_link_libraries(${BASE_LIB} ${uuid_LIBRARIES} ${pcre_LIBRARIES} mcommon ${SIGAR} ${glib_LIBRARIES})

install(TARGETS ${BASE_LIB} DESTINATION lib${LIB_SUFFIX})
install(FILES ${BASE}.h DESTINATION include/matahari)

if(WIN32)
    target_link_libraries(${BASE_LIB} wsock32 dnsapi rpcrt4 wininet)
    install(TARGETS ${BASE_LIB} DESTINATION sbin)
else(WIN32)
    target_link_libraries(${BASE_LIB} curl)
    install(TARGETS ${BASE_LIB} DESTINATION lib${LIB_SUFFIX})
endif(WIN32)

# QMF daemon
if(WITH-QMF)
    set(SCHEMA_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/qmf/org/matahariproject/QmfPackage.cpp)
    generate_qmf_schemas(${CMAKE_CURRENT_SOURCE_DIR}/schema.xml ${SCHEMA_SOURCES})

    generate_qmf_glue(${CMAKE_CURRENT_SOURCE_DIR}/schema.xml
                      ${BASE}-qmf-glue.cpp)

    add_executable(${QMF_AGENT} ${BASE}-qmf-glue.cpp ${SCHEMA_SOURCES})
    target_link_libraries(${QMF_AGENT} ${BASE_LIB} mcommon_qmf)

    #create_manpage(${QMF_AGENT} ${AGENT_MAN_SECTION} ${AGENT_MAN_DESC})
    install_manpage(${QMF_AGENT} ${AGENT_MAN_SECTION})
    create_service_scripts(${BASE})

    install(TARGETS ${QMF_AGENT} DESTINATION sbin)

endif(WITH-QMF)

# DBus daemon
if(WITH-DBUS)
    # Auto-generated stuff
    # Create src/dbus directory and generate DBus XML definition files into it
    generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/schema.xml)

    generate_dbus_headers(${BASE} ${CMAKE_CURRENT_BINARY_DIR}/org.matahariproject.xml)

    generate_dbus_glue(${CMAKE_CURRENT_SOURCE_DIR}/schema.xml
                       ${BASE}-dbus-glue.c)

    # Check if all properties|statistics|methods have PolicyKit action defined
    check_policies_for_schema(${CMAKE_CURRENT_SOURCE_DIR}/schema.xml)

    set(DBUS_AGENT_SOURCE
        ${BASE}-dbus-glue.c
        ${BASE}-dbus-glue.h
    )

    add_executable(${DBUS_AGENT} ${DBUS_AGENT_SOURCE})

    target_link_libraries(${DBUS_AGENT} ${BASE_LIB} mcommon_dbus)

    include_directories(${polkit_INCLUDE_DIRS})
    target_link_libraries(${DBUS_AGENT} ${polkit_LIBRARIES})

    include_directories(${dbus-glib_INCLUDE_DIRS})
    target_link_libraries(${DBUS_AGENT} ${dbus-glib_LIBRARIES})

    # Install targets
    # TODO: fix hardcoded paths, should go to libexec
    install(TARGETS ${DBUS_AGENT} DESTINATION sbin)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.matahariproject.xml DESTINATION share/dbus-1/interfaces RENAME org.matahariproject.Network.xml)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.matahariproject.Network.service DESTINATION share/dbus-1/system-services)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.matahariproject.Network.conf DESTINATION /etc/dbus-1/system.d)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.matahariproject.Network.policy DESTINATION share/polkit-1/actions)
endif(WITH-DBUS)
