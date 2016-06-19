
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

#ifndef GPU_BUFFER_H 
#define GPU_BUFFER_H

#include "GL\glew.h"

namespace ks {

	typedef unsigned BufferType;

class GPUBuffer
{
public:
	GPUBuffer(unsigned byte_size, GLenum target = GL_ARRAY_BUFFER, GLenum usage = GL_STATIC_DRAW, void* data = nullptr);
    ~GPUBuffer();

    void bind();
    void unbind();

	void *map(unsigned p_size, unsigned flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	template<typename T>
	T* map(unsigned pNumElements)
	{
		return (T*)map(sizeof(T) * pNumElements, GL_MAP_WRITE_BIT );
	}
    
	void unmap();

	BufferType getBuffer() const	{ return m_buffer; }
	unsigned capacity() const		{ return m_size; }

	template<typename T>
	static GPUBuffer* create(unsigned pNumElements, GLenum type, GLenum usage, void* data = nullptr)
	{
		return new GPUBuffer(sizeof(T) * pNumElements, type, usage, data);
	}

private:
	unsigned	m_size;

	BufferType	m_buffer;

	GLenum		m_target;
};

}	// namespace ks

#endif // GPU_BUFFER_H