/*
** gl_dynlight1.cpp
** dynamic light buffer for shader rendering
**
**---------------------------------------------------------------------------
** Copyright 2009 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
** 4. When not used as part of GZDoom or a GZDoom derivative, this code will be
**    covered by the terms of the GNU Lesser General Public License as published
**    by the Free Software Foundation; either version 2.1 of the License, or (at
**    your option) any later version.
** 5. Full disclosure of the entire project's source code, except for third
**    party libraries is mandatory. (NOTE: This clause is non-negotiable!)
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#if 0	// unused for now. Code doesn't work

#include "gl/system/gl_system.h"
#include "c_dispatch.h"
#include "p_local.h"
#include "vectors.h"
#include "g_level.h"

#include "gl/system/gl_cvars.h"
#include "gl/renderer/gl_renderer.h"
#include "gl/renderer/gl_lightdata.h"
#include "gl/data/gl_data.h"
#include "gl/dynlights/gl_dynlight.h"
#include "gl/dynlights/gl_lightbuffer.h"
#include "gl/scene/gl_drawinfo.h"
#include "gl/scene/gl_portal.h"
#include "gl/shaders/gl_shader.h"
#include "gl/textures/gl_material.h"


//==========================================================================
//
//
//
//==========================================================================

FLightBuffer::FLightBuffer()
{
	glGenBuffers(1, &mIDbuf_RGB);
	glBindBuffer(GL_TEXTURE_BUFFER, mIDbuf_RGB);

	glGenBuffers(1, &mIDbuf_Position);
	glBindBuffer(GL_TEXTURE_BUFFER, mIDbuf_Position);

	glGenTextures(1, &mIDtex_RGB);
	glBindTexture(GL_TEXTURE_BUFFER, mIDtex_RGB);
	gl.TexBufferARB(GL_TEXTURE_BUFFER, GL_RGBA8, mIDbuf_RGB);

	glGenTextures(1, &mIDtex_Position);
	glBindTexture(GL_TEXTURE_BUFFER, mIDtex_Position);
	gl.TexBufferARB(GL_TEXTURE_BUFFER, GL_RGBA32F, mIDbuf_Position);
}


//==========================================================================
//
//
//
//==========================================================================

FLightBuffer::~FLightBuffer()
{
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glDeleteBuffers(1, &mIDbuf_RGB);
	glDeleteBuffers(1, &mIDbuf_Position);

	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glDeleteTextures(1, &mIDtex_RGB);
	glDeleteTextures(1, &mIDtex_Position);

}

//==========================================================================
//
//
//
//==========================================================================

void FLightBuffer::BindTextures(int texunit1, int texunit2)
{
	glActiveTexture(texunit1);
	glBindTexture(GL_TEXTURE_BUFFER, mIDtex_RGB);
	glActiveTexture(texunit2);
	glBindTexture(GL_TEXTURE_BUFFER, mIDtex_Position);
	glActiveTexture(GL_TEXTURE0);
}


//==========================================================================
//
// This collects all currently actove
//
//==========================================================================

void FLightBuffer::CollectLightSources()
{
	if (gl_dynlight_shader && gl_lights && GLRenderer->mLightCount && gl_fixedcolormap == CM_DEFAULT)
	{
		// [AK] Take care of gl_lights_size and ZADF_FORCE_VIDEO_DEFAULTS.
		OVERRIDE_LIGHTS_SIZE_IF_NECESSARY

		TArray<FLightRGB> pLights(100);
		TArray<FLightPosition> pPos(100);
		TThinkerIterator<ADynamicLight> it(STAT_DLIGHT);

		ADynamicLight *light;

		while ((light = it.Next()) != NULL)
		{
			if (!(light->flags2 & MF2_DORMANT))
			{
				FLightRGB rgb;
				FLightPosition pos;

				rgb.R = light->GetRed();
				rgb.G = light->GetGreen();
				rgb.B = light->GetBlue();
				rgb.Type = (light->flags4 & MF4_SUBTRACTIVE)? 128 : (light->flags4 & MF4_ADDITIVE || foggy)? 255:0;
				pos.X = FIXED2FLOAT(light->x);
				pos.Y = FIXED2FLOAT(light->y); 
				pos.Z =  FIXED2FLOAT(light->z);
				pos.Distance = (light->GetRadius() * gl_lights_size);
				light->bufferindex = pPos.Size();
				pLights.Push(rgb);
				pPos.Push(pos);
			}
			else light->bufferindex = -1;
		}
		GLRenderer->mLightCount = pPos.Size();

		glBindBuffer(GL_TEXTURE_BUFFER, mIDbuf_RGB);
		glBufferData(GL_TEXTURE_BUFFER, pLights.Size() * sizeof (FLightRGB), &pLights[0], GL_STREAM_DRAW);

		glBindBuffer(GL_TEXTURE_BUFFER, mIDbuf_Position);
		glBufferData(GL_TEXTURE_BUFFER, pPos.Size() * sizeof (FLightPosition), &pPos[0], GL_STREAM_DRAW);

	}
}


//==========================================================================
//
//
//
//==========================================================================

FLightIndexBuffer::FLightIndexBuffer()
{
	glGenBuffers(1, &mIDBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, mIDBuffer);

	glGenTextures(1, &mIDTexture);
	glBindTexture(GL_TEXTURE_BUFFER, mIDTexture);
	gl.TexBufferARB(GL_TEXTURE_BUFFER, GL_R16UI, mIDBuffer);
}

//==========================================================================
//
//
//
//==========================================================================

FLightIndexBuffer::~FLightIndexBuffer()
{
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glDeleteBuffers(1, &mIDBuffer);

	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glDeleteTextures(1, &mIDTexture);
}


//==========================================================================
//
//
//
//==========================================================================

void FLightIndexBuffer::AddLight(ADynamicLight *light)
{
	if (light->bufferindex >= 0)
	{
		mBuffer.Push(light->bufferindex);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void FLightIndexBuffer::SendBuffer()
{
	glBindBuffer(GL_TEXTURE_BUFFER, mIDBuffer);
	glBufferData(GL_TEXTURE_BUFFER, mBuffer.Size() * sizeof (short), &mBuffer[0], GL_STREAM_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
}


//==========================================================================
//
//
//
//==========================================================================

void FLightIndexBuffer::BindTexture(int texunit1)
{
	glActiveTexture(texunit1);
	glBindTexture(GL_TEXTURE_BUFFER, mIDTexture);
	glActiveTexture(GL_TEXTURE0);
}



#endif