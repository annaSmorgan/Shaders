#pragma once
#include <glm/glm.hpp>

class SVertex
{
public:

	enum VertexAttributeFlags
	{
		POSITION = (1 << 0),
		TEXCOORD1 = (1 << 1),
	};

	enum Offsets
	{
		PositionOffset = 0,
		TexCoord1Offset = PositionOffset + sizeof(glm::vec4),
	};

	SVertex();
	SVertex(const glm::vec4& a_position, const glm::vec2 a_uv);
	SVertex(const SVertex& a_v);
	~SVertex();

	glm::vec4	position;
	glm::vec2	texCoord1;

	bool operator == (const SVertex& a_rhs) const;
	bool operator < (const SVertex& a_rhs) const;
};

inline SVertex::SVertex()
	: position(0, 0, 0, 1),
	texCoord1(0, 0)
{

}

inline SVertex::SVertex(const glm::vec4& a_position, const glm::vec2 a_uv) : position(a_position), texCoord1(a_uv)
{

}

inline SVertex::SVertex(const SVertex& a_V) : position(a_V.position), texCoord1(a_V.texCoord1)
{

}

inline SVertex::~SVertex()
{

}

inline bool SVertex::operator == (const SVertex& a_rhs) const
{
	return memcmp(this, &a_rhs, sizeof(SVertex)) == 0;
}

inline bool SVertex::operator < (const SVertex& a_rhs) const
{
	return memcmp(this, &a_rhs, sizeof(SVertex)) < 0;
}
