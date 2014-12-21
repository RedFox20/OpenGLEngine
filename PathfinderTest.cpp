/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "PathfinderTest.h"

#include "Input.h"
#include "Timer.h"
#include "GuiObject.h"
#include "FreeType.h"

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

static void OnMouseButton(int button, bool down, bool doubleClick)
{
	if(!down) { // on mouse up
		Vector2 pos(mouseX() - WorldPos.x, gScreen.h - mouseY() - WorldPos.y);
		if(button == MBLEFT) 
		{
			if(Finder.SetStart(pos)) PathChanged = true;
		}
		else if(button == MBRIGHT) 
		{
			if(Finder.SetEnd(pos)) PathChanged = true;
		}
	}
}

Vector4* selection = NULL;
static void OnMouseMove(int relX, int relY, int relZ)
{
	static Vector4 sel;
	if(relX || relY)
	{
		Vector2 pos(mouseX() - WorldPos.x, gScreen.h - mouseY() - WorldPos.y);
		Vector2i vpos = Finder.ToVirtualCoord(pos);

#if USE_QTREE
		if(AstarQNode* qnode = Finder.grid.Qtree->Get(vpos.x, vpos.y))
		{
			sel.set((float)qnode->x, (float)qnode->y, (float)qnode->size, (float)qnode->size);
			sel *= Finder.CellSize;
			selection = &sel;
		}
		else selection = NULL;
#else
		if(AstarNode* node = Finder.grid.Get(vpos.x, vpos.y))
		{
			sel.set(node->X, node->Y, 1.0f, 1.0f);
			sel *= Finder.CellSize;
			selection = &sel;
		}
		else selection = NULL;

#endif
	}
}




void PathfinderStressTest()
{
	int width = Finder.grid.Width;
	int height = Finder.grid.Height;

#if USE_PFLIST
	const wchar_t* container = L"PfList";
	PfVector<Vector2> path;
#elif USE_PFVECTOR
	const wchar_t* container = L"PfVector";
	PfVector<Vector2> path;
#else
	const wchar_t* container = L"std::vector";
	vector<Vector2> path;
#endif

	Finder.SetStart(0, 0);
	int opens = 0;
	int reopens = 0;
	double pfElapsed = Timer::Measure([&]() {
		for(int x = 0; x < width; ++x)
		{
			for(int y = 0; y < height; ++y)
			{
				Finder.SetEnd(x, y);
				if(Finder.Start && Finder.End)
				{
					Finder.Process(path, NULL);
					path.clear();
					opens += Finder.NumOpened;
					reopens += Finder.NumReopened;
				}
			}
		}
	});

	int tilesPerSecond = int((1.0 / pfElapsed) * opens);
	PathfinderSTText.CreateF(MonoFont,
		L"A* stress-test:\n"
		L"  <%s>\n"
		L"  millis  %dms\n"
		L"  tiles/s %d\n"
		L"  opens   %d\n"
		L"  reopens %d\n",
		container, int(pfElapsed*1000), tilesPerSecond, opens, reopens);
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
	for(uint i = 6; i < Finder.grid.NumPlanes; ++i)
		planeColors.emplace_back(randcolor(), randcolor(), randcolor(), 0.8f);

	GLDraw gridOverlay;

#if USE_QTREE
	Vector4 coolblue(0.0f, 0.45f, 0.77f, 0.66f);

	Vector4 pink(0.79f, 0.25, 0.41f, 0.2f);
	Vector2 size(CELLSIZE, CELLSIZE);
	double tOverlay = Timer::Measure([&](){
		int count = world.width * world.height;
		for(int i = 0; i < count; ++i)
		{
			AstarNode& node = Finder.grid.Nodes[i];
			Vector2 pos(node.X * CELLSIZE, node.Y * CELLSIZE);
			gridOverlay.FillRect(pos, size, planeColors[node.Plane]);
			if(node.Plane != 1)
					gridOverlay.RectAA(pos, size, planeColors[node.Plane], 1.0f);
		}
		vector<AstarQNode*> qnodes;
		Finder.grid.Qtree->GetAllQNodes(qnodes);
		for(AstarQNode* q : qnodes)
		{
			Vector2 pos(q->x * CELLSIZE, q->y * CELLSIZE);
			Vector2 size(q->size * CELLSIZE, q->size * CELLSIZE);
			gridOverlay.RectAA(pos, size, coolblue, 2.0f);
		}
		size_t numQTreeNodes = qnodes.size();
		GridOverlay.Create(gridOverlay);
	});
#else
	Vector2 origin(Vector2::ZERO); 
	Vector2 size(CELLSIZE, CELLSIZE);
	double tOverlay = Timer::Measure([&](){
		for(int x = 0; x < world.width; x++)
		{
			for(int y = 0; y < world.height; y++)
			{
				AstarNode& node = Finder.grid.Nodes[x * world.width + y];
				gridOverlay.FillRect(origin, size, planeColors[node.Plane]);
				if(node.Plane != 1) // no grid for obstructed areas
					gridOverlay.RectAA(origin, size, Vector4(planeColors[node.Plane].rgb * 2.0f, 0.5f)); // brighter transparent
				origin.y += CELLSIZE;
			}
			origin.x += CELLSIZE;
			origin.y = 0.0f;
		}
		GridOverlay.Create(gridOverlay);
	});
#endif


	printf("Num Planes: %d\n", Finder.grid.NumPlanes);
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
	GridMinText.Create(MonoFont, L"0, 0");
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


void PathfinderTest::DrawScene(TextShader2D* ts, ColorShader2D* gui, const Matrix4& projection)
{
	float X = WorldPos.x = ((gScreen.w - WorldSize.x - Finder.CellHalfSize) / 2);
	float Y = WorldPos.y = ((gScreen.h - WorldSize.y - Finder.CellHalfSize) / 2);

	if(PathChanged && Finder.Start && Finder.End)
	{
#if USE_PFLIST
		PfVector<Vector2> path;
		PfVector<Vector2> explored;
#elif USE_PFVECTOR
		PfVector<Vector2> path;
		PfVector<Vector2> explored;
#else
		vector<Vector2> path;
		vector<Vector2> explored;
#endif
		Timer time(tstart);
		double pfElapsed = Timer::Measure([&]() {
			if(true) // debug
				Finder.Process(path, &explored);
			else
				Finder.Process(path, NULL);
		});
		PathChanged = false;

		PathfinderDebugOverlay.Destroy();
		GLDraw debugOverlay;
		int numLinks = 0;

		if(!explored.empty())
			for(auto it = explored.begin(); it != explored.end();) // green 'explored' lines
				debugOverlay.LineAA(*it++, *it++, GreenExplored, 1.0f);

		if(!path.empty())
			for(auto a = path.begin(), b = ++path.begin(); b != path.end(); ++numLinks) // red 'path' lines
				debugOverlay.LineAA(*a++, *b++, RedPath, 3.0f);
		PathfinderDebugOverlay.Create(debugOverlay);
		//debugOverlay.Clear();

		PathfinderDebugText.CreateF(MonoFont, 
			L"A* pathfinder\n"
			L"  millis  %dms\n"
			L"  micros  %dus\n"
			L"  opens   %d\n"
			L"  reopens %d\n"
			L"  links   %d\n",
			int(pfElapsed*1000),int(pfElapsed*1000000), Finder.NumOpened, Finder.NumReopened, numLinks);
	}

	gui->Bind(); // overlay graphics
	{
		GridOverlay.SetPosition(WorldPos);
		GridOverlay.Draw(projection);
		PathfinderDebugOverlay.SetPosition(WorldPos);
		PathfinderDebugOverlay.Draw(projection);
		if(Finder.Start) {
			StartMarker.SetPosition(WorldPos + Finder.ToScreenCoord(Finder.Start));
			StartMarker.Draw(projection);
		}
		if(Finder.End) {
			EndMarker.SetPosition(WorldPos + Finder.ToScreenCoord(Finder.End));
			EndMarker.Draw(projection);
		}
		if(selection) {
			GLDraw selMarker;
			selMarker.RectAA(selection->xy, selection->zw, Vector4(1.0f, 0.2f, 0.2f, 0.66f), 2.0f);
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