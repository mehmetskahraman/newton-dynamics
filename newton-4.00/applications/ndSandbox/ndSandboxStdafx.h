/* Copyright (c) <2003-2022> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently


#ifndef _TOLLBOX_STDAFX_H_
#define _TOLLBOX_STDAFX_H_

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <ctype.h>

#if (defined(WIN32) || defined(_WIN32))
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

	#undef APIENTRY
	#define GLFW_EXPOSE_NATIVE_WIN32
	#define GLFW_EXPOSE_NATIVE_WGL

	#include <windows.h>
	#include <commctrl.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <malloc.h>
	#include <memory.h>
	#include <time.h>
	#include <tchar.h>
	#include <crtdbg.h>
	
	#include <glatter.h>
	#include <GL/glu.h>
	#include <GL/gl.h>
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>

	// audio library support
	#include <AL/al.h>
	#include <AL/alc.h>

#endif
	
#if defined (__linux__ )
	#include <stdlib.h>
	#include <unistd.h>
	#include <time.h>

	#include <glatter.h>
	#include <GL/glu.h>
	#include <GL/gl.h>
	#include <GLFW/glfw3.h>

	// audio library support
	#include <AL/al.h>
	#include <AL/alc.h>
#endif

#if defined (__APPLE__)
	#include <CoreFoundation/CoreFoundation.h> 
	#include <unistd.h>
	#include <GL/glew.h>
	#include <OpenGL/glu.h>
	#include <OpenGL/gl.h>

	// audio library support
	#include <OpenAl/al.h>
	#include <OpenAl/alc.h>
#endif

// some third party libraries includes here
#include <ofbx.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "VHACD.h"
//#include "include/VHACD.h"

// SDK includes
#include <ndNewton.h>

#include <ndBrainStdafx.h>
#include <ndBrain.h>
#include <ndBrainTrainer.h>
#include <ndBrainAgentDQN.h>
#include <ndBrainAgentDDPG.h>
#include "ndBrainReplayBuffer.h"
#include <ndBrainParallelTrainer.h>

void dGetWorkingFileName (const char* const name, char* const outPathName);

// endian conversion
ndUnsigned32 SWAP_INT32(ndUnsigned32 x);
ndUnsigned16 SWAP_INT16(ndUnsigned16 x);
ndUnsigned16 ndIndian16(ndUnsigned16 x);
ndUnsigned32 ndIndian32(ndUnsigned32 x);

#endif 

