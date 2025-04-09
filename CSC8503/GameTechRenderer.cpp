
#include "Win32Window.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include "GameTechRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"
#include "typeindex"
using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 4096

Matrix4 biasMatrix = Matrix::Translation(Vector3(0.5f, 0.5f, 0.5f)) * Matrix::Scale(Vector3(0.5f, 0.5f, 0.5f));

GameTechRenderer::GameTechRenderer(GameWorld& world) : OGLRenderer(*Window::GetWindow()), gameWorld(world) {
	glEnable(GL_DEPTH_TEST);

	debugShader = new OGLShader("debug.vert", "debug.frag");
	shadowShader = new OGLShader("shadow.vert", "shadow.frag");

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(1, 1, 1, 1);

	//Set up the light properties
	lightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	lightRadius = 1000.0f;
	lightPosition = Vector3(-200.0f, 60.0f, -200.0f);

	//Skybox!
	skyboxShader = new OGLShader("skybox.vert", "skybox.frag");
	skyboxMesh = new OGLMesh();
	skyboxMesh->SetVertexPositions({ Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	LoadSkybox();

	glGenVertexArrays(1, &lineVAO);
	glGenVertexArrays(1, &textVAO);

	glGenBuffers(1, &lineVertVBO);
	glGenBuffers(1, &textVertVBO);
	glGenBuffers(1, &textColourVBO);
	glGenBuffers(1, &textTexVBO);

	Debug::CreateDebugFont("PressStart2P.fnt", *LoadTexture("PressStart2P.png"));

	//Debug quad for drawing tex
	debugTexMesh = new OGLMesh();
	debugTexMesh->SetVertexPositions({ Vector3(-1, 1,0), Vector3(-1,-1,0) , Vector3(1,-1,0) , Vector3(1,1,0) });
	debugTexMesh->SetVertexTextureCoords({ Vector2(0, 1), Vector2(0,0) , Vector2(1,0) , Vector2(1,1) });
	debugTexMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	debugTexMesh->UploadToGPU();


	SetDebugStringBufferSizes(10000);
	SetDebugLineBufferSizes(1000);
	SetupImgui();
}

GameTechRenderer::~GameTechRenderer() {
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
}

void GameTechRenderer::SetupImgui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;


	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	const HWND windowHandle = NCL::Win32Code::Win32Window::windowHandle;
	ImGui_ImplWin32_InitForOpenGL(windowHandle);
	ImGui_ImplOpenGL3_Init();
}

void GameTechRenderer::LoadSkybox() {
	std::string filenames[6] = {
		"/Cubemap/skyrender0004.png",
		"/Cubemap/skyrender0001.png",
		"/Cubemap/skyrender0003.png",
		"/Cubemap/skyrender0006.png",
		"/Cubemap/skyrender0002.png",
		"/Cubemap/skyrender0005.png"
	};

	uint32_t width[6] = { 0 };
	uint32_t height[6] = { 0 };
	uint32_t channels[6] = { 0 };
	int flags[6] = { 0 };

	vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i) {
		TextureLoader::LoadTexture(filenames[i], texData[i], width[i], height[i], channels[i], flags[i]);
		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return;
		}
	}
	glGenTextures(1, &skyboxTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GameTechRenderer::RenderFrame() {
	glEnable(GL_CULL_FACE);
	glClearColor(1, 1, 1, 1);
	BuildObjectList();
	SortObjectList();
	RenderShadowMap();
	RenderSkybox();
	RenderCamera();
	LoadUI();
	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	NewRenderLines();
	NewRenderTextures();
	NewRenderText();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GameTechRenderer::BuildObjectList() {
	activeObjects.clear();

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {
				const RenderObject* g = o->GetRenderObject();
				if (g) {
					activeObjects.emplace_back(g);
				}
			}
		}
	);
}

void GameTechRenderer::SortObjectList() {
}


void GameTechRenderer::RenderShadowMap() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glCullFace(GL_FRONT);

	UseShader(*shadowShader);
	int mvpLocation = glGetUniformLocation(shadowShader->GetProgramID(), "mvpMatrix");

	Matrix4 shadowViewMatrix = Matrix::View(lightPosition, Vector3(0, 0, 0), Vector3(0, 1, 0));
	Matrix4 shadowProjMatrix = Matrix::Perspective(100.0f, 500.0f, 1.0f, 45.0f);

	Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;

	shadowMatrix = biasMatrix * mvMatrix; //we'll use this one later on

	for (const auto& i : activeObjects) {
		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		Matrix4 mvpMatrix = mvMatrix * modelMatrix;
		glUniformMatrix4fv(mvpLocation, 1, false, (float*)&mvpMatrix);
		BindMesh((OGLMesh&)*(*i).GetMesh());
		size_t layerCount = (*i).GetMesh()->GetSubMeshCount();
		for (size_t i = 0; i < layerCount; ++i) {
			DrawBoundMesh((uint32_t)i);
		}
	}

	glViewport(0, 0, windowSize.x, windowSize.y);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
}

void GameTechRenderer::RenderSkybox() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	UseShader(*skyboxShader);

	int projLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "viewMatrix");
	int texLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "cubeTex");

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

	glUniform1i(texLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	BindMesh(*skyboxMesh);
	DrawBoundMesh();

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void GameTechRenderer::RenderCamera() {	
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	OGLShader* activeShader = nullptr;
	int projLocation = 0;
	int viewLocation = 0;
	int modelLocation = 0;
	int colourLocation = 0;
	int hasVColLocation = 0;
	int hasTexLocation = 0;
	int shadowLocation = 0;

	int lightPosLocation = 0;
	int lightColourLocation = 0;
	int lightRadiusLocation = 0;

	int cameraLocation = 0;

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (int i = 0; i < activeObjects.size(); i++) {
		OGLShader* shader = (OGLShader*)activeObjects[i]->GetShader();
		UseShader(*shader);

		

		if (activeShader != shader) {
			projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
			viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
			modelLocation = glGetUniformLocation(shader->GetProgramID(), "modelMatrix");
			shadowLocation = glGetUniformLocation(shader->GetProgramID(), "shadowMatrix");
			colourLocation = glGetUniformLocation(shader->GetProgramID(), "objectColour");
			hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
			hasTexLocation = glGetUniformLocation(shader->GetProgramID(), "hasTexture");

			lightPosLocation = glGetUniformLocation(shader->GetProgramID(), "lightPos");
			lightColourLocation = glGetUniformLocation(shader->GetProgramID(), "lightColour");
			lightRadiusLocation = glGetUniformLocation(shader->GetProgramID(), "lightRadius");

			cameraLocation = glGetUniformLocation(shader->GetProgramID(), "cameraPos");

			Vector3 camPos = gameWorld.GetMainCamera().GetPosition();
			glUniform3fv(cameraLocation, 1, &camPos.x);

			glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
			glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

			glUniform3fv(lightPosLocation, 1, (float*)&lightPosition);
			glUniform4fv(lightColourLocation, 1, (float*)&lightColour);
			glUniform1f(lightRadiusLocation, lightRadius);

			int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			glUniform1i(shadowTexLocation, 1);

			activeShader = shader;
		}
		
		Matrix4 modelMatrix = activeObjects[i]->GetTransform()->GetMatrix();
		
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&modelMatrix);

		Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		Vector4 colour = activeObjects[i]->GetColour();
		glUniform4fv(colourLocation, 1, &colour.x);

		glUniform1i(hasVColLocation, !activeObjects[i]->GetMesh()->GetColourData().empty()); 
		glUniform1i(hasTexLocation, (OGLTexture*)activeObjects[i]->GetDefaultTexture() ? 1 : 0);

		OGLMesh* mesh = (OGLMesh*)activeObjects[i]->GetMesh();
		if (boundMesh != mesh) {
			BindMesh(*mesh);
			boundMesh = mesh;
		}
		size_t layerCount = activeObjects[i]->GetMesh()->GetSubMeshCount();
		
		if (gameReady) {
			if (activeObjects[i]->GetAnimObject()) {
				int j = glGetUniformLocation(shader->GetProgramID(), "joints");
				const std::vector<int>& matTextures = activeObjects[i]->GetMaterialTextures();
				std::vector<std::vector<Matrix4>> frameMatricesVec = activeObjects[i]->GetFrameMatricesVector();
				for (int b = 0; b < layerCount; ++b) {
					vector<Matrix4> frameMatrices = frameMatricesVec[b];
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, matTextures[b]);

					glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());
					DrawBoundMesh((uint32_t)b);
				}
			}
			else if (activeObjects[i]->GetMaterialTextures().size() > 1) {
				const std::vector<int>& matTextures = activeObjects[i]->GetMaterialTextures();
				for (size_t b = 0; b < layerCount; ++b) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, matTextures[b]);
					DrawBoundMesh((uint32_t)b);
				}
			}
			else {
				if (activeObjects[i]->GetDefaultTexture()) {
					BindTextureToShader(*(OGLTexture*)activeObjects[i]->GetDefaultTexture(), "mainTex", 0);
				}
				size_t layerCount = mesh->GetSubMeshCount();
				for (size_t b = 0; b < layerCount; ++b) {
					DrawBoundMesh((uint32_t)b);
				}
			}
		}
	}
}

Mesh* GameTechRenderer::LoadMesh(const std::string& name) {
	OGLMesh* mesh = new OGLMesh();
	MshLoader::LoadMesh(name, *mesh);
	mesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	mesh->UploadToGPU();
	return mesh;

}

void GameTechRenderer::LoadMeshes(std::unordered_map<std::string, Mesh*>& meshMap, const vector<std::string>& details) {
	std::vector <OGLMesh*> meshes;
	for (int i = 0; i < details.size(); i += 3) {
		meshes.push_back(new OGLMesh());
	}

	std::thread loadingThread[4];
	int splitLoad = details.size() / 12;

	for (int i = 0; i < 4; i++) {
		loadingThread[i] = std::thread([meshes, details, i, splitLoad] {
			int endPoint = i == 3 ? details.size() / 3 : splitLoad * (i + 1);
			for (int j = splitLoad * i; j < endPoint; j++) {
				MshLoader::LoadMesh(details[(j * 3) + 1], *meshes[j]);
				meshes[j]->SetPrimitiveType(GeometryPrimitive::Triangles);
			}
			});
	}
	for (int i = 0; i < 4; i++) {
		loadingThread[i].join();
	}

	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->UploadToGPU();
		meshMap[details[i * 3]] = meshes[i];
	}
}

void GameTechRenderer::NewRenderLines() {
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();
	if (lines.empty()) {
		return;
	}

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	Matrix4 viewProj = projMatrix * viewMatrix;

	UseShader(*debugShader);
	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 0);

	glUniformMatrix4fv(matSlot, 1, false, (float*)viewProj.array);

	debugLineData.clear();

	size_t frameLineCount = lines.size() * 2;

	SetDebugLineBufferSizes(frameLineCount);

	glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines.size() * sizeof(Debug::DebugLineEntry), lines.data());


	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, (GLsizei)frameLineCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderText() {
	const std::vector<Debug::DebugStringEntry>& strings = Debug::GetDebugStrings();
	if (strings.empty()) {
		return;
	}

	UseShader(*debugShader);

	OGLTexture* t = (OGLTexture*)Debug::GetDebugFont()->GetTexture();

	if (t) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		BindTextureToShader(*t, "mainTex", 0);
	}

	Matrix4 proj = Matrix::Orthographic(0.0f, 100.0f, 100.0f, 0.0f, -1.0f, 1.0f);

	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 1);

	debugTextPos.clear();
	debugTextColours.clear();
	debugTextUVs.clear();

	int frameVertCount = 0;
	for (const auto& s : strings) {
		frameVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	SetDebugStringBufferSizes(frameVertCount);

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildVerticesForString(s.data, s.position, s.colour, size, debugTextPos, debugTextUVs, debugTextColours);
	}

	glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector3), debugTextPos.data());
	glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector4), debugTextColours.data());
	glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector2), debugTextUVs.data());

	glBindVertexArray(textVAO);
	glDrawArrays(GL_TRIANGLES, 0, frameVertCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderTextures() {
	const std::vector<Debug::DebugTexEntry>& texEntries = Debug::GetDebugTex();
	if (texEntries.empty()) {
		return;
	}
	UseShader(*debugShader);

	Matrix4 proj = Matrix::Orthographic(0.0f, 100.0f, 100.0f, 0.0f, -1.0f, 1.0f);

	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 2);

	GLuint useColourSlot = glGetUniformLocation(debugShader->GetProgramID(), "useColour");
	glUniform1i(useColourSlot, 1);

	GLuint colourSlot = glGetUniformLocation(debugShader->GetProgramID(), "texColour");

	BindMesh(*debugTexMesh);

	glActiveTexture(GL_TEXTURE0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	for (const auto& tex : texEntries) {
		OGLTexture* t = (OGLTexture*)tex.t;
		glBindTexture(GL_TEXTURE_2D, t->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		BindTextureToShader(*t, "mainTex", 0);

		Matrix4 transform = Matrix::Translation(Vector3(tex.position.x, tex.position.y, 0)) * Matrix::Scale(Vector3(tex.scale.x, tex.scale.y, 1.0f));
		Matrix4 finalMatrix = proj * transform;

		glUniformMatrix4fv(matSlot, 1, false, (float*)finalMatrix.array);

		glUniform4f(colourSlot, tex.colour.x, tex.colour.y, tex.colour.z, tex.colour.w);

		DrawBoundMesh();
	}

	glUniform1i(useColourSlot, 0);
}

Texture* GameTechRenderer::LoadTexture(const std::string& name) {
	OGLTexture* texture = OGLTexture::TextureFromFile(name).release(); // TODO
	if (FindTextureIndex(texture->GetObjectID()) == -1)
	{
		const GLuint64 ID = glGetTextureHandleARB(texture->GetObjectID());
		glMakeTextureHandleResidentARB(ID);
		mTextureIDList.push_back(std::pair<GLuint, GLuint64>(texture->GetObjectID(), ID));
		mLoadedTextureList[name] = texture->GetObjectID();
	}
	return texture;
}

GLuint GameTechRenderer::LoadTextureGetID(const std::string& name) {
	if (mLoadedTextureList.find(name) != mLoadedTextureList.end()) {
		return mLoadedTextureList[name];
	}
	Texture* texture = LoadTexture(name);
	return ((OGLTexture*)texture)->GetObjectID();
}

int GameTechRenderer::FindTextureIndex(GLuint texId) {
	for (int i = 0; i < mTextureIDList.size(); i++) {
		if (texId == mTextureIDList[i].first) {
			return i;
		}
	}
	return -1;
}

Shader* GameTechRenderer::LoadShader(const std::string& vertex, const std::string& fragment) {
	return new OGLShader(vertex, fragment);
}

MeshAnimation* GameTechRenderer::LoadAnimation(const std::string& name) {
	return new MeshAnimation(name);
}

MeshMaterial* GameTechRenderer::LoadMaterial(const std::string& name) {
	return new MeshMaterial(name);
}

vector<int> GameTechRenderer::LoadMeshMaterial(Mesh& mesh, MeshMaterial& meshMaterial) {
	std::vector<int> matTextures = std::vector<int>();

	if (&mesh == nullptr) {
		std::cerr << "Error: Mesh reference is null!" << std::endl;
		return {};
	}
	std::cout << "Mesh is valid, checking submesh count..." << std::endl;


	for (int i = 0; i < mesh.GetSubMeshCount(); ++i) {
		std::cout << "I am inside the loop!" << std::endl;
		const MeshMaterialEntry* matEntry = meshMaterial.GetMaterialForLayer(i);
		if (!matEntry) {
			std::cout << "No Material for layer " << i << std::endl;
			continue;
		}
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		GLuint texID = 0;

		if (filename) {
			string path = *filename;
			std::cout << path << std::endl;
			texID = LoadTextureGetID(path.c_str());
			std::cout << texID << std::endl;
		}

		matTextures.emplace_back(texID);

		filename = nullptr;
		matEntry->GetEntry("Normal", &filename);
		texID = 0;

		if (!matEntry) {
			std::cout << "No Material for layer " << i << std::endl;
			continue;
		}

		if (filename) {
			string path = *filename;
			std::cout << path << std::endl;
			texID = LoadTextureGetID(path.c_str());
			std::cout << texID << std::endl;
		}
		matTextures.emplace_back(texID);
	}
	std::cout << "Loaded Mesh Material" << std::endl;
	return matTextures;
}
void GameTechRenderer::SetDebugStringBufferSizes(size_t newVertCount) {
	if (newVertCount > textCount) {
		textCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector4), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector2), nullptr, GL_DYNAMIC_DRAW);

		debugTextPos.reserve(textCount);
		debugTextColours.reserve(textCount);
		debugTextUVs.reserve(textCount);

		glBindVertexArray(textVAO);

		glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, textVertVBO, 0, sizeof(Vector3));

		glVertexAttribFormat(1, 4, GL_FLOAT, false, 0);
		glVertexAttribBinding(1, 1);
		glBindVertexBuffer(1, textColourVBO, 0, sizeof(Vector4));

		glVertexAttribFormat(2, 2, GL_FLOAT, false, 0);
		glVertexAttribBinding(2, 2);
		glBindVertexBuffer(2, textTexVBO, 0, sizeof(Vector2));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}
}


void GameTechRenderer::AddPanelToCanvas(const std::string& key, std::function<void()> func) {
	// Avoid adding duplicates by key
	if (mImguiCanvasFuncToRenderList.find(key) == mImguiCanvasFuncToRenderList.end()) {
		mImguiCanvasFuncToRenderList[key] = std::move(func);
	}
}


void GameTechRenderer::DeletePanelFromCanvas(const std::string& key) {

	auto it = mImguiCanvasFuncToRenderList.find(key);
	removePanelList.push_back(key);
	/*auto it = mImguiCanvasFuncToRenderList.find(key);
	if (it != mImguiCanvasFuncToRenderList.end()) {
		mImguiCanvasFuncToRenderList.erase(it);

	}*/
}

void GameTechRenderer::UpdatePanelList() {

	for (const auto& key : removePanelList) {
		auto it = mImguiCanvasFuncToRenderList.find(key);
		if (it != mImguiCanvasFuncToRenderList.end()) {
			mImguiCanvasFuncToRenderList.erase(it);
		}
	}
	removePanelList.clear();
}


void GameTechRenderer::SetDebugLineBufferSizes(size_t newVertCount) {
	if (newVertCount > lineCount) {
		lineCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
		glBufferData(GL_ARRAY_BUFFER, lineCount * sizeof(Debug::DebugLineEntry), nullptr, GL_DYNAMIC_DRAW);

		debugLineData.reserve(lineCount);

		glBindVertexArray(lineVAO);

		int realStride = sizeof(Debug::DebugLineEntry) / 2;

		glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, start));
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, lineVertVBO, 0, realStride);

		glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, colourA));
		glVertexAttribBinding(1, 0);
		glBindVertexBuffer(1, lineVertVBO, sizeof(Vector4), realStride);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
}


void GameTechRenderer::LoadUI() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Next window to be created will cover the entire screen
	NCL::Maths::Vector2i windowSize = NCL::Window::GetWindow()->GetScreenSize();
	float windowWidth = static_cast<float>(windowSize.x);
	float windowHeight = static_cast<float>(windowSize.y);

	ImVec2 size(windowWidth, windowHeight);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(size);

	ImGui::Begin("Background", NULL,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoResize
	);

	for (const auto& [key, func] : mImguiCanvasFuncToRenderList) {
		func();
	}
	if (USEDEBUGMODE) {
		Debug::DrawDebugMenu();
	}
	UpdatePanelList();

	//CLEAR

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}