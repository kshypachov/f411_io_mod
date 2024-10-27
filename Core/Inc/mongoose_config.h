#pragma once

// See https://mongoose.ws/documentation/#build-options
#define MG_ARCH MG_ARCH_FREERTOS

#define MG_ENABLE_TCPIP 			1
#define MG_ENABLE_DRIVER_W5500 		1
//#define MG_ENABLE_PACKED_FS  		1
#define MG_ENABLE_DIRLIST 			1
#define MG_HTTP_INDEX 				"index.html"
//#define MG_ENABLE_CUSTOM_LOG 		1

// <o MG_TLS> TLS support
//   <MG_TLS_NONE=> None
//   <MG_TLS_BUILTIN=> Built-in TLS 1.3 stack
//   <MG_TLS_MBED=> MbedTLS
#define MG_TLS 						MG_TLS_NONE


// <q> Enable packed (embedded) filesystem
//#if MG_ENABLE_PACKED_FS
#define MG_ENABLE_POSIX_FS 			0
//#define MG_ENABLE_LINES 1
//#endif
