// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 2, screenHeight / 2, -screenWidth / 2, screenHeight / 2);
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 4096;
	int shadowmapHeight = 4096;
	int sceneWidth = 100;
	int sceneHeight = 100;

	// This is your shadow map
	shadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Configure directional light
	light = new Light();
	light->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(0.0f, -0.7f, 0.7f);
	light->setPosition(posX,posY,posZ);
	light->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	light->generateProjectionMatrix(5, 180);

}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}
	
	return true;
}

bool App1::render()
{
	elapsedTime += timer->getTime();
	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	// Set the render target to be the render to texture.
	shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
	light->setDirection(dirX,dirY ,dirZ);
	light->setPosition(posX, posY, posZ);
	
	// get the world, view, and projection matrices from the camera and d3d objects.
	light->generateViewMatrix();
	XMMATRIX lightViewMatrix = light->getViewMatrix();
	//XMMATRIX lightProjectionMatrix = light->getOrthoMatrix();
	XMMATRIX lightProjectionMatrix = light->getProjectionMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	//render cube
	//worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(10.f, 10.f, 5.f);
	XMMATRIX rotMatrix1 = XMMatrixRotationRollPitchYaw(0.0, elapsedTime, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotMatrix1);
	
	cube->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());
	
	//render sphere
	worldMatrix = XMMatrixTranslation(15.f, 15.f, 5.f);
	XMMATRIX rotMatrix2 = XMMatrixRotationRollPitchYaw(0.0, elapsedTime, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotMatrix2);

	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	//stops zbuffer porblem
	worldMatrix = XMMatrixTranslation(10.f, 10.f, 5.f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix(); 
	XMMATRIX viewMatrix = camera->getViewMatrix();   
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	

	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	//render cube
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(10.f, 10.f, 5.f);
	XMMATRIX rotMatrix1 = XMMatrixRotationRollPitchYaw(0.0, elapsedTime, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotMatrix1);
	
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	worldMatrix = XMMatrixTranslation(15.f, 15.f, 5.f);
	XMMATRIX rotMatrix2 = XMMatrixRotationRollPitchYaw(0.0, elapsedTime, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotMatrix2);

	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	worldMatrix = XMMatrixTranslation(10.f, 10.f, 5.f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);

	//render 2d orto mesh shadow map
	renderer->setZBuffer(false);
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowMap->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	gui();
	renderer->endScene();
}



void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::SliderFloat("light PosX", &posX, -50,50, "%.3f");
	ImGui::SliderFloat("light PosY", &posY, -50,50, "%.3f");
	ImGui::SliderFloat("light PosZ", &posZ, -50,50, "%.3f");

	ImGui::SliderFloat("light dirX", &dirX, -50, 50, "%.3f");
	ImGui::SliderFloat("light dirY", &dirY, -50, 50, "%.3f");
	ImGui::SliderFloat("light dirZ", &dirZ, -50, 50, "%.3f");
	

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

