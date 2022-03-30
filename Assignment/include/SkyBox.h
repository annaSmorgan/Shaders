#ifndef __SkyBox_H_
#define __SkyBox_H_

#include <glm/glm.hpp>


class SkyBox
{
public:

	SkyBox();
	virtual ~SkyBox();

	void Create();
	void Draw();
	void Destroy();

private:
	unsigned int cubeMapTexID;
	unsigned int SBVAO;
	unsigned int SBVBO;
	unsigned int SBProgramID;
};

#endif // _SkyBox.H_