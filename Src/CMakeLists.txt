#Author: Kang Lin(kl222@126.com)

project(RabbitProxy)

set(INSTALL_HEAD_FILES
    ${INSTALL_HEAD_FILES}
    ProxyServer.h
    ProxyServerSocket.h
    Proxy.h
    ProxySocket4.h
    ProxySocket5.h
    PeerConnecter.h
    Signal.h
    Parameter.h
    ParameterSocket.h
    )
set(HEADER_FILES
    ${INSTALL_HEAD_FILES}
    )
set(SOURCE_FILES
    ${SOURCE_FILES}
    ProxyServer.cpp
    ProxyServerSocket.cpp
    Proxy.cpp
    ProxySocket4.cpp
    ProxySocket5.cpp
    PeerConnecter.cpp
    Signal.cpp
    Parameter.cpp
    ParameterSocket.cpp
    )
set(SOURCE_UI_FILES
    )

set(_LibDataChannel_LIBS RabbitCommon ${QT_LIBRARIES})
set(_LibDataChannel_DIR ${LibDataChannel_DIR})
find_package(LibDataChannel)
if(LibDataChannel_FOUND)
    list(APPEND _LibDataChannel_LIBS LibDatachannel::datachannel)
else()
    find_path(_LibDataChannel_INCLUDE_DIRS
        NAMES "rtc/rtc.hpp"
        HINTS ${_LibDataChannel_DIR}
        PATHS ${_LibDataChannel_DIR}
        PATH_SUFFIXES include        
        )
    find_library(_LibDataChannel_LIB
        NAMES datachannel
        HINTS ${_LibDataChannel_DIR}
        PATHS ${_LibDataChannel_DIR}
        PATH_SUFFIXES lib
        )
    if(_LibDataChannel_LIB)
        list(APPEND _LibDataChannel_LIBS ${_LibDataChannel_LIB})
    endif()
    if(_LibDataChannel_INCLUDE_DIRS AND _LibDataChannel_LIB)
        set(LibDataChannel_FOUND TRUE)
    endif()
endif()
if(LibDataChannel_FOUND)
    set(INSTALL_HEAD_FILES
        ${INSTALL_HEAD_FILES}
        PeerConnecterIce.h
        SignalWebSocket.h
        )
    set(SOURCE_FILES
        ${SOURCE_FILES}
        PeerConnecterIce.cpp
        SignalWebSocket.cpp
        )
    list(APPEND PROXY_PRIVATE_DEFINITIONS HAVE_ICE)
endif()

#翻译
GENERATED_QT_TRANSLATIONS(SOURCES ${SOURCE_FILES} ${SOURCE_UI_FILES} ${HEADER_FILES}
    OUT_QRC TRANSLATIONS_QRC_FILES)
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
if(BUILD_TYPE STREQUAL "debug")
    list(APPEND PROXY_PRIVATE_DEFINITIONS _DEBUG)
    LIST(APPEND RCC_FILES 
        ${TRANSLATIONS_QRC_FILES}
        )
endif()

ADD_TARGET(NAME ${PROJECT_NAME}
    SOURCE_FILES ${SOURCE_FILES} ${SOURCE_UI_FILES} ${RCC_FILES} ${HEADER_FILES}
    INSTALL_HEADER_FILES ${INSTALL_HEAD_FILES}
    INCLUDE_DIRS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # See: http://www.it1352.com/478094.html
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    PRIVATE_DEFINITIONS ${PROXY_PRIVATE_DEFINITIONS}
    LIBS ${_LibDataChannel_LIBS}
    PRIVATE_INCLUDE_DIRS ${_LibDataChannel_INCLUDE_DIRS}
    VERSION ${BUILD_VERSION})
