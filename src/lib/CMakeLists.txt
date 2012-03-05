# Library

include_directories(${pcre_INCLUDE_DIRS})

# See http://sourceware.org/autobook/autobook/autobook_91.html for
# how to set the library version appropriately

add_library (mcommon SHARED utilities.c utilities_${VARIANT}.c mainloop.c dnssrv.c dnssrv_${VARIANT}.c)
set_target_properties(mcommon PROPERTIES SOVERSION 1.0.0)
target_link_libraries(mcommon ${SIGAR} ${glib_LIBRARIES})

if(HAVE_RESOLV_H)
    target_link_libraries(mcommon resolv)
endif(HAVE_RESOLV_H)

if(NOT WIN32)
add_library (mrpc SHARED rpc.c rpc-python.c)
set_target_properties(mrpc PROPERTIES SOVERSION 1.0.0)
target_link_libraries(mrpc mcommon ${python_LIBRARIES})
install(TARGETS mrpc DESTINATION lib${LIB_SUFFIX})
endif(NOT WIN32)

if(WIN32)
    target_link_libraries(mcommon wsock32 dnsapi rpcrt4 wininet)
    install(TARGETS mcommon DESTINATION sbin)
else(WIN32)
    target_link_libraries(mcommon curl)
    install(TARGETS mcommon DESTINATION lib${LIB_SUFFIX})
endif(WIN32)

if(WITH-QMF)
    add_library (mcommon_qmf SHARED mh_agent.cpp mh_object.cpp)
    set_target_properties(mcommon_qmf PROPERTIES SOVERSION 1.0.0)

    target_link_libraries(mcommon_qmf mcommon ${QPIDCOMMON_LIBRARY}
        ${QPIDCLIENT_LIBRARY} ${QPIDMESSAGING_LIBRARY} ${QMF2_LIBRARY}
        ${nss_LIBRARIES})
    if(QPIDTYPES_LIBRARY)
        target_link_libraries(mcommon_qmf ${QPIDTYPES_LIBRARY})
    endif(QPIDTYPES_LIBRARY)

    if(WIN32)
        target_link_libraries(mcommon_qmf wsock32)
        install(TARGETS mcommon_qmf DESTINATION sbin)
    else(WIN32)
        install(TARGETS mcommon_qmf DESTINATION lib${LIB_SUFFIX})
    endif(WIN32)
endif(WITH-QMF)

if(WITH-DBUS)
    add_library (mcommon_dbus SHARED mh_dbus_common.c)

    include_directories(${dbus-glib_INCLUDE_DIRS})
    include_directories(${polkit_INCLUDE_DIRS})

    target_link_libraries(mcommon_dbus ${polkit_LIBRARIES})
    set_target_properties(mcommon_dbus PROPERTIES SOVERSION 1.0.0)

    install(TARGETS mcommon_dbus DESTINATION lib${LIB_SUFFIX})
endif(WITH-DBUS)
