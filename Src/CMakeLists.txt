# Author: Kang Lin <kl222@126.com>

project(RabbitProxy)

set(INSTALL_HEAD_FILES
    ${INSTALL_HEAD_FILES}
    Server.h
    ServerSocks.h
    Proxy.h
    ProxySocks4.h
    ProxySocks5.h
    PeerConnector.h
    Parameter.h
    ParameterIce.h
    ParameterSocks.h
    )
set(HEADER_FILES
    ${INSTALL_HEAD_FILES}
    )
set(SOURCE_FILES
    Server.cpp
    ServerSocks.cpp
    Proxy.cpp
    ProxySocks4.cpp
    ProxySocks5.cpp
    PeerConnector.cpp
    Parameter.cpp
    ParameterIce.cpp
    ParameterSocks.cpp
    )
set(SOURCE_UI_FILES
    )

set(_PROXY_LIBS RabbitCommon ${QT_LIBRARIES})

option(WITH_ICE "With ICE" ON)
if(WITH_ICE)
    find_package(LibDataChannel)
    if(LibDataChannel_FOUND)
        if (TARGET LibDataChannel::LibDataChannel)
            list(APPEND _PROXY_PRIVATE_LIBS LibDataChannel::LibDataChannel)
        elseif (TARGET LibDatachannel::datachannel)
            list(APPEND _PROXY_PRIVATE_LIBS LibDatachannel::datachannel)
        else()
            list(APPEND _PROXY_PRIVATE_LIBS LibDataChannel::datachannel)
        endif()
        list(APPEND INSTALL_HEAD_FILES
            PeerConnectorIceClient.h
            PeerConnectorIceServer.h
            IceSignal.h
            DataChannelIce.h
            DataChannelIceChannel.h
            )
        list(APPEND HEADER_FILES
            IceManager.h)
        list(APPEND SOURCE_FILES
            PeerConnectorIceClient.cpp
            PeerConnectorIceServer.cpp
            IceSignal.cpp
            DataChannelIce.cpp
            DataChannelIceChannel.cpp
            IceManager.cpp
            )
        list(APPEND PROXY_DEFINITIONS HAVE_ICE)
        option(WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL
            "Use one peer connect to one data channel" ON)
        if(WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL)
            list(APPEND PROXY_DEFINITIONS WITH_ONE_PEERCONNECTION_ONE_DATACHANNEL)
        endif()
        
        find_package(nlohmann_json)
        if(nlohmann_json_FOUND)
            list(APPEND _PROXY_PRIVATE_LIBS nlohmann_json::nlohmann_json)
            list(APPEND PROXY_DEFINITIONS HAVE_WebSocket)
            list(APPEND HEADER_FILES IceSignalWebSocket.h)
            list(APPEND SOURCE_FILES IceSignalWebSocket.cpp)
        endif()
        
        find_package(QXmpp)
        if(QXmpp_FOUND)
            list(APPEND _PROXY_LIBS QXmpp::QXmpp)
            list(APPEND PROXY_DEFINITIONS HAVE_QXMPP)
            list(APPEND HEADER_FILES
                IceSignalQxmpp.h
                IceSignalQXmppManager.h
                IceSignalQXmppIq.h)
            list(APPEND SOURCE_FILES
                IceSignalQxmpp.cpp
                IceSignalQXmppManager.cpp
                IceSignalQXmppIq.cpp)
            INSTALL_TARGETS(TARGETS QXmpp::QXmpp)
        endif()
    else()
        message(AUTHOR_WARNING "Please install libdatachannel from https://github.com/paullouisageneau/libdatachannel")
    endif()
endif(WITH_ICE)
message("LIBS: ${_PROXY_LIBS} ${_PROXY_PRIVATE_LIBS}")

ADD_TARGET(NAME ${PROJECT_NAME}
    SOURCE_FILES ${SOURCE_FILES} ${SOURCE_UI_FILES} ${HEADER_FILES}
    INSTALL_HEADER_FILES ${INSTALL_HEAD_FILES}
    INCLUDE_DIRS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # See: http://www.it1352.com/478094.html
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    PRIVATE_DEFINITIONS ${PROXY_PRIVATE_DEFINITIONS}
    DEFINITIONS ${PROXY_DEFINITIONS}
    PRIVATE_LIBS ${_PROXY_PRIVATE_LIBS}
    LIBS ${_PROXY_LIBS}
    VERSION ${BUILD_VERSION}
    )
