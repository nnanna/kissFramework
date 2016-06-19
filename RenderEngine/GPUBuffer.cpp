
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		19/06/2016
///	@brief		
///
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
/// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
/// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////

#include "GPUBuffer.h"
#include "Debug.h"
#include "SimpleShaderContainer.h"

namespace ks {

GPUBuffer::GPUBuffer(unsigned size, GLenum target /*= GL_ARRAY_BUFFER*/, GLenum usage /*= GL_STATIC_DRAW*/, void* data /*= nullptr*/)
	: m_size(size)
	, m_target(target)
{
    glGenBuffers(1, &m_buffer);

    bind();
	glBufferData(m_target, m_size, data, usage);
    unbind();
}

GPUBuffer::~GPUBuffer()
{
    glDeleteBuffers(1, &m_buffer);
}


void GPUBuffer::bind()
{
    glBindBuffer(m_target, m_buffer);
	CHECK_GL_ERROR;
}

void GPUBuffer::unbind()
{
	glBindBuffer(m_target, 0);
	CHECK_GL_ERROR;
}

void *GPUBuffer::map(unsigned p_size, unsigned access /*= GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT*/)
{
	KS_ASSERT( p_size <= m_size );
    bind();
	return glMapBufferRange(m_target, 0, p_size, access);
}

void GPUBuffer::unmap()
{
	glUnmapBuffer(m_target);
	unbind();
}

}