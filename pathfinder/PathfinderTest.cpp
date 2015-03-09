/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "PathfinderTest.h"
#include "Input.h"
#include "Timer.h"
#include <gui/GuiObject.h>
#include <gui/freetype.h>
#include <vector>
using std::vector;
#include "PathfinderAstar.h"

static GuiOverlay GridOverlay;
static GuiOverlay StartMarker;
static GuiOverlay EndMarker;
static GuiOverlay PathfinderDebugOverlay;
static GuiText PathfinderDebugText;
static GuiText PathfinderSTText;
static GuiText GridMinText;
static GuiText GridMaxText;

static Vector2 WorldSize;
static Vector2 WorldPos;
static PathfinderAstar Finder;
static bool PathChanged = false;

static Vector4 RedPath(1.0f, 0.05f, 0.05f, 0.75f);
static Vector4 GreenExplored(0.05f, 0.8f, 0.05f, 0.75f);
static Vector3 Indigo(0.29f, 0.0f, 0.51f);				// Indigo (blue-violet)
static Vector3 IndigoBlue(0.0f, 0.26f, 0.41f);			// Indigo Dye (soft dark blue)
static Vector3 SlateBlue(0.28f, 0.35f, 0.67f);			// Dark Slate Blue (dark soft violet-blue)


static freetype::FontFace* MonoFace = NULL;
static freetype::Font* MonoFont = NULL;
static Vector4* Selection = NULL;

static void OnMouseButton(int button, bool down, bool doubleClick)
{
	if (!down) // on mouse up
	{
		Vector2 mouse((float)mouseX(), gScreen.h - mouseY() - 1);
		Vector2 pos = mouse - WorldPos;
		if (button == MBLEFT) 
		{
			if (Finder.SetStart(pos)) PathChanged = true;
		}
		else if (button == MBRIGHT) 
		{
			if (Finder.SetEnd(pos)) PathChanged = true;
		}
	}
}

static void OnMouseMove(int relX, int relY, int relZ)
{
	static Vector4 sel;
	if (relX || relY)
	{
		Vector2 mouse((float)mouseX(), gScreen.h - mouseY() - 1);
		Vector2 pos = mouse - WorldPos;
		Vector2i vpos = Finder.ToVirtualCoord(pos);
		printf("mx %.0f, my %.0f\n", mouse.x, mouse.y);
		printf("px %.0f, py %.0f\n", pos.x, pos.y);
		printf("vx %d, vy %d\n", vpos.x, vpos.y);

		if (AstarNode* node = Finder.Grid.get(vpos.x, vpos.y))
		{
			sel.set((float)node->X, (float)node->Y, 1.0f, 1.0f);
			sel *= Finder.CellSize;
			Selection = &sel;
		}
		else Selection = NULL;
	}
}




void PathfinderStressTest()
{
	const char* container = typeid(PfOpenList).name() + 7; // skip "struct "
	int width  = Finder.Grid.Width;
	int height = Finder.Grid.Height;
	PfVector<Vector2> path;
	Finder.SetStart(0, 0);
	int opens = 0;
	int reopens = 0;
#if _DEBUG
	const int iterations = 5;
#else
	const int iterations = 50;
#endif

	double pfElapsed = Timer::Measure([&]()
	{

		for (int i = 0; i < iterations; ++i)
		for (int x = 0; x < width;  ++x)
		for (int y = 0; y < height; ++y)
		{
			Finder.SetEnd(x, y);
			if (Finder.Start && Finder.End)
			{
				Finder.Process(path, NULL);
				path.clear();
				opens   += Finder.NumOpened;
				reopens += Finder.NumReopened;
			}
		}
	});

	int tilesPerSecond = int((1.0 / pfElapsed) * opens);
	PathfinderSTText.CreateF(MonoFont,
		L"A* stress-test:\n"
		L"  <%hs>\n"
		L"  millis   %dms\n"
		L"  tiles/s  %d\n"
		L"  opens    %d\n"
		L"  reopens  %d\n"
		L"  maxdepth %d\n",
		container, int(pfElapsed*1000), tilesPerSecond, opens, reopens, Finder.MaxDepth);
}






void PathfinderTest::Create()
{
	Input::AddMouseButton(&OnMouseButton);
	Input::AddMouseMove(&OnMouseMove);

	// load the world and initialize the pathfinder
	Image world("pathfinding.bmp");
	//Image world("pathfinder2.bmp");
	//const float CELLSIZE = 14.0f;
	const float CELLSIZE = 14.0f;
	double tCreate = Timer::Measure([&](){
		Finder.Create(CELLSIZE, world.Width(), world.Height(), world.Data());
	});
	printf("Pathfinder init: %fs\n", tCreate);

	WorldSize.set(world.width * CELLSIZE, world.height * CELLSIZE);

	// initialize the visual representation of the world
	vector<Vector4> planeColors;
	planeColors.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);		// plane 0 is plain BLACK
	planeColors.emplace_back(0.5f, 0.15f, 0.15f, 0.33f);	// plane 1 (obstruction) is DARK-RED
	planeColors.emplace_back(IndigoBlue, 0.33f);			// plane 2 is INDIGO DYE
	planeColors.emplace_back(0.55f, 0.15f, 0.55f, 0.33f);	// plane 3 is DARK PURPLE
	planeColors.emplace_back(0.55f, 0.55f, 0.15f, 0.33f);	// plane 4 is DARK YELLOW
	planeColors.emplace_back(0.15f, 0.55f, 0.55f, 0.33f);	// plane 5 is BLUE GREEN
	srand(12344); // so the colors always look the same
	auto randcolor = []() { return (32 + rand() % 128) / 255.0f; };
	for(uint i = 6; i < Finder.Grid.NumPlanes; ++i)
		planeColors.emplace_back(randcolor(), randcolor(), randcolor(), 0.8f);

	GLDraw gridOverlay;

	Vector2 origin(Vector2::ZERO); 
	Vector2 size(CELLSIZE, CELLSIZE);
	double tOverlay = Timer::Measure([&]()
	{
		for (int y = 0; y < world.height; ++y)
		{
			for (int x = 0; x < world.width; ++x)
			{
				origin.set(x * size.x, y * size.y);
				AstarNode& node = Finder.Grid.Nodes[y * world.width + x];
				gridOverlay.FillRect(origin, size, planeColors[node.Plane]);
				if (node.Plane != 1) // no grid for obstructed areas
					gridOverlay.RectAA(origin, size, Vector4(planeColors[node.Plane].rgb * 2.0f, 0.5f)); // brighter transparent
			}
		}

		GridOverlay.Create(gridOverlay);
	});


	printf("Num Planes: %d\n", Finder.Grid.NumPlanes);
	printf("Overlay init: %fs\n", tOverlay);

	// initialize start / end markers
	GLDraw target;
	target.FillRect(Vector2::ZERO, size, Vector4(0.05f, 0.25f, 0.5f, 0.66f)); // transparent blue
	target.RectAA(Vector2::ZERO, size, Vector4(0.05f, 0.25f, 0.5f, 1.0f)); // solid blue outline
	StartMarker.Create(target);
	target.Clear();
	target.FillRect(Vector2::ZERO, size, Vector4(0.9f, 0.9f, 0.4f, 0.66f)); // transparent yellow
	target.RectAA(Vector2::ZERO, size, Vector4(0.9f, 0.9f, 0.4f, 1.0f)); // solid yellow outline
	EndMarker.Create(target);

	// load fonts & generate text
	MonoFace = new freetype::FontFace("fonts/DejaVuSansMono.ttf");
	MonoFont = MonoFace->NewFont(10, freetype::FONT_SHADOW, 1.0f);
	GridMinText.CreateF(MonoFont, L"0, 0");
	GridMaxText.CreateF(MonoFont, L"%d, %d", world.width, world.height);

	PathfinderStressTest();
}
void PathfinderTest::Destroy()
{
	GridOverlay.Destroy();
	StartMarker.Destroy();
	EndMarker.Destroy();
	PathfinderDebugOverlay.Destroy();

	GridMinText.Destroy();
	GridMaxText.Destroy();
	PathfinderDebugText.Destroy();
	delete MonoFont;
	delete MonoFace;
	Finder.Destroy();
}


void PathfinderTest::DrawScene(ShaderProgram* ts, ShaderProgram* gui, const Matrix4& projection)
{
	//float X = WorldPos.x = 0;
	//float Y = WorldPos.y = 0;
	float X = WorldPos.x = ((gScreen.w - WorldSize.x) / 2);
	float Y = WorldPos.y = ((gScreen.h - WorldSize.y) / 2);

	if (PathChanged && Finder.Start && Finder.End)
	{
		PfVector<Vector2> path;
		PfVector<Vector2> explored;

		double pfElapsed = Timer::Measure([&]() {
			if (true) // debug
				Finder.Process(path, &explored);
			else
				Finder.Process(path, NULL);
		});
		PathChanged = false;

		PathfinderDebugOverlay.Destroy();
		GLDraw debugOverlay;
		int numLinks = 0;

		if (!explored.empty())
		{
			for (auto it = explored.begin(); it != explored.end(); it += 2) // green 'explored' lines
				debugOverlay.LineAA(it[0], it[1], GreenExplored, 1.0f);
		}

		if (!path.empty())
		{
			auto a = path.begin();
			auto b = path.begin() + 1;
			for (; b != path.end(); ++numLinks) // red 'path' lines
				debugOverlay.LineAA(*a++, *b++, RedPath, 3.0f);
		}

		PathfinderDebugOverlay.Create(debugOverlay);
		//debugOverlay.Clear();

		PathfinderDebugText.CreateF(MonoFont, 
			L"A* pathfinder\n"
			L"  millis  %dms\n"
			L"  micros  %dus\n"
			L"  opens   %d\n"
			L"  reopens %d\n"
			L"  links   %d\n",
			int(pfElapsed*1000), int(pfElapsed*1000000), Finder.NumOpened, Finder.NumReopened, numLinks);
	}

	gui->Bind(); // overlay graphics
	{
		GridOverlay.SetPosition(WorldPos);
		GridOverlay.Draw(projection);
		PathfinderDebugOverlay.SetPosition(WorldPos);
		PathfinderDebugOverlay.Draw(projection);
		if (Finder.Start) {
			StartMarker.SetPosition(WorldPos + Finder.ToScreenCoord(Finder.Start));
			StartMarker.Draw(projection);
		}
		if (Finder.End) {
			EndMarker.SetPosition(WorldPos + Finder.ToScreenCoord(Finder.End));
			EndMarker.Draw(projection);
		}
		if (Selection) {
			GLDraw selMarker;
			selMarker.RectAA(Selection->xy, Selection->zw, Vector4(1.0f, 0.2f, 0.2f, 0.66f), 2.0f);
			GuiOverlay selOvl; selOvl.Create(selMarker);
			selOvl.SetPosition(WorldPos);
			selOvl.Draw(projection);
		}
	}

	ts->Bind(); // text 
	{
		GridMinText.SetPosition(Vector2(X - GridMinText.Size().w, Y));
		GridMinText.Draw(projection);

		GridMaxText.SetPosition(Vector2(X + WorldSize.w, Y + WorldSize.h));
		GridMaxText.Draw(projection);

		PathfinderDebugText.SetPosition(Vector2(X - PathfinderDebugText.Size().w, Y + WorldSize.h));
		PathfinderDebugText.Draw(projection);

		PathfinderSTText.SetPosition(Vector2(10.0f, Y + WorldSize.h));
		PathfinderSTText.Draw(projection);
	}
}