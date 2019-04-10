# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/components/libwebsockets/include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/libwebsockets -llibwebsockets
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += libwebsockets
component-libwebsockets-build: component-mbedtls-build component-openssl-build
