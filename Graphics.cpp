#include "Graphics.h"


Graphics::Graphics()
{
}


Graphics::~Graphics()
{
}



bool Graphics::Create(HWND window, int width, int height, bool fullscreen)
{
	return true;
}
void Graphics::Destroy()
{

}


bool Graphics::IsBusyResizing() const
{
	return false;
}
bool Graphics::ResizeViewport(int width, int height)
{
	return true;
}