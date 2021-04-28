#include "stdafx.h"
#include "Renderer.h"
#include "LoadPng.h"
#include <Windows.h>
#include <cstdlib>
#include <cassert>

Renderer::Renderer(int windowSizeX, int windowSizeY)
{
	//default settings
	glClearDepth(1.f);

	Initialize(windowSizeX, windowSizeY);
}


Renderer::~Renderer()
{
}

void Renderer::Initialize(int windowSizeX, int windowSizeY)
{
	//Set window size
	m_WindowSizeX = windowSizeX;
	m_WindowSizeY = windowSizeY;

	//Load shaders
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");
	m_FSSandBoxShader = CompileShaders("./Shaders/FSSandBox.vs", "./Shaders/FSSandBox.fs");
	
	//Create VBOs
	CreateVertexBufferObjects();

	//Initialize camera settings
	m_v3Camera_Position = glm::vec3(0.f, 0.f, 1000.f);
	m_v3Camera_Lookat = glm::vec3(0.f, 0.f, 0.f);
	m_v3Camera_Up = glm::vec3(0.f, 1.f, 0.f);
	m_m4View = glm::lookAt(
		m_v3Camera_Position,
		m_v3Camera_Lookat,
		m_v3Camera_Up
	);

	//Initialize projection matrix
	m_m4OrthoProj = glm::ortho(
		-(float)windowSizeX / 2.f, (float)windowSizeX / 2.f,
		-(float)windowSizeY / 2.f, (float)windowSizeY / 2.f,
		0.0001f, 10000.f);
	m_m4PersProj = glm::perspectiveRH(45.f, 1.f, 1.f, 1000.f);

	//Initialize projection-view matrix
	m_m4ProjView = m_m4OrthoProj * m_m4View; //use ortho at this time
	//m_m4ProjView = m_m4PersProj * m_m4View;

	//Initialize model transform matrix :; used for rotating quad normal to parallel to camera direction
	m_m4Model = glm::rotate(glm::mat4(1.0f), glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));

	float tempVertices[] = { 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f };
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices), tempVertices, GL_STATIC_DRAW);

	float tempVertices1[] = { 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 1.f, 0.f };
	glGenBuffers(1, &m_VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices1), tempVertices1, GL_STATIC_DRAW);

	float tempVertices2[] = {
		-0.5f, -0.5f, 0.f, 
		-0.5f, 0.5f, 0.f, 
		0.5f, 0.5f, 0.f,
	-0.5f, -0.5f, 0.f,
	0.5f, 0.5f, 0.f,
	0.5f, -0.5f, 0.f};
	glGenBuffers(1, &m_VBOFSSandBox);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOFSSandBox);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tempVertices2), tempVertices2, GL_STATIC_DRAW);

	CreateParticle(5000);
}

void Renderer::CreateVertexBufferObjects()
{
	float rect[]
		=
	{
		-0.5, -0.5, 0.f, -0.5, 0.5, 0.f, 0.5, 0.5, 0.f, //Triangle1
		-0.5, -0.5, 0.f,  0.5, 0.5, 0.f, 0.5, -0.5, 0.f, //Triangle2
	};

	glGenBuffers(1, &m_VBORect);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);
}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	//쉐이더 오브젝트 생성
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = (GLint)strlen(pShaderText);
	//쉐이더 코드를 쉐이더 오브젝트에 할당
	glShaderSource(ShaderObj, 1, p, Lengths);

	//할당된 쉐이더 코드를 컴파일
	glCompileShader(ShaderObj);

	GLint success;
	// ShaderObj 가 성공적으로 컴파일 되었는지 확인
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		//OpenGL 의 shader log 데이터를 가져옴
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

	// ShaderProgram 에 attach!!
	glAttachShader(ShaderProgram, ShaderObj);
}

bool Renderer::ReadFile(char* filename, std::string *target)
{
	std::ifstream file(filename);
	if (file.fail())
	{
		std::cout << filename << " file loading failed.. \n";
		file.close();
		return false;
	}
	std::string line;
	while (getline(file, line)) {
		target->append(line.c_str());
		target->append("\n");
	}
	return true;
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	GLuint ShaderProgram = glCreateProgram(); //빈 쉐이더 프로그램 생성

	if (ShaderProgram == 0) { //쉐이더 프로그램이 만들어졌는지 확인
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs, fs;

	//shader.vs 가 vs 안으로 로딩됨
	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		return -1;
	};

	//shader.fs 가 fs 안으로 로딩됨
	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		return -1;
	};

	// ShaderProgram 에 vs.c_str() 버텍스 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram 에 fs.c_str() 프레그먼트 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//Attach 완료된 shaderProgram 을 링킹함
	glLinkProgram(ShaderProgram);

	//링크가 성공했는지 확인
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
		// shader program 로그를 받아옴
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error linking shader program\n" << ErrorLog;
		return -1;
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error validating shader program\n" << ErrorLog;
		return -1;
	}

	glUseProgram(ShaderProgram);
	std::cout << filenameVS << ", " << filenameFS << " Shader compiling is done.\n";

	return ShaderProgram;
}
unsigned char * Renderer::loadBMPRaw(const char * imagepath, unsigned int& outWidth, unsigned int& outHeight)
{
	std::cout << "Loading bmp file " << imagepath << " ... " << std::endl;
	outWidth = -1;
	outHeight = -1;
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = NULL;
	fopen_s(&file, imagepath, "rb");
	if (!file)
	{
		std::cout << "Image could not be opened, " << imagepath << " is missing. " << std::endl;
		return NULL;
	}

	if (fread(header, 1, 54, file) != 54)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (header[0] != 'B' || header[1] != 'M')
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (*(int*)&(header[0x1E]) != 0)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	if (*(int*)&(header[0x1C]) != 24)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;
		return NULL;
	}

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	outWidth = *(int*)&(header[0x12]);
	outHeight = *(int*)&(header[0x16]);

	if (imageSize == 0)
		imageSize = outWidth * outHeight * 3;

	if (dataPos == 0)
		dataPos = 54;

	data = new unsigned char[imageSize];

	fread(data, 1, imageSize, file);

	fclose(file);

	std::cout << imagepath << " is succesfully loaded. " << std::endl;

	return data;
}

GLuint Renderer::CreatePngTexture(char * filePath)
{
	//Load Pngs: Load file and decode image.
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filePath);
	if (error != 0)
	{
		lodepng_error_text(error);
		assert(error == 0);
		return -1;
	}

	GLuint temp;
	glGenTextures(1, &temp);

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

	return temp;
}

GLuint Renderer::CreateBmpTexture(char * filePath)
{
	//Load Bmp: Load file and decode image.
	unsigned int width, height;
	unsigned char * bmp
		= loadBMPRaw(filePath, width, height);

	if (bmp == NULL)
	{
		std::cout << "Error while loading bmp file : " << filePath << std::endl;
		assert(bmp != NULL);
		return -1;
	}

	GLuint temp;
	glGenTextures(1, &temp);

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp);

	return temp;
}

void Renderer::CreateParticle(int count)
{
	float* particleVertices = new float[count * (3 + 3 + 1 + 1 + 1 + 1 + 1 + 4) * 3 * 2];
	int floatCount = count * (3 + 3 + 1) * 3 * 2;
	int vertexCount = count * 3 * 2;

	int index = 0;
	float particleSize = 0.01f;

	for (int i = 0; i < count; ++i)
	{
		float randomValueX = 0.f;
		float randomValueY = 1.f;
		float randomValueZ = 0.f;
		float randomValueVX = 1.f;
		float randomValueVY = 0.f;
		float randomValueVZ = 0.f;
		float randomEmitTime = 0.f;
		float randomLifeTime = 2.f;
		float randomPeriod = 1.f;
		float randomAmp = 1.f;
		float randomValue = 0;
		float randR = 0.f;
		float randG = 0.f;
		float randB = 0.f;
		float randA = 0.f;

		//randomValueX = ((float)rand() / (float)RAND_MAX - 0.5f) * 10.f;
		//randomValueY = ((float)rand() / (float)RAND_MAX - 0.5f) * 10.f;
		//randomValueZ = 0.f;
		randomValueVX = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.1f;
		randomValueVY = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.1f;
		randomValueVZ = 0.f;
		randomLifeTime = ((float)rand() / (float)RAND_MAX) * 1.f;
		randomEmitTime = ((float)rand() / (float)RAND_MAX) * 10.f;
		randomPeriod = ((float)rand() / (float)RAND_MAX) * 10.f + 1.f;
		randomAmp = ((float)rand() / (float)RAND_MAX) * 0.02f - 0.01f;
		randomValue = ((float)rand() / (float)RAND_MAX);
		randR = ((float)rand() / (float)RAND_MAX);
		randR = ((float)rand() / (float)RAND_MAX);
		randB = ((float)rand() / (float)RAND_MAX);
		randA = ((float)rand() / (float)RAND_MAX);

		particleVertices[index++] = -particleSize / 2.f + randomValueX;
		particleVertices[index++] = -particleSize / 2.f + randomValueY;
		particleVertices[index++] = 0.f;	//position
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.f;	//velocity
		particleVertices[index++] = randomEmitTime;	//emitTime
		particleVertices[index++] = randomLifeTime;	//lifeTime
		particleVertices[index++] = randomPeriod;	//emitTime
		particleVertices[index++] = randomAmp;	//lifeTime
		particleVertices[index++] = randomValue;
		particleVertices[index++] = randR;
		particleVertices[index++] = randG;
		particleVertices[index++] = randB;
		particleVertices[index++] = randA;

		particleVertices[index++] = particleSize / 2.f + randomValueX;
		particleVertices[index++] = -particleSize / 2.f + randomValueY;
		particleVertices[index++] = 0.f;
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.f;	//velocity
		particleVertices[index++] = randomEmitTime;	//emitTime
		particleVertices[index++] = randomLifeTime;	//lifeTime
		particleVertices[index++] = randomPeriod;	//emitTime
		particleVertices[index++] = randomAmp;	//lifeTime
		particleVertices[index++] = randomValue;
		particleVertices[index++] = randR;
		particleVertices[index++] = randG;
		particleVertices[index++] = randB;
		particleVertices[index++] = randA;

		particleVertices[index++] = particleSize / 2.f + randomValueX;
		particleVertices[index++] = particleSize / 2.f + randomValueY;
		particleVertices[index++] = 0.f;
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.f;	//velocity
		particleVertices[index++] = randomEmitTime;	//emitTime
		particleVertices[index++] = randomLifeTime;	//lifeTime
		particleVertices[index++] = randomPeriod;	//emitTime
		particleVertices[index++] = randomAmp;	//lifeTime
		particleVertices[index++] = randomValue;
		particleVertices[index++] = randR;
		particleVertices[index++] = randG;
		particleVertices[index++] = randB;
		particleVertices[index++] = randA;

		particleVertices[index++] = -particleSize / 2.f + randomValueX;
		particleVertices[index++] = -particleSize / 2.f + randomValueY;
		particleVertices[index++] = 0.f;
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.f;	//velocity
		particleVertices[index++] = randomEmitTime;	//emitTime
		particleVertices[index++] = randomLifeTime;	//lifeTime
		particleVertices[index++] = randomPeriod;	//emitTime
		particleVertices[index++] = randomAmp;	//lifeTime
		particleVertices[index++] = randomValue;
		particleVertices[index++] = randR;
		particleVertices[index++] = randG;
		particleVertices[index++] = randB;
		particleVertices[index++] = randA;

		particleVertices[index++] = particleSize / 2.f + randomValueX;
		particleVertices[index++] = particleSize / 2.f + randomValueY;
		particleVertices[index++] = 0.f;
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.f;	//velocity
		particleVertices[index++] = randomEmitTime;	//emitTime
		particleVertices[index++] = randomLifeTime;	//lifeTime
		particleVertices[index++] = randomPeriod;	//emitTime
		particleVertices[index++] = randomAmp;	//lifeTime
		particleVertices[index++] = randomValue;
		particleVertices[index++] = randR;
		particleVertices[index++] = randG;
		particleVertices[index++] = randB;
		particleVertices[index++] = randA;

		particleVertices[index++] = -particleSize / 2.f + randomValueX;
		particleVertices[index++] = particleSize / 2.f + randomValueY;
		particleVertices[index++] = 0.f;
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.f;	//velocity
		particleVertices[index++] = randomEmitTime;	//emitTime
		particleVertices[index++] = randomLifeTime;	//lifeTime
		particleVertices[index++] = randomPeriod;	//emitTime
		particleVertices[index++] = randomAmp;	//lifeTime
		particleVertices[index++] = randomValue;
		particleVertices[index++] = randR;
		particleVertices[index++] = randG;
		particleVertices[index++] = randB;
		particleVertices[index++] = randA;

		glGenBuffers(1, &m_VBOManyParticle);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatCount, particleVertices, GL_STATIC_DRAW);
		m_VBOManyParticleCount = vertexCount;
	}
}

void Renderer::Test()
{
	glUseProgram(m_SolidRectShader);

	GLuint VBOLocation = glGetAttribLocation(m_SolidRectShader, "Position");
	glEnableVertexAttribArray(VBOLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glVertexAttribPointer(VBOLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint VBOLocation1 = glGetAttribLocation(m_SolidRectShader, "Position1");
	glEnableVertexAttribArray(VBOLocation1);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO1);
	glVertexAttribPointer(VBOLocation1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	static float gscale = 0.f;
	GLint ScaleUniform = glGetUniformLocation(m_SolidRectShader, "u_Scale");
	glUniform1f(ScaleUniform, gscale);

	GLint colorUniform = glGetUniformLocation(m_SolidRectShader, "u_Color");
	glUniform4f(colorUniform, 1.f, gscale, 1.f, 1.f);

	GLint positionUniform = glGetUniformLocation(m_SolidRectShader, "u_Position");
	glUniform3f(positionUniform, -gscale, -gscale, 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);	//start rendering, primitive
	gscale += 0.01f;

	if (gscale > 1.f)
		gscale = 0.f;

	glDisableVertexAttribArray(VBOLocation);
	glDisableVertexAttribArray(VBOLocation1);
}

float g_Time = 0.f;
void Renderer::Particle()
{
	glUseProgram(m_SolidRectShader);

	GLuint VBOLocation = glGetAttribLocation(m_SolidRectShader, "a_Position");
	glEnableVertexAttribArray(VBOLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOLocation, 3, GL_FLOAT, 
		GL_FALSE, sizeof(float) * 15, (GLvoid*)0);

	GLuint VBOVLocation = glGetAttribLocation(m_SolidRectShader, "a_Velocity");
	glEnableVertexAttribArray(VBOVLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOVLocation, 3, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 3));

	GLuint VBOEmitLocation = glGetAttribLocation(m_SolidRectShader, "a_EmitTime");
	glEnableVertexAttribArray(VBOEmitLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOEmitLocation, 1, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 6));

	GLuint VBOLifeLocation = glGetAttribLocation(m_SolidRectShader, "a_LifeTime");
	glEnableVertexAttribArray(VBOLifeLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOLifeLocation, 1, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 7));

	GLuint VBOPLocation = glGetAttribLocation(m_SolidRectShader, "a_P");
	glEnableVertexAttribArray(VBOPLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOPLocation, 1, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 8));

	GLuint VBOALocation = glGetAttribLocation(m_SolidRectShader, "a_A");
	glEnableVertexAttribArray(VBOALocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOALocation, 1, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 9));

	GLuint VBOAValue = glGetAttribLocation(m_SolidRectShader, "a_randValue");
	glEnableVertexAttribArray(VBOAValue);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOAValue, 1, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 10));

	GLuint VBOColor = glGetAttribLocation(m_SolidRectShader, "a_Color");
	glEnableVertexAttribArray(VBOColor);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOManyParticle);
	glVertexAttribPointer(VBOColor, 4, GL_FLOAT,
		GL_FALSE, sizeof(float) * 15, (GLvoid*)(sizeof(float) * 11));

	GLint timeUniform = glGetUniformLocation(m_SolidRectShader, "u_Time");
	glUniform1f(timeUniform, g_Time);

	GLint forceUniform = glGetUniformLocation(m_SolidRectShader, "u_ExForce");
	glUniform3f(forceUniform, sin(g_Time), cos(g_Time), 0);

	glDrawArrays(GL_TRIANGLES, 0, m_VBOManyParticleCount);

	g_Time += 0.016;
}

void Renderer::FSSandBox()
{
	GLuint shader = m_FSSandBoxShader;
	glUseProgram(shader);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint VBOLocation = glGetAttribLocation(m_SolidRectShader, "a_Position");
	glEnableVertexAttribArray(VBOLocation);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOFSSandBox);
	glVertexAttribPointer(VBOLocation, 3, GL_FLOAT,
		GL_FALSE, sizeof(float) * 3, (GLvoid*)0);

	GLuint posUniform = glGetUniformLocation(shader, "u_Point");
	glUniform3f(posUniform, 0.5f, 0.5f, 0.1f);

	float points[] = {
	-0.5, -0.5, 0.01,
	-0.4, -0.4, 0.01 ,
	-0.3, -0.3, 0.01,
	-0.2, -0.2, 0.01,
	-0.1, -0.1, 0.01,
	0.5, 0.5, 0.01,
	0.4, 0.4, 0.01 ,
	0.3, 0.3, 0.01,
	0.2, 0.2, 0.01,
	0.1, 0.1, 0.01 };

	GLuint pointsUniform = glGetUniformLocation(shader, "u_Points");
	glUniform3fv(pointsUniform, 10, points);

	GLuint timeUniform = glGetUniformLocation(shader, "u_Time");
	glUniform1f(timeUniform, g_Time);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_BLEND);

	g_Time += 0.005;
}