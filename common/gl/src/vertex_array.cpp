#include <algorithm>
#include <array>
#include <gl/vertex_array.h>

#include "gl/glew.h"

namespace gl
{
    VertexArray::~VertexArray()
    {
		FreeVao();
    }

    void VertexArray::GenerateVao()
    {
        glGenVertexArrays(1, &vao_);
    }

    void VertexArray::FreeVao()
    {
        if(vao_)
        {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
    }

    Quad::Quad(glm::vec2 size, glm::vec2 offset) : size_(size), offset_(offset)
    {
    }

    Quad::~Quad()
    {
		FreeBuffers();
    }

    void Quad::Init()
    {

		const glm::vec2 vertices[4] = {
			glm::vec2(0.5f, 0.5f) * size_ + offset_,  // top right
			glm::vec2(0.5f, -0.5f) * size_ + offset_,  // bottom right
			glm::vec2(-0.5f, -0.5f) * size_ + offset_,  // bottom left
			glm::vec2(-0.5f, 0.5f) * size_ + offset_   // top left
		};

		const glm::vec2 texCoords[4] = {
				glm::vec2(1.0f, 1.0f),      // top right
				glm::vec2(1.0f, 0.0f),   // bottom right
				glm::vec2(0.0f, 0.0f),   // bottom left
				glm::vec2(0.0f, 1.0f),   // bottom left
		};

		constexpr glm::vec3 back(0, 0, -1);
		constexpr glm::vec3 normals[4] = {
			back,
			back,
			back,
			back
		};

		std::array<glm::vec3, 4> tangent{};

		{
			const glm::vec2 edge1(vertices[1] - vertices[0]);
			const glm::vec2 edge2(vertices[2] - vertices[0]);
			const glm::vec2 deltaUV1 = texCoords[1] - texCoords[0];
			const glm::vec2 deltaUV2 = texCoords[2] - texCoords[0];

			const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			tangent[0].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent[0].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent[0].z = 0.0f;
		}
		std::fill(tangent.begin() + 1, tangent.end(), tangent[0]);

		constexpr unsigned int indices[6] = {
			// note that we start from 0!
			0, 1, 3,   // first triangle
			1, 2, 3    // second triangle
		};
        GenerateVao();
		//Initialize the EBO program
		glGenBuffers(4, &vbo_[0]);
		glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);
		// 2. copy our vertices array in a buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
		glEnableVertexAttribArray(0);
		//bind texture coords data
		glBindBuffer(GL_ARRAY_BUFFER, vbo_[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		// bind normals data
		glBindBuffer(GL_ARRAY_BUFFER, vbo_[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(2);
		// bind tangent data
		glBindBuffer(GL_ARRAY_BUFFER, vbo_[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tangent), &tangent[0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(3);
		//bind EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glBindVertexArray(0);
    }

    void Quad::Destroy()
    {
		FreeBuffers();
		FreeVao();
    }

    void Quad::Draw()
    {
		glBindVertexArray(vao_);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void Quad::FreeBuffers()
    {
		if(ebo_ != 0)
		{
			glDeleteBuffers(4, &vbo_[0]);
			glDeleteBuffers(1, &ebo_);
		}
    }

    Cuboid::Cuboid(glm::vec3 size, glm::vec3 offset) : size_(size), offset_(offset)
    {
    }

    Cuboid::~Cuboid()
    {
		FreeBuffers();
    }

    void Cuboid::Init()
    {

		glm::vec3 position[36] =
		{
			//Right face 
			 glm::vec3(0.5f,   0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(0.5f,  -0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(0.5f,   0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(0.5f,  -0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(0.5f,   0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(0.5f,  -0.5f ,  0.5f) * size_ + offset_,
			 //Left face                 *size_+offset_
			 glm::vec3(-0.5f,  0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(-0.5f,  0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(-0.5f, -0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(-0.5f, -0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(-0.5f, -0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(-0.5f,  0.5f ,  0.5f) * size_ + offset_,
			 //Top face                  *size_+offset_
			 glm::vec3(-0.5f,  0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(0.5f,  0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(0.5f,  0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(0.5f,  0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(-0.5f,  0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(-0.5f,  0.5f ,  0.5f) * size_ + offset_,
			 //Bottom fa                 *size_+offset_
			 glm::vec3(-0.5f, -0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(0.5f, -0.5f , -0.5f) * size_ + offset_,
			 glm::vec3(0.5f, -0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(0.5f, -0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(-0.5f, -0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(-0.5f, -0.5f , -0.5f) * size_ + offset_,
			 //Front fac                 *size_+offset_
			 glm::vec3(-0.5f, -0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(0.5f, -0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(0.5f,  0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(0.5f,  0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(-0.5f,  0.5f ,  0.5f) * size_ + offset_,
			 glm::vec3(-0.5f, -0.5f ,  0.5f) * size_ + offset_,
			 //Back face
			glm::vec3(-0.5f , -0.5f , -0.5f) * size_ + offset_,
			glm::vec3(0.5f ,  0.5f , -0.5f) * size_ + offset_,
			glm::vec3(0.5f , -0.5f , -0.5f) * size_ + offset_,
			glm::vec3(0.5f ,  0.5f , -0.5f) * size_ + offset_,
			glm::vec3(-0.5f , -0.5f , -0.5f) * size_ + offset_,
			glm::vec3(-0.5f ,  0.5f , -0.5f) * size_ + offset_,
		};
		glm::vec2 texCoords[36] = {
			glm::vec2(1.0f, 0.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(0.0f, 0.0f),

			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 0.0f),

			glm::vec2(0.0f, 1.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(0.0f, 0.0f),

			glm::vec2(0.0f, 1.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(0.0f, 0.0f),
			glm::vec2(0.0f, 1.0f),

			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(0.0f, 0.0f),

			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f),
			glm::vec2(0.0f, 0.0f),
			glm::vec2(0.0f, 1.0f),
		};

		glm::vec3 normals[36] =
		{
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),

			glm::vec3(-1.0f, 0.0f, 0.0f),
			glm::vec3(-1.0f, 0.0f, 0.0f),
			glm::vec3(-1.0f, 0.0f, 0.0f),
			glm::vec3(-1.0f, 0.0f, 0.0f),
			glm::vec3(-1.0f, 0.0f, 0.0f),
			glm::vec3(-1.0f, 0.0f, 0.0f),

			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),

			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f),

			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),

			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),
		};

		glm::vec3 tangent[36]{};
		for (int i = 0; i < 36; i += 3)
		{
			const glm::vec3 edge1 = position[i + 1] - position[i];
			const glm::vec3 edge2 = position[i + 2] - position[i];
			const glm::vec2 deltaUV1 = texCoords[i + 1] - texCoords[i];
			const glm::vec2 deltaUV2 = texCoords[i + 2] - texCoords[i];

			const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			tangent[i].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent[i].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent[i].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			tangent[i + 1] = tangent[i];
			tangent[i + 2] = tangent[i];
		}
		GenerateVao();
		glGenBuffers(4, &vbo_[0]);

		glBindVertexArray(vao_);
		// position attribute
		glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);
		// texture coord attribute
		glBindBuffer(GL_ARRAY_BUFFER, vbo_[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);
		// normal attribute
		glBindBuffer(GL_ARRAY_BUFFER, vbo_[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(2);
		//tangent attribute
		glBindBuffer(GL_ARRAY_BUFFER, vbo_[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tangent), tangent, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(3);

		glBindVertexArray(0);
    }

	void Cuboid::Destroy()
	{
		FreeVao();
		FreeBuffers();
	}

    void Cuboid::Draw()
    {
		glBindVertexArray(vao_);
		glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void Cuboid::FreeBuffers()
    {
		glDeleteBuffers(4, &vbo_[0]);
    }
}
