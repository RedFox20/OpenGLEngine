/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "VertexBuffer.h"
#include "GL\glew.h"
#include "GL\glut.h"

static const VertexBuffer* CurrentVertexBuffer = NULL;
static const VertexBuffer* CurrentIndexBuffer = NULL;
#define BindVertexBuffer() if(CurrentVertexBuffer != this) { glBindBuffer(GL_ARRAY_BUFFER, VBO); CurrentVertexBuffer = this; }
#define BindIndexBuffer() if(CurrentIndexBuffer != this) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO); CurrentIndexBuffer = this; }




	/**
	 * @brief Destroys the VertexBuffer if needed
	 */
	VertexBuffer::~VertexBuffer()
	{
		if(VBO) 
		{
			if(this == CurrentVertexBuffer) CurrentVertexBuffer = NULL;
			glDeleteBuffers(1, &VBO);
		}
	}


	/**
	 * @brief Copying actually produces move semantics
	 */
	VertexBuffer& VertexBuffer::operator=(VertexBuffer& vb)
	{
		if(this != &vb)
		{
			VBO = vb.VBO, Descr = vb.Descr, VertexCount = vb.VertexCount, Type = vb.Type, Style = vb.Style;
			vb.VBO = 0, vb.Descr = 0, vb.VertexCount = 0, vb.Type = BUFFER_INVALID, vb.Style = DRAW_INVALID;
		}
		return *this;
	}




	/**
	 * @brief Creates a new VertexBuffer, default type is STATIC TRIANGLES
	 * @param vdescr Vertex layout description object
	 * @param btype Type of buffer to use
	 * @param dstyle Drawing style
	 * @return TRUE on success. Fails if buffer has been created.
	 */
	bool VertexBuffer::Create(const VertexDescr* vdescr, BufferType type, DrawStyle dstyle)
	{
		if(VBO) return false; // already created
		Descr = vdescr;
		Type = type;
		switch(dstyle) {
			case DRAW_TRIANGLES: Style = GL_TRIANGLES; break;
			case DRAW_TRIANGLE_STRIP: Style = GL_TRIANGLE_STRIP; break;
			case DRAW_TRIANGLE_FAN: Style = GL_TRIANGLE_FAN; break;
			case DRAW_LINES: Style = GL_LINES; break;
			case DRAW_LINE_STRIP: Style = GL_LINE_STRIP; break;
		}
		return true;
	}


	/**
	 * @brief Destroys the VertexBuffer and its buffers
	 */
	void VertexBuffer::Destroy()
	{
		if(VBO)
		{
			glDeleteBuffers(1, &VBO);
			VBO = 0;
			VertexCount = 0;
		}
	}


	/**
	 * @brief Binds this VertexBuffer's attributes to the shader
	 */
	void VertexBuffer::_Bind() const
	{
		BindVertexBuffer();
		int numAttributes = Descr->Attributes;
		int sizeOfVertex = Descr->SizeOf;
		int attrOffset = 0;
		for(int a = 0; a < numAttributes; a++) {
			int elemCount = Descr->ElementCount[a];
			glVertexAttribPointer(a,
				elemCount,		// number of float elements in a single vertex
				GL_FLOAT,		// we only deal with floats
				GL_FALSE,		// no normalization
				sizeOfVertex,	// stride in sizeof a single vertex
				(void*)(sizeof(float)*attrOffset) // attribute offset
			);
			attrOffset += elemCount;
		}
	}


	/**
	 * @brief Draws the VertexBuffer on the screen. Assumes Bind() has been called.
	 */
	void VertexBuffer::Draw() const
	{
		_Bind(); // we're using auto-bind now
		glDrawArrays(Style, 0, VertexCount); // draw all vertices
	}


	/**
	 * @brief Draws a segment of the VertexBuffer on the screen. Assumes Bind() has been called.
	 * @param start Starting offset index
	 * @param count Number of elements to draw
	 */
	void VertexBuffer::Draw(int start, int count) const
	{
		_Bind(); // we're using auto-bind now
		int available = VertexCount - start;
		if(count > available) count = available;
		if(count) glDrawArrays(Style, start, count);
	}


	/**
	 * @brief Maps this VertexBuffer into client memory
	 * @param mapType Type of mapping to use R/W/RW
	 * @note UnmapVBO must be called after calling MapVBO (!)
	 * @return Mapped address to client memory
	 */
	void* VertexBuffer::MapVBO(BufferMapType mapType)
	{
		BindVertexBuffer();
		GLenum access[] = { GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE };
		return glMapBuffer(GL_ARRAY_BUFFER, access[mapType]);
	}


	/**
	 * @brief Unmaps the VertexBuffer from client memory
	 * @note Must be called after using MapVBO (!)
	 */
	void VertexBuffer::UnmapVBO()
	{
		BindVertexBuffer();
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}


	/**
	 * @brief Buffers data into the VertexBuffer
	 * @param vertices Array of vertices to buffer
	 * @param vertexCount Number of vertices in the array
	 */
	void VertexBuffer::BufferVertices(const void* vertices, int vertexCount)
	{
		if(!VBO) glGenBuffers(1, &VBO);
		if(!vertices || !vertexCount) return; // nothing to do here

		GLenum bufferType;
		switch(this->Type) {
			case BUFFER_DYNAMIC: bufferType = GL_DYNAMIC_DRAW; break;
			case BUFFER_STREAM: bufferType = GL_STREAM_DRAW; break;
			default: bufferType = GL_STATIC_DRAW; break;
		}

		VertexCount = vertexCount;
		int sizeOfVertex = Descr->SizeOf;
		BindVertexBuffer();
		glBufferData(GL_ARRAY_BUFFER, sizeOfVertex*vertexCount, vertices, bufferType);
	}


	/**
	 * @brief Buffers subdata or updates inside the VertexBuffer
	 * @param vertices Array of vertices to buffer
	 * @param vertexCount Number of vertices in the array
	 */
	void VertexBuffer::UpdateVertices(const void* vertices, int vertexCount)
	{
		if(!VBO || !vertices || !vertexCount)
			return; // nothing to do here
		if(vertexCount <= VertexCount) // new size is smaller - we don't need to resize anything
		{
			memcpy((char*)MapVBO(MAP_WO), vertices, Descr->SizeOf * vertexCount);
			UnmapVBO();
		}
		else // whole buffer needs a resize. damnit
		{
			glBufferData(GL_ARRAY_BUFFER, Descr->SizeOf * vertexCount, vertices, GL_DYNAMIC_DRAW);
		}
		VertexCount = vertexCount;
	}


	/**
	 * @brief Inserts new vertices at the specified index
	 * @note The vertex array will be expanded to fit all the vertices
	 * @param vertices Array of vertices to buffer
	 * @param vertexCount Number of vertices in the array
	 * @param offsetIndex Offset index where to insert
	 */
	void VertexBuffer::InsertVertices(const void* vertices, int vertexCount, int offsetIndex)
	{
		if(!VBO || !vertices || !vertexCount)
			return;

		int sizeOf = Descr->SizeOf;
		int insertSize = vertexCount * sizeOf;
		int offsetSize = offsetIndex * sizeOf;
		int oldSize = VertexCount * sizeOf;
		int newSize = oldSize + insertSize;

		char* data = newSize <= 65536 ? (char*)alloca(newSize) : (char*)malloc(newSize);
		char* vbuffer = (char*)MapVBO(MAP_RO);
			memcpy(data, vbuffer, offsetSize);
			memcpy(data + offsetSize, vertices, insertSize);
			memcpy(data + offsetSize + insertSize, vbuffer + offsetSize, oldSize - offsetSize);
		UnmapVBO();

		glBufferData(GL_ARRAY_BUFFER, newSize, data, GL_DYNAMIC_DRAW);
		VertexCount += vertexCount;
		if(newSize > 65536) free(data);
	}


	/**
	 * @brief Appends vertices to the end of the buffer
	 * @note The vertex array will be expanded to fit all the vertices
	 * @param vertices Array of vertices to append
	 * @param vertexCount Number of vertices in the arrray
	 */
	void VertexBuffer::AppendVertices(const void* vertices, int vertexCount)
	{
		if(!VBO || !vertices || !vertexCount)
			return;

		int sizeOf = Descr->SizeOf;
		int appendSize = vertexCount * sizeOf;
		int oldSize = VertexCount * sizeOf;
		int newSize = oldSize + appendSize;

		char* data = newSize <= 65536 ? (char*)alloca(newSize) : (char*)malloc(newSize);
		char* vbuffer = (char*)MapVBO(MAP_RO);
			memcpy(data, vbuffer, oldSize);
			memcpy(data + oldSize, vertices, appendSize);
		UnmapVBO();

		glBufferData(GL_ARRAY_BUFFER, newSize, data, GL_DYNAMIC_DRAW);
		VertexCount += vertexCount;
		if(newSize > 65536) free(data);
	}

	/**
	 * @brief Destroys the VertexIndexBuffer if needed
	 */
	VertexIndexBuffer::~VertexIndexBuffer()
	{
		if(IBO) 
		{
			if(this == CurrentIndexBuffer) CurrentIndexBuffer = NULL;
			glDeleteBuffers(1, &IBO);
		}

	}

	/**
	 * @brief Copying actually produces move semantics
	 */
	VertexIndexBuffer& VertexIndexBuffer::operator=(VertexIndexBuffer& vib)
	{
		if(this != &vib)
		{
			VertexBuffer::operator=(vib);
			IBO = vib.IBO, IndexCount = vib.IndexCount;
			vib.IBO = 0, vib.IndexCount = 0;
		}
		return *this;
	}


	/**
	 * @brief Destroys the VertexIndexBuffer
	 */
	void VertexIndexBuffer::Destroy()
	{
		VertexBuffer::Destroy(); // we need to also destroy the VertexBuffer
		if(IBO)
		{
			glDeleteBuffers(1, &IBO); // delete IBO
			IBO = 0;
			IndexCount = 0;
		}
	}


	/**
	 * @brief Draws this VertexIndexBuffer
	 */
	void VertexIndexBuffer::Draw() const
	{
		_Bind(); // we're using auto-bind now
		BindIndexBuffer(); // bind index buffer (if needed)
		// draw by index, no additional data (NULL)
		glDrawElements(Style, IndexCount, GL_UNSIGNED_INT, NULL);
	}


	/**
	 * @brief Draws a segment of the VertexIndexBuffer on the screen. Assumes Bind() has been called.
	 * @param start Starting offset index
	 * @param count Number of elements to draw
	 */
	void VertexIndexBuffer::Draw(int start, int count) const
	{
		_Bind(); // we're using auto-bind now
		BindIndexBuffer(); // bind index buffer (if needed)
		int available = IndexCount - start;
		if(count > available) count = available;
		if(count) glDrawRangeElements(Style, start, IndexCount, count, GL_UNSIGNED_INT, NULL);
	}


	/**
	 * @brief Maps this IndexBuffer into client memory
	 * @param mapType Type of mapping to use R/W/RW
	 * @note UnmapIBO must be called after MapIBO()
	 * @return Mapped address to client memory
	 */
	void* VertexIndexBuffer::MapIBO(BufferMapType mapType)
	{
		BindIndexBuffer();
		GLenum access[] = { GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE };
		return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, access[mapType]);
	}


	/**
	 * @brief Unmaps the IndexBuffer from client memory
	 * @note Must be called after MapIBO()
	 */
	void VertexIndexBuffer::UnmapIBO()
	{
		BindIndexBuffer();
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	}


	/**
	 * @brief Buffers indices into the index buffer
	 * @param indices Array of indices to buffer into the IBO
	 * @param indexCount Number of indices in the buffer
	 */
	void VertexIndexBuffer::BufferIndices(const Index* indices, int indexCount)
	{
		if(!IBO)
			glGenBuffers(1, &IBO);
		else if(Type == BUFFER_STATIC)
			return; // can't change a static buffer
		if(!indices || !indexCount)
			return; // nothing to do here

		GLenum bufferType;
		switch(this->Type) {
			case BUFFER_DYNAMIC: bufferType = GL_DYNAMIC_DRAW; break;
			case BUFFER_STREAM: bufferType = GL_STREAM_DRAW; break;
			default: bufferType = GL_STATIC_DRAW; break;
		}

		IndexCount = indexCount;
		BindIndexBuffer(); // bind index buffer (if needed)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index)*indexCount, indices, bufferType);
	}


	/**
	 * @brief Buffers subdata insider the indexbuffer
	 * @param indices Array of indices to buffer into the IBO
	 * @param indexCount Number of indices in the array
	 * @param offsetIndex Offset index of the first index in IBO to replace
	 */
	void VertexIndexBuffer::BufferSubIndices(const Index* indices, int indexCount, int offsetIndex)
	{
		if(!IBO || Type == BUFFER_STATIC)
			return; // nothind to do here
		if(!indices || !indexCount)
			return; // nothing to do here

		throw "BufferSubIndices is unimplemented!";
	}




