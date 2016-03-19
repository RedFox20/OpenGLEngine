/**
 * Copyright (c) 2013 - Jorma Rebane
 * FreeType font engine test
 */
#include <cstdio>
#include <conio.h>
#include "gl\glew.h"
#include "gl\glut.h"
#include "GL\glm\gtx\transform.hpp" // perspective, lookAt

// engine specifics:
#include "shader/ShaderProgram.h"
#include "Timer.h"
#include <gui/GuiObject.h>
#include "GameObject.h"
#include "Input.h"
#include <gui/freetype.h>
using namespace freetype;
#include <pathfinder/PathfinderTest.h>


static const bool PathfinderTest = true;

/////////////////////  game objects

struct GameCube : public GameObject
{
	~GameCube()
	{
		Destroy();
	}

	virtual void Create() override
	{
		// in order to display UV-s correctly per face, we need a lot more verts
		Vertex3UV vertices[] = {
			// front face
			{ -1.0f, +1.0f, +1.0f,  0.0f, 1.0f }, // front upper-left
			{ -1.0f, -1.0f, +1.0f,  0.0f, 0.0f }, // front lower-left
			{ +1.0f, -1.0f, +1.0f,  1.0f, 0.0f }, // front lower-right
			{ +1.0f, +1.0f, +1.0f,  1.0f, 1.0f }, // front upper-right
			// back face
			{ +1.0f, +1.0f, -1.0f,  0.0f, 1.0f }, // back upper-right
			{ +1.0f, -1.0f, -1.0f,  0.0f, 0.0f }, // back lower-right
			{ -1.0f, -1.0f, -1.0f,  1.0f, 0.0f }, // back lower-left
			{ -1.0f, +1.0f, -1.0f,  1.0f, 1.0f }, // back upper-left
			// left face
			{ -1.0f, +1.0f, -1.0f,  0.0f, 1.0f }, // back upper-left
			{ -1.0f, -1.0f, -1.0f,  0.0f, 0.0f }, // back lower-left
			{ -1.0f, -1.0f, +1.0f,  1.0f, 0.0f }, // front lower-left
			{ -1.0f, +1.0f, +1.0f,  1.0f, 1.0f }, // front upper-left
			// right face
			{ +1.0f, +1.0f, +1.0f,  0.0f, 1.0f }, // front upper-right
			{ +1.0f, -1.0f, +1.0f,  0.0f, 0.0f }, // front lower-right
			{ +1.0f, -1.0f, -1.0f,  1.0f, 0.0f }, // back lower-right
			{ +1.0f, +1.0f, -1.0f,  1.0f, 1.0f }, // back upper-right
			// top face
			{ -1.0f, +1.0f, -1.0f,  0.0f, 1.0f }, // back upper-left
			{ -1.0f, +1.0f, +1.0f,  0.0f, 0.0f }, // front upper-left
			{ +1.0f, +1.0f, +1.0f,  1.0f, 0.0f }, // front upper-right
			{ +1.0f, +1.0f, -1.0f,  1.0f, 1.0f }, // back upper-right
			// bottom face
			{ -1.0f, -1.0f, +1.0f,  0.0f, 1.0f }, // front lower-left
			{ -1.0f, -1.0f, -1.0f,  0.0f, 0.0f }, // back lower-left
			{ +1.0f, -1.0f, -1.0f,  1.0f, 0.0f }, // back lower-right
			{ +1.0f, -1.0f, +1.0f,  1.0f, 1.0f }, // front lower-right
		};
		Index indices[] = {
			 0,  1,  2,    0,  2,  3, // front face
			 4,  5,  6,    4,  6,  7, // back face
			 8,  9, 10,    8, 10, 11, // left face
			12, 13, 14,   12, 14, 15, // right face
			16, 17, 18,   16, 18, 19, // top face
			20, 21, 22,   20, 22, 23, // bottom face
		};
		VertexIndexBuffer* b = new VertexIndexBuffer(Vertex3UV());
		b->BufferVertices(vertices, sizeof(vertices)/sizeof(Vertex3UV));
		b->BufferIndices(indices, sizeof(indices)/sizeof(Index));
		vbuffer = b;

		SetPosition(Vector3(2.f, 0.f, -2.f)); // Z - 2
		Rotate(Vector3(0.f, 1.f, 0.f), 45.f);
	}

	virtual void Destroy() override
	{
		if(vbuffer)
		{
			delete vbuffer;
			vbuffer = 0;
		}
	}
};


///////////////////// engine state, startup / shutdown initialization
static bool VSync = true;
static std::vector<GameObject*> GameObjects;
static std::vector<ShaderProgram*> Shaders;
static std::vector<Texture*> Textures;
static std::vector<FontFace*> FontFaces;
static std::vector<Font*> Fonts;
static std::vector<Text*> Texts;
static std::vector<GuiText*> GuiTexts;

Vector2 gScreen; // this is used globally, so don't hide it with 'static'
Vector2 gScreenCorrection; // GLUT doesn't report window client size, so we need a bit of correction

static unsigned SID_SimpleShader;
static unsigned SID_TextShader2D;
static unsigned SID_ColorShader2D;
static unsigned SID_SDFTextShader2D;

bool startup()
{
	if (!GLEW_VERSION_3_1) {
		fprintf(stderr, "Error: your graphics card does not support OpenGL 3.1!\n");
		return false;
	}

	if (PathfinderTest)
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // clear background to soft black
	else
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // clear background to soft black
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // pure white background
	glEnable(GL_BLEND); // enable alpha mapping
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// load shaders
	Shaders.push_back(new ShaderProgram("simple.vp.hlsl", "simple.fp.hlsl"));
	Shaders.push_back(new ShaderProgram("guitext.vp.hlsl", "guitext.fp.hlsl"));
	Shaders.push_back(new ShaderProgram("guicolor.vp.hlsl", "guicolor.fp.hlsl"));
	Shaders.push_back(new ShaderProgram("sdftext.vp.hlsl", "sdftext.fp.hlsl"));
	for (ShaderProgram* shader : Shaders) if(!shader->Compile()) return false;
	SID_SimpleShader = 0;
	SID_TextShader2D = 1;
	SID_ColorShader2D = 2;
	SID_SDFTextShader2D = 3;
	
	// load textures
	Textures.push_back(new Texture("texture1.bmp"));

	// load objects
	GameObjects.push_back(new GameCube());
	for (GameObject* obj : GameObjects) 
	{	
		obj->SetTexture(Textures[0]);
		obj->Create();
	}
	Timer t1(tstart);

	// load fontfaces
	FontFaces.push_back(new FontFace("fonts/veronascript.ttf"));
	FontFaces.push_back(new FontFace("fonts/arialblack.ttf"));
	FontFaces.push_back(new FontFace("fonts/combust_i.ttf"));
	FontFaces.push_back(new FontFace("fonts/Anonymous_Pro.ttf"));

	Fonts.push_back(FontFaces[0]->NewFont(48, FONT_STROKE, 3.0f));
	Fonts.push_back(FontFaces[1]->NewFont(32, FONT_OUTLINE, 1.5f));
	Fonts.push_back(FontFaces[1]->NewFont(24, FONT_STROKE, 1.5f));
	Fonts.push_back(FontFaces[2]->NewFont(32, FONT_SHADOW, 2.0f));
	Fonts.push_back(FontFaces[3]->NewFont(12, FONT_SHADOW, 1.0f));
	Fonts.push_back(FontFaces[3]->NewFont(12));

	printf("Fonts loaded in: %.0fms\n", t1.StopElapsed() * 1000);
	
	t1.Start();
	auto create_text = [&](Font* font, float y, wchar_t* fmt) {
		GuiText* gt = new GuiText(font, fmt, font->atlas.Width(), font->atlas.Height());
		gt->SetPosition(5.0f, y);
		GuiTexts.push_back(gt);
		return gt;
	};
	create_text(Fonts[4], 20.0f, L"FPS: 0");
	create_text(Fonts[0], 60.0f, L"Font VeronaScript.ttf stroke 48px-atlas: %dx%dpx");
	create_text(Fonts[1], 140.0f, L"Font arialblack.ttf outline 32px-atlas: %dx%dpx");
	create_text(Fonts[2], 190.0f, L"Font arialblack.ttf stroke 24px-atlas: %dx%dpx");
	create_text(Fonts[3], 230.0f, L"Font Combust.ttf shadow 32px-atlas: %dx%dpx\nTTF defined newlines.");
	create_text(Fonts[4], 330.0f, L"Font Anonymous_Pro.ttf shadow 12px-atlas %dx%dpx");
	create_text(Fonts[5], 360.0f, L"Font Anonymous_Pro.ttf normal 12px-atlas %dx%dpx");
	create_text(Fonts[3], 446.0f, L"Rotate text.");

	printf("Text generated in: %.0fms\n", t1.StopElapsed() * 1000);

	static const Vector4 Shadow    = Vector4(0.05f, 0.05f, 0.05f, 1.0f);
	static const Vector4 White     = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	static const Vector4 GoldenRod = Vector4(0.854f, 0.647f, 0.125f, 1.0f);
	static const Vector4 Pink      = Vector4(0.894f, 0.368f, 0.615f, 1.0f);
	static const Vector4 DarkRed   = Vector4(0.55f, 0.05f, 0.05f, 1.0f);
	static const Vector4 SpaceBlue = Vector4(0.172f, 0.458f, 0.86f, 1.0f);
	GuiTexts[0]->SetColor(White, Shadow);
	GuiTexts[1]->SetColor(White, SpaceBlue);
	GuiTexts[2]->SetColor(GoldenRod, DarkRed);
	GuiTexts[3]->SetColor(GoldenRod, DarkRed);
	GuiTexts[4]->SetColor(Pink, Shadow);
	GuiTexts[5]->SetColor(White, Shadow);
	GuiTexts[6]->SetColor(White, Shadow);
	GuiTexts[7]->SetColor(Pink, Shadow);

	if (PathfinderTest) PathfinderTest::Create();
	return true;
}
void shutdown()
{
	if (PathfinderTest) PathfinderTest::Destroy();

	glutDestroyWindow(1);
	for (GameObject* obj : GameObjects)	   delete obj;
	for (ShaderProgram* shader : Shaders)  delete shader;
	for (Texture* texture : Textures)      delete texture;
	for (Font* font : Fonts)               delete font;
	for (FontFace* fontface : FontFaces)   delete fontface;
	for (Text* text : Texts)               delete text;
	for (GuiText* gtxt : GuiTexts)         delete gtxt;
	Shaders.clear();
	GameObjects.clear();
	Textures.clear();
	Fonts.clear();
	Texts.clear();
	GuiTexts.clear();

	exit(0); // exit the process
}




void OnKeyChange(int key, wchar_t keyChar, bool down, bool repeat)
{
	static bool DrawWireFrame = false;
	if (down && key == 'p')
	{
		if (DrawWireFrame = !DrawWireFrame) // toggle bool DrawWireFrame
		{
			glDisable(GL_BLEND);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else // filled mode:
		{
			glEnable(GL_BLEND);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
}

static float TextX = 10.0f, TextY = 10.0f;
static float TextS = 1.0f;


///////////////////// game loop 
void frame_enter(float deltaTime)
{
	static float time = 0.0f;
	static float avgDelta = deltaTime;
	avgDelta = (avgDelta + deltaTime) * 0.5f;
	if ((time += deltaTime) > 0.5f)
	{
		wchar_t buffer[48];
		swprintf(buffer, L"FPS: %d", int(1.0f / avgDelta));
		SetConsoleTitleW(buffer);
		time -= 0.5f;

		//GuiTexts[0]->Update(buffer, wcslen(buffer));
	}
	if (isKeyDown(KEY_ESCAPE))
		shutdown();

	if (isKeyDown('a')) TextX -= 100.0f * deltaTime, TextX = roundf(TextX);
	if (isKeyDown('d')) TextX += 100.0f * deltaTime, TextX = roundf(TextX);
	if (isKeyDown('w')) TextY -= 100.0f * deltaTime, TextY = roundf(TextY);
	if (isKeyDown('s')) TextY += 100.0f * deltaTime, TextY = roundf(TextY);

	Vector3 rot;
	if (isKeyDown('z')) rot.z += 1.0f; // roll left
	if (isKeyDown('c')) rot.z -= 1.0f; // roll right
	if (isKeyDown('q')) rot.y += 1.0f; // rotate left
	if (isKeyDown('e')) rot.y -= 1.0f; // rotate right
	for (GameObject* obj : GameObjects) // apply rotation for any and all objects
		if (rot.length() > 0.0001f) 
			obj->Rotate(rot, deltaTime * 180.0f); // 180* per second


	if (isKeyDown('+')) TextS += 0.25f * deltaTime;
	if (isKeyDown('-')) TextS -= 0.25f * deltaTime;
	GuiTexts[0]->Rotate(90.0f * deltaTime);
}


bool spare_time(const SpareTime& spareTime)
{
	// we are waiting for vsync and have some extra time to waste

	return false; // we didn't do anything
}


static Timer GameTimer(tstart);   // for measuring actual game logic timing
static float AvgGameTime = 0.0;  // average time for every frame we processed


float vsync_frame()
{
	static const float VSyncInterval = (1.0f / 61.0f);
	static Timer VSyncTimer(tstart);

	if (VSync && AvgGameTime < VSyncInterval) // should we do VSync at all?
	{
		for (;;)
		{
			float timeUntilVSync = VSyncInterval - float(VSyncTimer.StopElapsed());
			if (timeUntilVSync < 0.0011f) // we still got at least 1ms of time left
				break;

			//printf("vsync dt %.1fms\n", timeUntilVSync * 1000);

			if (spare_time(timeUntilVSync)) // did we utilize spare time?
				continue;
			Sleep(1); // smallest possible sleep on Win32
		}
	}
	float deltaTime = (float)VSyncTimer.StopElapsed();
	VSyncTimer.Start(); // mark new synctimer spot
	return deltaTime;
}

void frame_start()
{
	// we use this section to limit FPS to 60
	float deltaTime = vsync_frame(); // perform vsync if needed

	GameTimer.Start();
	frame_enter(deltaTime);

	// can't believe GLUT is a buggy POS and it renders into window itself, not its client area
	// for that reason we need to do some screen size corrections
	// ultimately we'll have to switch to native windows instead
	Vector2 ortho{ (float)glutGet(GLUT_WINDOW_WIDTH), (float)glutGet(GLUT_WINDOW_HEIGHT) };
	gScreen.w = ortho.w - gScreenCorrection.w;
	gScreen.h = ortho.h - gScreenCorrection.h;


	// Clear Screen & Prepare 3D
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // enable depth testing
	glEnable(GL_CULL_FACE); // enable face culling

	// create the 3D view-projection matrix
	Matrix4 ViewProjection = 
		glm::perspective(45.0f, gScreen.w / gScreen.h, 0.1f, 10000.0f) *
		glm::lookAt<float>(Vector3(0, 0, 3), Vector3(0, 0, 0), Vector3(0, 1, 0));

	if (!PathfinderTest)
	{
		Shaders[SID_SimpleShader]->Bind();
		for (const GameObject* obj : GameObjects)
			obj->Draw(ViewProjection);
	}

	glDisable(GL_DEPTH_TEST); // disable depth testing
	glDisable(GL_CULL_FACE); // disable face culling

	
	ViewProjection = glm::ortho(0.0f, ortho.w, 0.0f, ortho.h, -1.0f, 1.0f);
	{
		if (PathfinderTest)
		{
			PathfinderTest::DrawScene(Shaders[SID_TextShader2D], Shaders[SID_ColorShader2D], ViewProjection);
		}
		else
		{
			for (GuiText* text : GuiTexts)
			{
				ShaderProgram* shader = Shaders[text->Font()->is_sdf ? SID_SDFTextShader2D : SID_TextShader2D];
				shader->HotLoad(); // perform hotloading if needed
				shader->Bind();

				Vector2 pos = text->Pos();
				Vector2 scale = text->Scale();
				text->SetPosition(pos.x + TextX, gScreen.h - pos.y - TextY);
				text->SetScale(scale * TextS);

				text->Draw(ViewProjection);
				
				text->SetPosition(pos);
				text->SetScale(scale);
			}
		}
	}

	glutSwapBuffers(); // display the result
	AvgGameTime = (AvgGameTime + (float)GameTimer.StopElapsed()) * 0.5f;
}

////////////////

int main(int argc, char** argv)
{
	if (_wchdir(L"data") == -1 && _wchdir(L"bin/data")) // working dir to the data dir
	{
		fprintf(stderr, "Failed to find data/ or bin/data dir\n");
		system("pause");
		return -1;
	}

	glutInit(&argc, argv);
	glutReshapeFunc(&window_reshaped);
	glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);

	gScreen.set(1280, 720);
	RECT desired = { 0, 0, (int)gScreen.w, (int)gScreen.h };
	AdjustWindowRect(&desired, WS_OVERLAPPEDWINDOW, 0);
	Vector2i realSize(desired.right - desired.left, desired.bottom - desired.left);
	gScreenCorrection.set((float)-desired.left, (float)-desired.left);
	glutInitWindowSize(realSize.width, realSize.height);
	glutCreateWindow("GL-Engine by Jorma Rebane");

	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK)
	{
		fprintf(stderr, "glewInit error: %s\n", glewGetErrorString(glew_status));
		return -1;
	}
	if (startup())
	{
		glutIdleFunc(frame_start);
		glutDisplayFunc(frame_start);

		Input::BindGLUT();
		Input::AddKeyChange(OnKeyChange);
		
		try 
		{
			TimeSampler::NextSample();
			glutMainLoop(); // exceptions could be thrown
		}
		catch (std::exception ex)
		{
			printf("Exception: %s\n", ex.what());
		}
		catch (const char* err)
		{
			printf("Exception: %s\n", err);
		}
	}
	glutHideWindow();
	glutDestroyWindow(1);
	printf("Press any key to exit...");
	getch();
	return 0;
}